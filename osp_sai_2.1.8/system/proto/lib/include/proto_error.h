
#ifndef __PROTO_ERROR_H__
#define __PROTO_ERROR_H__

typedef enum
{
    PM_E_NONE              = 0,

    PM_E_FAIL              = -1000,
    PM_E_INVALID_PARAM     = -999,
    PM_E_NOT_EXIST         = -998,
    PM_E_EXIST             = -997,
    PM_E_NOT_INITED        = -996,
    PM_E_INITED            = -995,
    PM_E_INIT_FAIL         = -994,
    PM_E_NO_MEMORY         = -993,
    PM_E_SOCK              = -992,
    PM_E_NOT_SUPPORT       = -991,
    PM_E_NOT_PERMIT        = -990,

    PM_E_FLOW_EXIST        = -989,
    PM_E_CFG_PROMPT        = -988,
    PM_E_CFG_DUPLICATED    = -987,
    PM_E_CFG_CONTINUE      = -986,

    PM_E_HAL_FAIL,
    PM_E_IPTABLES_EXEC_FAIL,
    PM_E_CONNTRACK_EXEC_FAIL,
    PM_E_IOCTL_FAIL,
    PM_E_EXCEED_CMD_LEN,
    PM_E_EXCEED_MAX_NUM,
    PM_E_EXCEED_ECMP_MAX_NUM,
    PM_E_EXCEED_STATIC_RT_MAX_NUM,
    PM_E_CFG_NOT_TO_PM,         /* need not send to PM, e.g. enter created vlan 100 */
    PM_E_CFG_SYS_BUSY,          /* for config ratelimit */
    PM_E_CFG_CONFLICT,          /* for config conflict */
    PM_E_CFG_NO_RESOURCE,       /* for resource exhaust */
    PM_E_CFG_INVALID_TABLE,     /* for table name invalid */
    PM_E_CFG_DB_FAIL,           /* for internal database error */
    PM_E_CFG_INVALID_PARAM,     /* for cmd parameter is invalid */
    PM_E_CFG_EXCEED_LEN,        /* for cmd exceed length */
    PM_E_CFG_PM_ERROR,          /* for cmd fails in PM */
    PM_E_CFG_PM_NOT_SUPPORT,    /* for cmd fails in PM */
    PM_E_CFG_CDS_SHOW,          /* show CDS information */
    PM_E_CFG_POLICER_NO_RESOURCE,       /* for resource exhaust */
    PM_E_CFG_POLICER_W_STATS_NO_RESOURCE,       /* for resource exhaust */
    PM_E_CFG_ACL_TCP_NO_RESPURCE,  /* for acl tcp resource exhaust */
    PM_E_CFG_ACL_NO_TIME_RANGE,
    PM_E_CFG_ACL_TCP_FLAGS_NONE,
    PM_E_CFG_ACL_VNI_CONFIG_CONFLICT,
    PM_E_CFG_ACL_STRIP_HEADER_INVALID_PARAM,
    PM_E_CFG_ACL_TRUNCTION_CONFIG_CONFLICT,
    PM_E_CFG_ACL_MAKR_SOURCE_UNTAG_CONFIG_CONFLICT,
    PM_E_CFG_ACL_SERVICE_ATTACHED_CONFLICT,
    PM_E_CFG_ERR_ETHER_TYPE,
    PM_E_CFG_ERR_UDF_VALUE,
    PM_E_CFG_ERR_UDF_TYPE,
    PM_E_CFG_ERR_UDF_NO_RESPURCE,

    PM_E_INVALID_IFNAME,
    PM_E_RESOURCE_FULL,
    PM_E_ECMP_RESOURCE_FULL,
    PM_E_EXCEED_TAP_GRP_TRCA_MAX_NUM,
    PM_E_ERPS_INSTANCE_NOT_EXIST,
    PM_E_ERPS_INSTANCE_USED_BY_OTHER_DOMAIN,
    PM_E_EXCEED_ARP_LIMIT_MAX_NUM,

    /*added by yejl for acl policy*/
    PM_E_ACL_NOT_EXIST,
    PM_E_CMAP_NOT_EXIST,
    PM_E_PMAP_NOT_EXIST,
    PM_E_ACL_IN_CLASS_NOT_EXIST,
    PM_E_CLASS_IN_POLICY_NOT_EXIST,
    /*end*/

    PM_E_ARP_RATE_LIMIT_ERR,
    PM_E_VRRP_BAD_PACKET,  /* support vrrp modified by liwh for bug 45215, 2017-09-17 */

    PM_E_CDB_SYNC_INVALID_CHANNEL,
    PM_E_CDB_SYNC_INVALID_OPER,
    PM_E_CDB_SYNC_KEY_NAME_NOT_EXIST,
    PM_E_CDB_SYNC_FIELD_NAME_NOT_EXIST,
    PM_E_CDB_SYNC_FIELD_VALUE_NOT_EXIST,
    PM_E_CDB_SYNC_INVALID_KEY,
    PM_E_CDB_SYNC_INVALID_FIELD,
    PM_E_CDB_SYNC_INVALID_VALUE,
    PM_E_ND_BAD_PACKET,    /* support nd modified by liwh for bug 47547, 2018-06-28 */

    PM_E_ROUTE_DEST_SAME_WITH_NEXTHOP,
    PM_E_ROUTE_INDIRECT_ROUTE_FULL,
    PM_E_ROUTE_HOST_ROUTE_FULL,
    
    PM_E_ROUTE_IF_INVALID_PREFIX_LEN_FOR_LB_IF,
    PM_E_MANAGEMENT_IF_GATEWAY_NOT_EXIST,

    /*modified by yejl for bug 49938 and bug 49973, 2018-11-13*/
    PM_E_CHECK_CMAP_MONITOR_UNSUPPORT,
    PM_E_CHECK_CMAP_POLICY_UNSUPPORT,
    PM_E_CHECK_CMAP_CTAG_COS_UNSUPPORT,
    PM_E_CHECK_CMAP_CTAG_UNSUPPORT,
    PM_E_CHECK_CMAP_DSCP_UNSUPPORT,
    PM_E_CHECK_CMAP_STAG_COS_UNSUPPORT,
    PM_E_CHECK_CMAP_STAG_UNSUPPORT,
    PM_E_CHECK_CMAP_TC_TO_COLOR_UNSUPPORT,
    PM_E_CHECK_CMAP_REDIRECT_UNSUPPORT,
    PM_E_CHECK_CMAP_POLICY_APPLY_ON_COPP,
    PM_E_CHECK_CMAP_POLICY_APPLY_ON_EGRESS_PORT,
    /*end*/
    CFG_RC_MAX
} pm_error_t;

#endif /* !__PROTO_ERROR_H__ */


int32
tbl_interface_get_ifindex_by_name(const char *ifname);

uint32
tbl_interface_get_type_by_ifindex(uint32 ifindex);

vid_t
tbl_interface_get_vid_by_ifindex(uint32 ifindex);

int32
tbl_interface_get_name_by_ifindex(uint32 ifindex, char *name, uint32 len);

tbl_interface_t*
tbl_interface_get_interface_by_name(const char* ifname);

tbl_interface_t*
tbl_interface_get_interface_by_ifindex(uint32 ifindex);

uint32
tbl_interface_is_up(tbl_interface_t *p_if);

uint32
tbl_interface_is_running(tbl_interface_t *p_if);

tbl_interface_t*
tbl_interface_get_interface_by_port_id(uint64 port_id);

int32
tbl_interface_get_lag_1st_member_ifindex(uint32 lag_ifindex, uint32 *mem_ifindex);

#define INTERFACE_IS_LAG_GROUP(IF)      (GLB_IF_TYPE_LINKAGG_IF == (IF)->hw_type)
#define INTERFACE_IS_LAG_MEMBER(IF)     ((GLB_IF_TYPE_LINKAGG_IF != (IF)->hw_type) && (NULL != (IF)->lag))
#define INTERFACE_IS_LAG(IF)            (NULL != (IF)->lag)

#define INTERFACE_IS_LOOPBACK(IF)       (GLB_IF_TYPE_LOOPBACK_IF == (IF)->hw_type)

#define INTERFACE_IS_TUNNEL(IF) ((GLB_IF_TYPE_TUNNEL_IF == (IF)->hw_type) || (GLB_IF_TYPE_L2GRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_L3GRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_NVGRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_VXLAN_IF == (IF)->hw_type))
#define INTERFACE_IS_OVERLAY_TN(IF) ((GLB_IF_TYPE_L2GRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_L3GRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_NVGRE_IF == (IF)->hw_type) || (GLB_IF_TYPE_VXLAN_IF == (IF)->hw_type))

#define INTERFACE_IS_CPUIF(IF)       (GLB_IF_TYPE_CPU_IF == (IF)->hw_type)

#define IF_MGR_CHECK_LAG_MEMBER(IF) \
if (INTERFACE_IS_LAG_MEMBER((IF))) \
{ \
    CFG_CONFLICT_RET("Cannot configure on LAG member port"); \
}

#define IF_MGR_CHECK_LAG_GROUP(IF) \
if (INTERFACE_IS_LAG_GROUP((IF))) \
{ \
    CFG_CONFLICT_RET("Cannot configure on LAG interface"); \
}

#define IF_MGR_CHECK_OVERLAY_TL_GROUP(IF) \
if (INTERFACE_IS_OVERLAY_TN((IF))) \
{ \
    CFG_CONFLICT_RET("Cannot configure on overlay tunnel interface"); \
}

#define INTERFACE_IS_OPENFLOW_PORT(IF)     (NULL != (IF)->openflow_if)

#define IF_MGR_CHECK_OPENFLOW_PORT(IF) \
if (INTERFACE_IS_OPENFLOW_PORT((IF))) \
{ \
    CFG_CONFLICT_RET("Cannot configure on openflow port"); \
}

#define INTF_CHECK_DOT1X_UNAUTH_RX_RETURN(_P_DB_IF_) \
do { \
    if ((_P_DB_IF_)->dot1x_enable) \
    { \
        if (GLB_FLAG_ISSET((_P_DB_IF_)->dot1x_flag, GLB_IF_DOT1X_UNAUTHED)) \
        { \
            return PM_E_NONE; \
        } \
    } \
} while (0)

#define INTF_CHECK_DOT1X_UNAUTH_TX_RETURN(_P_DB_IF_) \
do { \
    if ((_P_DB_IF_)->dot1x_enable) \
    { \
        if (GLB_FLAG_ISSET((_P_DB_IF_)->dot1x_flag, GLB_IF_DOT1X_UNAUTHED) && GLB_FLAG_ISSET((_P_DB_IF_)->dot1x_flag, GLB_IF_DOT1X_DIR_OUT)) \
        { \
            return PM_E_NONE; \
        } \
    } \
} while (0)

#define TBL_INTERFACE_ST        tbl_interface_t
#define TBL_INTERFACE_DS                        \
    DEFINE_DS(DS_BRGIF,         brgif)          \
    DEFINE_DS(DS_ACLQOS_IF,     aclqos_if)      \
    DEFINE_DS(DS_LAG,           lag)            \
    DEFINE_DS(DS_LACP,          lacp)           \
    DEFINE_DS(DS_STORM_CONTROL, storm_control)  \
    DEFINE_DS(DS_OPENFLOW_IF,   openflow_if)


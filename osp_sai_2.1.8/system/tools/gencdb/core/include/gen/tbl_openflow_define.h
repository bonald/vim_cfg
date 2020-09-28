
#ifndef __TBL_OPENFLOW_DEFINE_H__
#define __TBL_OPENFLOW_DEFINE_H__

/* TBL_OPENFLOW field defines */
typedef enum
{
    TBL_OPENFLOW_FLD_GROUP_HASH_KEY       = 0 ,  /* RW */
    TBL_OPENFLOW_FLD_BOND_HASH_KEY        = 1 ,  /* RW */
    TBL_OPENFLOW_FLD_BOND_HASH_USE        = 2 ,  /* RW */
    TBL_OPENFLOW_FLD_LACP_LOCAL_PROC      = 3 ,  /* RW */
    TBL_OPENFLOW_FLD_LOCAL_VTEP_IP        = 4 ,  /* RW */
    TBL_OPENFLOW_FLD_DECAP_MODE           = 5 ,  /* RW */
    TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT = 6 ,  /* RW */
    TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT = 7 ,  /* RW */
    TBL_OPENFLOW_FLD_UDF_PARSER           = 8 ,  /* RW */
    TBL_OPENFLOW_FLD_METER_IPG            = 9 ,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_EN            = 10,  /* RW */
    TBL_OPENFLOW_FLD_PTP_E2E_EN           = 11,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_STAG          = 12,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_ADDR          = 13,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_GW            = 14,  /* RW */
    TBL_OPENFLOW_FLD_MPLS_PARSER          = 15,  /* RW */
    TBL_OPENFLOW_FLD_STPID                = 16,  /* RW */
    TBL_OPENFLOW_FLD_CTPID                = 17,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP = 18,  /* RW */
    TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME = 19,  /* RW */
    TBL_OPENFLOW_FLD_FLOW_HASH_KEY        = 20,  /* RW */
    TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN  = 21,  /* RW */
    TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN  = 22,  /* RW */
    TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE  = 23,  /* RW */
    TBL_OPENFLOW_FLD_EFD_GRANULARITY      = 24,  /* RW */
    TBL_OPENFLOW_FLD_EFD_DETECT_SPEED     = 25,  /* RW */
    TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL = 26,  /* RW */
    TBL_OPENFLOW_FLD_EFD_AGING_TIMER      = 27,  /* RW */
    TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS = 28,  /* RW */
    TBL_OPENFLOW_FLD_EFD_FLOW_COLOR       = 29,  /* RW */
    TBL_OPENFLOW_FLD_EFD_IPG_ENABLE       = 30,  /* RW */
    TBL_OPENFLOW_FLD_DPID                 = 31,  /* RW */
    TBL_OPENFLOW_FLD_DPID_IS_SET          = 32,  /* RW */
    TBL_OPENFLOW_FLD_PRIVATE_KEY          = 33,  /* RW */
    TBL_OPENFLOW_FLD_CERTIFICATE          = 34,  /* RW */
    TBL_OPENFLOW_FLD_CA_CERT              = 35,  /* RW */
    TBL_OPENFLOW_FLD_VXLAN_SRCPORT        = 36,  /* RW */
    TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET = 37,  /* RW */
    TBL_OPENFLOW_FLD_VXLAN_DESTPORT       = 38,  /* RW */
    TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET = 39,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL  = 40,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET = 41,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE = 42,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL  = 43,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET = 44,  /* RW */
    TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE = 45,  /* RW */
    TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE = 46,  /* RW */
    TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM  = 47,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT = 48,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT = 49,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT = 50,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT = 51,  /* RW */
    TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT = 52,  /* RW */
    TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT = 53,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT = 54,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT = 55,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT = 56,  /* RW */
    TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT = 57,  /* RW */
    TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT = 58,  /* RW */
    TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT = 59,  /* RW */
    TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE = 60,  /* RW */
    TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE = 61,  /* RW */
    TBL_OPENFLOW_FLD_ASYNC_MSGID_EN       = 62,  /* RW */
    TBL_OPENFLOW_FLD_FDB_FLOW_EN          = 63,  /* RW */
    TBL_OPENFLOW_FLD_FDB_FLOW_VALID       = 64,  /* RW */
    TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN = 65,  /* RW */
    TBL_OPENFLOW_FLD_MAX                  = 66
} tbl_openflow_field_id_t;

/* TBL_OPENFLOW defines */
typedef struct
{
    uint32               group_hash_key;      /* group hash key */
    uint32               bond_hash_key;       /* bond hash key */
    uint32               bond_hash_use;       /* bond hash use */
    uint32               lacp_local_proc;     /* bond hash use */
    addr_ipv4_t          local_vtep_ip;       /* local vtep ip */
    char                 decap_mode[GLB_MAX_DESC_STR_LEN]; /* decapsulation mode */
    uint8                flow_drop_pkt_to_ingress_port; /* flow drop pkt to ingress port */
    uint8                group_drop_pkt_to_ingress_port; /* group drop pkt to ingress port */
    uint8                udf_parser;          /* udf parser */
    uint8                meter_ipg;           /* meter ipg */
    uint8                inband_en;           /* inband enable */
    uint8                ptp_e2e_en;          /* e2e enable */
    vid_t                inband_stag;         /* inband stag */
    prefix_ipv4_t        inband_addr;         /* inband ip address */
    addr_ipv4_t          inband_gw;           /* inband gw address */
    uint8                mpls_parser;         /* mpls parser */
    int32                stpid;               /* stpid */
    int32                ctpid;               /* ctpid */
    port_bmp_t           inband_downlink_port_bmp;
    char                 inband_uplink_port_name[IFNAME_SIZE];
    uint32               flow_hash_key;       /* flow hash key */
    uint8                vxlan_hash_merge_en; /* flow hash vxlan merge enable */
    uint8                nvgre_hash_merge_en; /* flow hash nvgre merge enable */
    uint8                efd_tcp_only_enable; /* efd tcp-only enable*/
    uint32               efd_granularity;     /* efd granularity*/
    uint32               efd_detect_speed;    /* efd detect speed*/
    uint32               efd_detect_time_interval; /* efd detect time-interval*/
    uint32               efd_aging_timer;     /* efd aging-timer*/
    uint32               efd_flow_traffic_class; /* efd_flow_traffic_class*/
    uint32               efd_flow_color;      /* efd_flow_color*/
    uint8                efd_ipg_enable;      /* efd ipg enable*/
    char                 dpid[GLB_MAX_DPID_STR_LEN]; /* openflow set dpid*/
    uint32               dpid_is_set;         /* openflow dpid is set*/
    char                 private_key[FILE_NAME_SIZE]; /* ssl private_key*/
    char                 certificate[FILE_NAME_SIZE]; /* ssl certificate*/
    char                 ca_cert[FILE_NAME_SIZE]; /* ssl ca_cert*/
    uint32               vxlan_srcport;
    uint32               vxlan_srcport_is_set;
    uint32               vxlan_destport;
    uint32               vxlan_destport_is_set;
    uint32               tp_oam_oam_interval;
    uint32               tp_oam_oam_interval_is_set;
    uint32               tp_oam_oam_cycle_is_disable;
    uint32               tp_oam_aps_interval;
    uint32               tp_oam_aps_interval_is_set;
    uint32               tp_oam_aps_cycle_is_disable;
    uint32               pw_not_standard_mode; /* pw mode, default is  standard */
    uint32               check_port_bind_oam; /* ifindex*/
    uint32               oam_packet_in_cycle_cnt;
    uint32               oam_packet_in_event_ccm_cnt;
    uint32               oam_packet_in_event_lm_cnt;
    uint32               oam_packet_in_event_dm_cnt;
    uint32               aps_packet_in_cycle_cnt;
    uint32               aps_packet_in_event_path_change_cnt;
    uint32               oam_packet_in_cycle_packet_cnt;
    uint32               oam_packet_in_event_packet_ccm_cnt;
    uint32               oam_packet_in_event_packet_lm_cnt;
    uint32               oam_packet_in_event_packet_dm_cnt;
    uint32               aps_packet_in_cycle_packet_cnt;
    uint32               aps_packet_in_event_path_change_packet_cnt;
    uint32               openflow_mac_learning_enable;
    uint8                controller_affect_flow_disable;
    uint8                async_msgid_en;
    uint8                fdb_flow_en;
    uint8                fdb_flow_valid;
    uint8                ffgroup_switchback_en;
} tbl_openflow_t;

#endif /* !__TBL_OPENFLOW_DEFINE_H__ */


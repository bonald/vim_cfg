
#ifndef __TBL_INTERFACE_DEFINE_H__
#define __TBL_INTERFACE_DEFINE_H__

#include "gen/ds_circuit_id_define.h"
#include "gen/ds_brgif_define.h"
#include "gen/ds_lag_define.h"
#include "gen/ds_lacp_define.h"
#include "gen/ds_storm_control_define.h"
#include "gen/ds_aclqos_if_define.h"
#include "gen/ds_openflow_if_define.h"
#include "gen/ds_dhclient_if_define.h"
#include "gen/ds_l2protocol_define.h"

/* TBL_INTERFACE field defines */
typedef enum
{
    TBL_INTERFACE_FLD_KEY                  = 0 ,  /* READ */
    TBL_INTERFACE_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_INTERFACE_FLD_PORTID               = 2 ,  /* READ */
    TBL_INTERFACE_FLD_HOSTIFID             = 3 ,  /* READ */
    TBL_INTERFACE_FLD_DESC                 = 4 ,  /* RW */
    TBL_INTERFACE_FLD_SPEED                = 5 ,  /* RW */
    TBL_INTERFACE_FLD_DUPLEX               = 6 ,  /* RW */
    TBL_INTERFACE_FLD_UNIDIR               = 7 ,  /* RW */
    TBL_INTERFACE_FLD_FLOWCONTROL_SEND     = 8 ,  /* RW */
    TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE  = 9 ,  /* RW */
    TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND = 10,  /* RW */
    TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE = 11,  /* RW */
    TBL_INTERFACE_FLD_FEC                  = 12,  /* RW */
    TBL_INTERFACE_FLD_PFC_ADMIN            = 13,  /* RW */
    TBL_INTERFACE_FLD_PFC_OPER             = 14,  /* RW */
    TBL_INTERFACE_FLD_PFC_PRIORITY         = 15,  /* RW */
    TBL_INTERFACE_FLD_JUMBOFRAME_EN        = 16,  /* RW */
    TBL_INTERFACE_FLD_ISOLATE_GROUP_ID     = 17,  /* RW */
    TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST  = 18,  /* RW */
    TBL_INTERFACE_FLD_PHY_STATUS           = 19,  /* RW */
    TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE    = 20,  /* RW */
    TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX  = 21,  /* RW */
    TBL_INTERFACE_FLD_PVLAN_TYPE           = 22,  /* RW */
    TBL_INTERFACE_FLD_PRIVATE_VLAN         = 23,  /* RW */
    TBL_INTERFACE_FLD_COMMUNITY_VLAN       = 24,  /* RW */
    TBL_INTERFACE_FLD_NTP_DISABLE          = 25,  /* RW */
    TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT = 26,  /* RW */
    TBL_INTERFACE_FLD_PVLAN_GROUP_ID       = 27,  /* RW */
    TBL_INTERFACE_FLD_SUPPORT_SPEED        = 28,  /* READ */
    TBL_INTERFACE_FLD_MODE                 = 29,  /* RW */
    TBL_INTERFACE_FLD_VLAN_TYPE            = 30,  /* RW */
    TBL_INTERFACE_FLD_UP                   = 31,  /* RW */
    TBL_INTERFACE_FLD_RUNNING              = 32,  /* READ */
    TBL_INTERFACE_FLD_MTU                  = 33,  /* RW */
    TBL_INTERFACE_FLD_OPER_SPEED           = 34,  /* READ */
    TBL_INTERFACE_FLD_PREV_OPER_SPEED      = 35,  /* READ */
    TBL_INTERFACE_FLD_OPER_DUPLEX          = 36,  /* READ */
    TBL_INTERFACE_FLD_OPER_UNIDIR          = 37,  /* RW */
    TBL_INTERFACE_FLD_OPER_FEC             = 38,  /* RW */
    TBL_INTERFACE_FLD_OPER_LOOPBACK        = 39,  /* RW */
    TBL_INTERFACE_FLD_BANDWIDTH            = 40,  /* RW */
    TBL_INTERFACE_FLD_TAP_USED             = 41,  /* RW */
    TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE   = 42,  /* RW */
    TBL_INTERFACE_FLD_TAP                  = 43,  /* RW */
    TBL_INTERFACE_FLD_TAP_MARK_VLAN        = 44,  /* RW */
    TBL_INTERFACE_FLD_MIRROR_ENABLE        = 45,  /* RW */
    TBL_INTERFACE_FLD_MIRROR_IGS_ID        = 46,  /* RW */
    TBL_INTERFACE_FLD_MIRROR_EGS_ID        = 47,  /* RW */
    TBL_INTERFACE_FLD_MIRROR_DEST_ID       = 48,  /* RW */
    TBL_INTERFACE_FLD_ADMIN_UP             = 49,  /* RW */
    TBL_INTERFACE_FLD_IP_SOURCE_EN         = 50,  /* RW */
    TBL_INTERFACE_FLD_IP_SOURCE_TYPE       = 51,  /* RW */
    TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER = 52,  /* RW */
    TBL_INTERFACE_FLD_UNICAST_RPF_EN       = 53,  /* RW */
    TBL_INTERFACE_FLD_OPER_BANDWIDTH       = 54,  /* READ */
    TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE = 55,  /* RW */
    TBL_INTERFACE_FLD_ERRDISABLE_REASON    = 56,  /* RW */
    TBL_INTERFACE_FLD_FDB_LOOP_OCCUR       = 57,  /* RW */
    TBL_INTERFACE_FLD_LINK_FLAP_CNT        = 58,  /* RW */
    TBL_INTERFACE_FLD_STATIC_SECURITY_NUM  = 59,  /* RW */
    TBL_INTERFACE_FLD_ERRDISABLE_TIMER     = 60,  /* READ */
    TBL_INTERFACE_FLD_SFLOW_FLOW_EN        = 61,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS   = 62,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS    = 63,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_COUNTER_EN     = 64,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_RATE           = 65,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_SEQUENCE       = 66,  /* RW */
    TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL    = 67,  /* RW */
    TBL_INTERFACE_FLD_MAC_ADDR             = 68,  /* READ */
    TBL_INTERFACE_FLD_HW_MAC_ADDR          = 69,  /* READ */
    TBL_INTERFACE_FLD_HW_TYPE              = 70,  /* READ */
    TBL_INTERFACE_FLD_PHY_TYPE             = 71,  /* READ */
    TBL_INTERFACE_FLD_MEDIA                = 72,  /* RW */
    TBL_INTERFACE_FLD_PHYPORT_COMBO        = 73,  /* READ */
    TBL_INTERFACE_FLD_PHYPORT_LOOPBACK     = 74,  /* RW */
    TBL_INTERFACE_FLD_PHYPORT_EEE          = 75,  /* READ */
    TBL_INTERFACE_FLD_PHYPORT_POE          = 76,  /* READ */
    TBL_INTERFACE_FLD_PHYPORT_MASTER       = 77,  /* READ */
    TBL_INTERFACE_FLD_PHYPORT_SLAVE        = 78,  /* READ */
    TBL_INTERFACE_FLD_PHYPORT_UNIDIR       = 79,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_100M         = 80,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_1G           = 81,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_10G          = 82,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_40G          = 83,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_100G         = 84,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_2_5G         = 85,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_5G           = 86,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_25G          = 87,  /* READ */
    TBL_INTERFACE_FLD_SUPPORT_50G          = 88,  /* READ */
    TBL_INTERFACE_FLD_CIRCUIT_ID           = 89,  /* SUB */
    TBL_INTERFACE_FLD_IGS_ACL              = 90,  /* RW */
    TBL_INTERFACE_FLD_EGS_ACL              = 91,  /* RW */
    TBL_INTERFACE_FLD_IGS_IPV6_ACL         = 92,  /* RW */
    TBL_INTERFACE_FLD_EGS_IPV6_ACL         = 93,  /* RW */
    TBL_INTERFACE_FLD_IGS_MAC_ACL          = 94,  /* RW */
    TBL_INTERFACE_FLD_EGS_MAC_ACL          = 95,  /* RW */
    TBL_INTERFACE_FLD_IGS_POLICY_MAP       = 96,  /* RW */
    TBL_INTERFACE_FLD_EGS_POLICY_MAP       = 97,  /* RW */
    TBL_INTERFACE_FLD_BRGIF                = 98,  /* SUB */
    TBL_INTERFACE_FLD_LAG                  = 99,  /* SUB */
    TBL_INTERFACE_FLD_LACP                 = 100,  /* SUB */
    TBL_INTERFACE_FLD_STORM_CONTROL        = 101,  /* SUB */
    TBL_INTERFACE_FLD_ACLQOS_IF            = 102,  /* SUB */
    TBL_INTERFACE_FLD_OPENFLOW_IF          = 103,  /* SUB */
    TBL_INTERFACE_FLD_DHCLIENT_IF          = 104,  /* SUB */
    TBL_INTERFACE_FLD_L2PROTOCOL           = 105,  /* SUB */
    TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN   = 106,  /* RW */
    TBL_INTERFACE_FLD_CRC_RECALCULATION_EN = 107,  /* RW */
    TBL_INTERFACE_FLD_MLAG_ID              = 108,  /* READ */
    TBL_INTERFACE_FLD_ERPS_ENABLE          = 109,  /* RW */
    TBL_INTERFACE_FLD_ERPS_PDU_ACTION      = 110,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_ENABLE         = 111,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED   = 112,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_DIR_IN         = 113,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_DIR_OUT        = 114,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_MODE_MAC       = 115,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN     = 116,  /* RW */
    TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID   = 117,  /* RW */
    TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE = 118,  /* RW */
    TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION = 119,  /* RW */
    TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN = 120,  /* RW */
    TBL_INTERFACE_FLD_EGS_ACL_EN           = 121,  /* RW */
    TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT = 122,  /* RW */
    TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT = 123,  /* RW */
    TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT   = 124,  /* RW */
    TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT     = 125,  /* RW */
    TBL_INTERFACE_FLD_METADATA_ENABLE      = 126,  /* RW */
    TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE = 127,  /* RW */
    TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE = 128,  /* RW */
    TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER = 129,  /* RW */
    TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE = 130,  /* RW */
    TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR = 131,  /* RW */
    TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR = 132,  /* RW */
    TBL_INTERFACE_FLD_INBAND_SNAT_TYPE     = 133,  /* RW */
    TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH   = 134,  /* RW */
    TBL_INTERFACE_FLD_LATENCY_EVENT_LOW    = 135,  /* RW */
    TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE = 136,  /* RW */
    TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN = 137,  /* RW */
    TBL_INTERFACE_FLD_SUPPORT_SSM_EN       = 138,  /* RW */
    TBL_INTERFACE_FLD_SVLAN_TPID           = 139,  /* RW */
    TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN = 140,  /* RW */
    TBL_INTERFACE_FLD_L2PRO_UPLINK         = 141,  /* RW */
    TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY  = 142,  /* RW */
    TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE = 143,  /* RW */
    TBL_INTERFACE_FLD_FEC_STATUS           = 144,  /* RW */
    TBL_INTERFACE_FLD_CL73_ABILITY         = 145,  /* RW */
    TBL_INTERFACE_FLD_IFG_STATS_EN         = 146,  /* RW */
    TBL_INTERFACE_FLD_OPENFLOW_EN          = 147,  /* RW */
    TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG = 148,  /* RW */
    TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX = 149,  /* RW */
    TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL = 150,  /* RW */
    TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID = 151,  /* RW */
    TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR = 152,  /* RW */
    TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG = 153,  /* RW */
    TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH  = 154,  /* RW */
    TBL_INTERFACE_FLD_POE                  = 155,  /* RW */
    TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT = 156,  /* RW */
    TBL_INTERFACE_FLD_G8032_RING_FIRST     = 157,  /* RW */
    TBL_INTERFACE_FLD_CFM_LMEP_NUM         = 158,  /* RW */
    TBL_INTERFACE_FLD_CFM_RMEP_NUM         = 159,  /* RW */
    TBL_INTERFACE_FLD_MAX                  = 160
} tbl_interface_field_id_t;

/* TBL_INTERFACE defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_interface_key_t;

typedef struct
{
    tbl_interface_key_t  key;
    uint32               ifindex;             /* ifindex assign in user-space(PMs, Fea) to indicate an interface */
    uint64               portid;              /* port id for ASIC mapping */
    uint64               hostifid;            /* hostif id for SAI */
    char                 desc[DESC_SIZE+1];
    uint8                speed;
    uint8                duplex;
    uint8                unidir;
    uint8                flowcontrol;         /* bitmap of GLB_IF_FLOW_CONTROL_ */
    uint8                fec;
    uint8                pfc_admin;
    uint8                pfc_oper;
    uint8                pfc_priority;
    uint8                jumboframe_en;
    uint8                isolate_group_id;
    uint8                dhcp_snooping_trust; /* dhcp snooping trust interface or not for L2 */
    uint8                phy_status;          /* physical port status, glb_port_oper_status_t*/
    uint8                phy_loopback_mode;   /* physical port loopback mode, glb_lb_phy_t, valid when phyport_flag is GLB_PHYPORT_FLAG_LOOPBACK */
    uint32               internal_lp_ifindex; /*  this is parameters only use when phy_loopback_mode is internal mode */
    uint32               pvlan_type;          /* glb_if_pvlan_type_t */
    vid_t                private_vlan;
    vid_t                community_vlan;
    uint32               ntp_disable;
    uint32               ntp_broadcast_client;
    uint8                pvlan_group_id;
    uint8                support_speed;       /* bitmap of GLB_SUPPORT_SPEED_ */
    uint8                mode;                /* glb_if_mode_t */
    uint8                vlan_type;           /* glb_vlan_port_type_t */
    uint32               flags;
    uint32               mtu;                 /* MTU */
    uint8                oper_speed;
    uint8                prev_oper_speed;     /* prev oper speed */
    uint8                oper_duplex;
    uint8                oper_unidir;
    uint8                oper_fec;
    uint8                oper_loopback;
    uint32               bandwidth;           /* bandwidth */
    uint32               tap_used;
    uint32               tap_ingress_enable;  /* tap group ingress bind to this interface */
    char                 tap[TAP_NAME_SIZE];  /* tap group name who used this ingress */
    uint32               tap_mark_vlan;       /* tap group mark source vlan */
    uint32               mirror_enable;       /* mirror source or destination enable */
    uint8                mirror_igs_id;       /* mirror ingress session id */
    uint8                mirror_egs_id;       /* mirror egress session id */
    uint8                mirror_dest_id;      /* mirror dest session id */
    uint8                admin_up;            /* admin_up is used for running-config and snmp get, same to up without errdisable */
    uint32               ip_source_en;        /* ip source guard enable */
    uint32               ip_source_type;      /* ip source guard type */
    uint32               ip_source_item_number; /* ip source guard num */
    uint32               unicast_rpf_en;
    uint32               oper_bandwidth;      /* operate bandwidth */
    uint32               bandwidth_may_change; /*used for openflow agg get bandwith*/
    uint32               errdisable_reason;   /* port Error disable reason*/
    uint32               fdb_loop_occur;      /* occur FDB loop, to trigger errdisable */
    uint32               link_flap_cnt;       /* link flap number*/
    uint32               static_security_num; /* static port-security mac num*/
    ctc_task_t           *errdisable_timer;
    uint32               sflow_flow_en;       /* sflow enable flow */
    uint32               sflow_flow_ingress;  /* sflow flow ingress */
    uint32               sflow_flow_egress;   /* sflow flow egress */
    uint32               sflow_counter_en;    /* sflow enable counter */
    uint32               sflow_rate;          /* sflow flow rate*/
    uint32               sflow_sequence;      /* sflow flow sequence, incremented with each flow sample */
    uint32               sflow_sample_pool;   /* sflow total number of packets that could have been sampled */
    mac_addr_t           mac_addr;            /* l2: hw_mac_addr, l3: route_mac */
    mac_addr_t           hw_mac_addr;         /* l2 mac addr */
    uint32               hw_type;             /* glb_if_type_t */
    uint32               phy_type;            /* glb_phy_type_t */
    uint32               media;               /* configured media for combo port, glb_port_media_t */
    uint8                phyport_flag;        /* bitmap of GLB_PHYPORT_FLAG_ */
    uint8                support_speed_flag;  /* bitmap of GLB_SUPPORT_SPEED_ */
    cdb_reference_list_t circuit_id;          /* circuit id hash for dhcp snooping */
    char                 igs_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* ingress ACL name*/
    char                 egs_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* egress ACL name*/
    char                 igs_ipv6_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* ingress Ipv6 ACL name*/
    char                 egs_ipv6_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* egress IPv6 ACL name*/
    char                 igs_mac_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* ingress MAC ACL name*/
    char                 egs_mac_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* egress MAC ACL name*/
    char                 igs_policy_map[PMAP_NAME_SIZE+1]; /* ingress ACL name*/
    char                 egs_policy_map[PMAP_NAME_SIZE+1]; /* egress ACL name*/
    ds_brgif_t           *brgif;              /* bridge interface info */
    ds_lag_t             *lag;                /* LAG info */
    ds_lacp_t            *lacp;               /* LACP link info */
    ds_storm_control_t   *storm_control;      /* storm control information */
    ds_aclqos_if_t       *aclqos_if;          /* acl&qos interface configurateion info */
    ds_openflow_if_t     *openflow_if;        /* openflow interface configurateion info */
    ds_dhclient_if_t     *dhclient_if;        /* dhclient interface configurateion info */
    cdb_reference_list_t l2protocol;
    void                 *mstp_port;          /* refer to tbl_mstp_port_t */
    uint32               crc_error_check_en;  /* crc error check enable */
    uint32               crc_recalculation_en; /* crc recalculation enable */
    uint32               mlag_id;             /* MLAG ID */
    uint32               erps_enable;
    uint32               erps_pdu_action;
    uint32               dot1x_enable;
    uint32               dot1x_flag;
    uint32               dot1x_guest_vlan;
    uint32               vlanclass_group_id;
    uint32               vlanclass_active_type;
    uint32               vlanclass_default_action;
    uint32               dot1x_cfg_guest_vlan; /* configurable guest vlan  */
    uint8                egs_acl_en;          /* set port egress acl enable or disable */
    uint32               tap_group_igs_ref_cnt; /* tap group ingress configured count  */
    uint32               tap_group_egs_ref_cnt; /* tap group egress configured count  */
    uint32               tap_flow_decap_cnt;
    uint32               tap_flow_udf_cnt;
    uint32               metadata_enable;     /* port enable metadata */
    uint32               arpinsp_trust_enable; /*arp inspection enable/disable on port*/
    uint32               arp_numberlimit_enable; /*arp number limit enable/disable*/
    uint32               arp_numberlimit_number; /*arp number limit number*/
    uint32               arp_numberlimit_violate; /*arp number limit violate: protect, restrict, errdisable*/
    uint32               arp_numberlimit_occur; /* arp number limit occur, to trigger errdisable */
    uint32               arp_rate_limit_errdisable_occur; /* arp rate limit occur, to trigger errdisable */
    uint32               inband_snat_type;    /* refer to inband_snat_type_t  */
    uint32               latency_event_high;  /* set latency event high threshold */
    uint32               latency_event_low;   /* set latency event low threshold */
    uint32               latency_event_enable; /* enable latency event */
    uint32               reflective_bridge_en; /*enable reflective bridge*/
    uint32               support_ssm_en;      /*Interface support synce ssm or not*/
    uint32               svlan_tpid;          /*svlan-tpid type value*/
    l2pro_if_action_vlan_t l2pro_if_action_vlan; /* l2 protocol action and vlan*/
    uint32               l2pro_uplink;        /* l2 protocol uplink port*/
    uint32               static_agg_priority;
    uint32               static_agg_is_active;
    uint32               fec_status;
    uint32               cl73_ability;
    uint8                ifg_stats_en;        /* flow statistic include ipg(default enable) */
    uint8                openflow_en;         /* check openflow enable in fea*/
    uint8                openflow_operation_agg; /*1 is add, 0 is del*/
    uint32               openflow_operation_agg_mem_ifindex;
    uint32               carrier_down_holdtime_interval;
    uint32               hash_value_profile_id;
    uint32               hash_value_profile_dir;
    uint32               hash_value_profile_id_agg;
    uint32               hash_value_agg_hash;
    glb_poe_cfg_t        poe;
    uint32               current_g8032_ring_count;
    uint32               g8032_ring_first[2];
    uint32               cfm_lmep_num;
    uint32               cfm_rmep_num;
} tbl_interface_t;

typedef struct
{
    ctclib_hash_t        *if_hash;
    ctclib_slist_t       *if_list;
} tbl_interface_master_t;

#endif /* !__TBL_INTERFACE_DEFINE_H__ */


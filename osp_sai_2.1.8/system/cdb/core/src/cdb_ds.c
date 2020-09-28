
#include <assert.h>
#include "proto.h"
#include "glb_phy_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_msti_port_define.h"
#include "prefix.h"
#include "mstp_define.h"
#include "gen/tbl_erps_domain_define.h"
#include "erps_define.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius_define.h"

void
_cmd_print_para(char** argv , uint8 argc)
{
    int i = 0;
    MY_PRINTF("total: %d", argc);
    
    for(i = 0; i < argc; i++)
    {
        MY_PRINTF("%2d  %15s", i, argv[i]);
    }

    return ;
}

ssm_clock_id_t synce_clock_id_all_0 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
ssm_clock_id_t synce_clock_id_all_1 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void
ptp_desc_ptp_sockaddr(ptp_sockaddr_t *p_skaddr)
{
    char buf[16] = {0};

    //if (p_skaddr->ptp_msg_type == 0)
    {
        printf("sockaddr:capture %d correction %d dest_mac %s ether_type 0x%x ifindex %d is_unicast %d nanoseconds %d ptp_msg_type %s ptp_offset %d replace %d seconds %d seq_id %d ts_offset %d vlan %d vlan_priority %d\n", 
            p_skaddr->capture, p_skaddr->correction, cdb_mac_addr_val2str(buf, sizeof(buf), p_skaddr->dest_mac), p_skaddr->ether_type, 
            p_skaddr->ifindex, p_skaddr->is_unicast, p_skaddr->nanoseconds,ptp_desc_msg_type(p_skaddr->ptp_msg_type), 
            p_skaddr->ptp_offset, p_skaddr->replace, p_skaddr->seconds, p_skaddr->seq_id, 
            p_skaddr->ts_offset, p_skaddr->vlan_id, p_skaddr->vlan_priority);
    }

    return;
}

char *
ptp_desc_oper(uint8 oper)
{
    switch (oper)
    {
    case GLB_PTP_CAPTURE_ONLY:
        return "Capture";
    case GLB_PTP_REPLACE_ONLY:
        return "Replace";
    case GLB_PTP_NULL_OPERATION:
        return "NULL_Operation";
    case GLB_PTP_CORRECTION:
        return "Correction";
    default:
        return "Unknown";
    }
}

char *
ptp_desc_msg_type(ptp_msg_type_t type)
{
    switch (type)
    {
    case PTP_MSG_TYPE_SYNC:
        return "Sync";
    case PTP_MSG_TYPE_DELAY_REQ:
        return "Delay_Req";
    case PTP_MSG_TYPE_PDELAY_REQ:
        return "Pdelay_Req";
    case PTP_MSG_TYPE_PDELAY_RESP:
        return "Pdelay_Resp";

    case PTP_MSG_TYPE_FOLLOW_UP:
        return "Follow_Up";
    case PTP_MSG_TYPE_DELAY_RESP:
        return "Delay_Resp";
    case PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP:
        return "Pdelay_Resp_Follow_Up";
    case PTP_MSG_TYPE_ANNOUNCE:
        return "Announce";
    case PTP_MSG_TYPE_SIGNALING:
        return "Signaling";
    case PTP_MSG_TYPE_MANAGEMENT:
        return "Management";

    default:
        return "Unknown";
    }
}

char *
ptp_desc_ptp_timestamp(const glb_ptp_timestamp_t *p_ptp_timestamp)
{
    static char ptp_timestamp_str[64];

    sal_memset(ptp_timestamp_str, 0, sizeof(ptp_timestamp_str));
    sal_sprintf(ptp_timestamp_str, "TS [(msb%hu lsb%u)s %uns]", p_ptp_timestamp->sec.msb, p_ptp_timestamp->sec.lsb, p_ptp_timestamp->ns);

    return ptp_timestamp_str;
}

char *
ptp_desc_ptp_timeinternal(glb_ptp_timeinternal_t *p_ptp_timeinternal)
{
    static char ptp_timeinternal_str[64];

    sal_memset(ptp_timeinternal_str, 0, sizeof(ptp_timeinternal_str));
    sal_sprintf(ptp_timeinternal_str, "%d:%u:%"PRId64, p_ptp_timeinternal->s.msb, p_ptp_timeinternal->s.lsb, p_ptp_timeinternal->v);

    return ptp_timeinternal_str;
}

char *
ptp_desc_ptp_timeinterval(const glb_ptp_timeinterval_t *p_ptp_timeinterval)
{
    static char ptp_timeinterval_str[64];

    sal_memset(ptp_timeinterval_str, 0, sizeof(ptp_timeinterval_str));
    sal_sprintf(ptp_timeinterval_str, "%d:%u:%"PRId64, p_ptp_timeinterval->s.msb, p_ptp_timeinterval->s.lsb, p_ptp_timeinterval->v);

    return ptp_timeinterval_str;
}

char *
ptp_desc_int64(int64 v)
{
    static char ptp_str[64];
    
    sal_memset(ptp_str, 0x00, sizeof(ptp_str));
    sal_sprintf(ptp_str, "%"PRId64, v);
    return ptp_str;
}

char *
ptp_desc_bool(bool v)
{
    return (v) ? "true" : "false";
}

char *
ptp_desc_clockid(const glb_ptp_clock_id_t clockid)
{
    static char ptp_str[64];
    
    sal_memset(ptp_str, 0x00, sizeof(ptp_str));
    sal_sprintf(ptp_str, "%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX:%.2hhX",
        clockid[0], clockid[1], clockid[2], clockid[3], clockid[4], clockid[5], clockid[6], clockid[7]);
        
    return ptp_str;
}

char *
ptp_desc_time_source(ptp_time_source_t time_source)
{
    switch (time_source)
    {
    case PTP_TIME_SOURCE_ATOMIC_CLOCK:
        return "atomic-clock";
    case PTP_TIME_SOURCE_GPS:
        return "gps";
    case PTP_TIME_SOURCE_TERRESTRIAL_RADIO:
        return "terrestrial-radio";
    case PTP_TIME_SOURCE_PTP:
        return "ptp";
    case PTP_TIME_SOURCE_NTP:
        return "ntp";
    case PTP_TIME_SOURCE_HAND_SET:
        return "hand-set";
    case PTP_TIME_SOURCE_OTHER:
        return "other";
    case PTP_TIME_SOURCE_INTERNAL_OSCILLATOR:
        return "internal-oscillator";

    default:
        return "unknown";
    }
}

uint32 g_ifname_mapping_enable = TRUE;
 
const char *ifname_media_prefix_long[GLB_IFNAME_MEDIA_MAX] = 
{
    "FastEthernet",
    "GigabitEthernet",
    "XgigabitEthernet"
};

const char *ifname_media_prefix_short[GLB_IFNAME_MEDIA_MAX] = 
{
    "Fa",
    "Gi",
    "Xg"
};

const char *cdb_dt_type_str_array[CDB_DT_MAX] =
{
    "UINT",
    "UINT64",
    "INT",
    "INT64",
    "STRING",
    "ENUM8",
    "ENUM32",
    "FLAGS8",
    "FLAGS32",
    "NONE",
    "DS",
    "REFERLIST",
    "COMPOSE",
    "VOID",
    "DOUBLE"
};

const char *glb_bhm_action_strs[GLB_BHM_ACTION_MAX] = 
{
    "reload",
    "shutdown",
    "warning"
};

const char *glb_time_sync_type_strs[GLB_TIME_SYNC_TYPE_MAX] = 
{
    "none",
    "systime",
    "ptp"
};
    
const char *glb_chip_type_strs[GLB_CHIP_TYPE_MAX] = 
{
    "Invalid",
    "Greatbelt",
    "Goldengate",
    "Duet2",
    "Tsingma"
};

const char *cdb_sub_format_str[CDB_SUB_FORMAT_MAX] = 
{
    "raw",
    "protobuf",
    "rawsnmp"
};

const char *cdb_oper_str_array[CDB_OPER_MAX] =
{
    "create",
    "update",
    "read",
    "delete",
    "show",
    "view",
    "buildcfg",
    "sync_refer",
    "readjson",
};

const char *cdb_access_str_array[CDB_ACCESS_MAX] =
{
    "read-only",
    "read-write",
    "sub"
};

const char *glb_port_speed_strs[GLB_PORT_SPEED_MAX] =
{
    "AUTO",
    "10",
    "100",
    "1000",
    "10G",
    "40G",
    "100G",
    "5G",
    "2.5G",
    "25G",
    "50G"
};

const char *glb_port_duplex_strs[GLB_PORT_DUPLEX_MAX] =
{
    "auto",
    "half",
    "full"
};

const char *glb_port_unidir_strs[GLB_PORT_UNIDIR_MAX] =
{
    "disable",
    "enable",
    "rx-only",
    "tx-only",
};

const char *glb_port_pfc_mode[GLB_PFC_FLOWCTRL_MAX] =
{
    "off",
    "on"
};
const char *glb_if_pvlan_type_strs[GLB_IF_PVLAN_TYPE_MAX] =
{
    "none",
    "promiscuous",
    "isolate",
    "community-vlan",
    "host",
    "init"
};

const char *glb_if_mode_strs[GLB_IF_MODE_MAX] =
{
    "routed",
    "switch",
    "other"
};

const char *glb_vlan_port_type_strs[GLB_VLAN_PORT_TYPE_MAX] =
{
    "access",
    "trunk",
    "dot1q-tunnel"
};

const char *glb_vlan_qinq_type_strs[GLB_VLAN_QINQ_TYPE_MAX] =
{
    "basic",
    "selective",
};


const char *glb_agg_mode_strs[GLB_AGG_MODE_MAX] =
{
    "static",
    "active",
    "passive"
};

const char *glb_if_type_strs[GLB_IF_TYPE_MAX] =
{
    "NULL",
    "Port",
    "LAG",
    "VLAN",
    "SubIF",
    "Loopback",
    "Tunnel",
    "L2gre",
    "L3gre",
    "Nvgre",
    "Vxlan",
    "Cpu"
};

const char *glb_phy_type_strs[GLB_PHY_TYPE_MAX] =
{
    "UNKNOWN",
    "1000BASE_T",
    "10BASE_T",
    "100BASE_TX",
    "BASE_PX",
    "BASE_BX10",
    "100BASE_FX",
    "100BASE_LX",
    "1000BASE_T_SFP",
    "1000BASE_CX",
    "1000BASE_LX",
    "1000BASE_SX",
    "1000BASE_ZX",
    "10GBASE_ZR",
    "10GBASE_ER",
    "10GBASE_LRM",
    "10GBASE_LR",
    "10GBASE_SR",
    "10GBASE_PASSIVE_COPPER",
    "10GBASE_ACTIVE_COPPER",
    "40GBASE_CR4",
    "40GBASE_SR4",
    "40GBASE_LR4",
    "40GBASE_XLPPI",
    "100G_AOC",
    "100GBASE_SR4",
    "100GBASE_LR4",
    "100GBASE_ER4",
    "100GBASE_SR10",
    "100G_CWDM4_MSA_FEC",
    "100G_PSM4_SMF",
    "100G_ACC",
    "100G_CWDM4_MSA_NOFEC",
    "100GBASE_CR4",
    "40GBASE_ER4",
    "4X10GBASE_SR",
    "40G_PSM4_SMF",
    "10GBASE_T",
    "2.5GBASE_T",
    "5GBASE_T",
    "10GBASE-KR",
    "40GBASE-KR4",
    "100GBASE-KR4",
    "25GAUI_C2M_AOC_BER_L",
    "25GAUI_C2M_AOC_BER_H",
    "25GBASE_SR",
    "25GBASE_LR",
    "25GBASE_ER",
    "25GAUI_C2M_ACC_BER_L",
    "25GAUI_C2M_ACC_BER_H",
    "25GBASE_CR_CA_L",
    "25GBASE_CR_CA_S",
    "25GBASE_CR_CA_N",
    "25GBASE_PASSIVE_CABLE"
};

const char *glb_ppt_numchg_type_strs[GLB_PPT_NUMCHG_TYPE_MAX] =
{
    "none",
    "10giga",
    "40giga",
    "1giga",
    "25giga",
    "50giga"
};

const char *glb_port_media_type_strs[GLB_PORT_MEDIA_TYPE_MAX] =
{
    "none",
    "sfp",
    "qsfp"
};

const char *glb_card_state_strs[GLB_CARD_STATE_MAX] =
{
    "absent",
    "present",
    "first_created",
    "conflict",
    "hw_insufficient"
};

const char *glb_chassis_type_strs[GLB_CHASSIS_TYPE_MAX] =
{
    "2slot",
    "6slot",
    "14slot",
    "pizzabox"
};

const char *glb_port_media_strs[GLB_PORT_MEDIA_MAX] =
{
    "auto",
    "copper",
    "fiber",
    "sgmii"
};

const char* glb_port_training_status_strs[] = 
{
    "disable",
    "init",
    "training",
    "done"
};

const char *ds_brgif_allowed_vlan_act_strs[DS_BRGIF_ALLOWED_VLAN_ACT_MAX] =
{
    "add",
    "remove",
    "all",
    "none"
};

const char *glb_agg_load_balance_strs[GLB_AGG_LOAD_BALANCE_MAX] =
{
    "src-mac",
    "dst-mac",
    "src-ip",
    "dst-ip",
    "ip-protocol",
    "src-port-l4",
    "dst-port-l4",
    "vxlan-vni",
    "inner-src-mac",
    "inner-dst-mac",
    "inner-src-ip",
    "inner-dst-ip",
    "inner-ip-protocol",
    "inner-src-port-l4",
    "inner-dst-port-l4",
    "gre-key",
    "nvgre-vni",
    "nvgre-flow-id"
};

const char *boot_mode_strs[BOOT_MODE_MAX] =
{
    "flash:",
    "tftp:",
    "tftp6:"
};

const char *mlag_role_strs[MLAG_ROLE_MAX] =
{
    "master",
    "slave"
};

const char *glb_flow_ip_frag_strs[GLB_FLOW_IP_FRAG_MAX] =
{
    "non-fragment",
    "first-fragment",
    "non-or-first-fragment",
    "small-fragment",
    "non-first-fragment",
    "last fragment"
};

const char *glb_port_isolate_strs[GLB_PORT_ISOLATE_MAX] =
{
    "l2",
    "all"
};

const char *glb_pdu_l2pdu_port_action_strs[GLB_PDU_L2PDU_PORT_ACTION_MAX] =
{
    "redirectToCpu",
    "copyToCpu",
    "fwd",
    "discard"
};

const char *glb_vlan_security_action_strs[GLB_VLAN_SECURITY_ACTION_MAX] =
{
    "disable",
    "discard",
    "warn",
    "forward"
};

const char *glb_port_security_violation_strs[GLB_PORT_SECURITY_VIOLATION_MODE_MAX] =
{
    "protect",
    "restrict",
    "shutdown"
};

const char *glb_port_security_enable_strs[GLB_PORT_SECURITY_MAX] =
{
    "disable",
    "enable"
};

const char *arp_inspection_trust_enable_strs[ARPINSP_PORT_MAX] =
{
    "untrust",
    "trust"
};

const char *arp_port_numberlimit_enable_strs[ARP_PORT_NUMBERLIMIT_MAX] =
{
    "disable",
    "enable"
};

const char *arp_port_numberlimit_violate_strs[ARP_PORT_NUMBERLIMIT_VIOLATE_MAX] =
{
    "protect",
    "restrict",
    "errdisable"
};

const char *arpratelimit_exceed_action_str[ARP_RATE_LIMIT_ACTION_MAX] =
{
    "restrict",
    "errdisable",
};

const char *arpratelimit_abnormal_str[ARP_RATE_LIMIT_ABNORMAL_MAX] =
{
    "N",
    "Y",
};

const char *arpratelimit_enable_str[ARP_RATE_LIMIT_STATE_MAX] =
{
    "disable",
    "enable",
};

const char *ptp_state_strs[PTP_STATE_MAX_PLUS] =
{
    "NULL",
    "PTP_STATE_INITIALIZING",
    "PTP_STATE_FAULTY",
    "PTP_STATE_DISABLED",
    "PTP_STATE_LISTENING",
    "PTP_STATE_PRE_MASTER",
    "PTP_STATE_MASTER",
    "PTP_STATE_PASSIVE",
    "PTP_STATE_UNCALIBRATED",
    "PTP_STATE_SLAVE",
    "PTP_STATE_MAX"
};

const char *ptp_bmc_code_strs[PTP_BMC_CODE_MAX] =
{
    "PTP_BMC_CODE_M1",
    "PTP_BMC_CODE_M2",
    "PTP_BMC_CODE_M3",
    "PTP_BMC_CODE_S1",
    "PTP_BMC_CODE_P1",
    "PTP_BMC_CODE_P2"
    //"PTP_BMC_CODE_P2",
    //"PTP_BMC_CODE_MAX"
};

const char *ptp_bmc_expires_strs[PTP_BMC_EXPIRES_MAX] =
{
    "PTP_BMC_EXPIRES_NONE",
    "PTP_BMC_EXPIRES_QUALIFICATION_TIMEOUT_EXPIRES",
    "PTP_BMC_EXPIRES_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES"
    //"PTP_BMC_EXPIRES_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES",
    //"PTP_BMC_EXPIRES_MAX"
};

const char *ptp_device_type_strs[PTP_DEVICE_TYPE_MAX] =
{
    "PTP_DEVICE_OC",
    "PTP_DEVICE_BC",
    "PTP_DEVICE_P2PTC",
    "PTP_DEVICE_E2ETC"
    //"PTP_DEVICE_E2ETC",
    //"PTP_DEVICE_TYPE_MAX"
};

const char *ptp_mechanism_strs[PTP_DELAY_MECHANISM_MAX] =
{
    "PTP_DELAY_MECHANISM_E2E",
    "PTP_DELAY_MECHANISM_P2P",
    "PTP_DELAY_MECHANISM_DISABLE"
    //"PTP_DELAY_MECHANISM_DISABLE",
    //"PTP_DELAY_MECHANISM_MAX"
};

const char *ptp_net_proto_strs[PTP_NET_PROTO_MAX] =
{
    "PTP_NET_PROTO_RESERVED",
    "PTP_NET_PROTO_UDP_IPV4",
    "PTP_NET_PROTO_UDP_IPV6",
    "PTP_NET_PROTO_IEEE_802_3",
    "PTP_NET_PROTO_DEVICE_NET",
    "PTP_NET_PROTO_CONTROL_NET",
    "PTP_NET_PROTO_PROFI_NET",
    "PTP_NET_PROTO_UDP_IPV4_UNICAST",
    //"PTP_NET_PROTO_MAX"
};

const char *tpoam_mp_type_strs[TPOAM_MP_TYPE_MAX] =
{
    "TPOAM_MP_TYPE_MEP",
    "TPOAM_MP_TYPE_MIP",
    //"TPOAM_MP_TYPE_MAX",
};

const char *tpoam_pw_type_strs[TPOAM_PW_TYPE_MAX] =
{
    "TPOAM_PW_TYPE_VPWS",
    "TPOAM_PW_TYPE_VPLS",
    "TPOAM_PW_TYPE_SPE",
    "TPOAM_PW_TYPE_TPE",
    //"TPOAM_PW_TYPE_MAX",
};

const char *tpoam_status_strs[TPOAM_STATUS_MAX] =
{
    "TPOAM_STATUS_INIT",
    "TPOAM_STATUS_FAIL",
    "TPOAM_STATUS_OK",
    "TPOAM_STATUS_DEGRADE",
    //"TPOAM_STATUS_MAX",
};


const char *glb_acl_type_str[GLB_ACL_CONFIG_TYPE_MAX] = {
    "IP ACL",
    "Inner-match",
    "Flow",
    "IPV6 ACL",
    "MAC ACL",
    "MPLS ACL"
};

const char *glb_acl_show_running_str[GLB_ACL_CONFIG_TYPE_MAX] = {
    "ip access-list",
    "inner-match",
    "flow",
    "ipv6 access-list",
    "mac access-list",
    "mpls access-list"
};
#if 0
const char *kernel_arp_type_strs[ARP_TYPE_MAX] =
{
    "imcomplete",    
    "dynamic",
    "static"
};
#else
const char *arp_type_strs[ARP_TYPE_MAX] =
{
    "dynamic",
    "static"
};
#endif

const char *route_type_strs[ROUTE_TYPE_MAX] =
{
    "default",
    "kernel",
    "connect",
    "static",
    "rip",
    "ripng",
    "ospf",
    "ospf6",
    "bgp"    
};

const char *as_strs[AS_MAX] =
{
    "init",
    "incomplete",
    "reachable",
    "refresh"
};

const char *ssh_version_strs[SSH_VERSION_MAX] =
{
    "v1",
    "v2",
    "all"
};

const char *syslog_facility_strs[SYSLOG_FACILITY_MAX] = 
{
    "kern", /* 0 */
    "user",
    "mail",
    "daemon",
    "auth",
    "syslog",
    "lpr",
    "news",
    "uucp",
    "cron",
    "authpriv",
    "ftp",
    "reserved", /* 12 */
    "reserved", /* 13 */
    "reserved", /* 14 */
    "reserved", /* 15 */
    "local0", /* 16 */
    "local1",
    "local2",
    "local3",
    "local4",
    "local5",
    "local6",
    "local7",
};

const char *syslog_severity_strs[SYSLOG_SEVERITY_MAX] = 
{
    "emergency",
    "alert",
    "critical",
    "error",
    "warning",
    "notice",
    "information",
    "debug",
};

const char *frr_syslog_severity_strs[SYSLOG_SEVERITY_MAX] = 
{
    "emergencies",
    "alerts",
    "critical",
    "errors",
    "warnings",
    "notifications",
    "informational",
    "debugging",
};

const char *syslog_timestamp_strs[SYSLOG_TIMESTAMP_MAX] = 
{
    "none",
    "bsd",
    "iso",
    "date",
};

const char *login_method_strs[LOGIN_METHOD_MAX] = 
{
    "nologin",
    "login",
    "local",
};

const char *parity_strs[PARITY_MAX] = 
{
    "none",
    "even",
    "odd",
};

const char *authen_method_strs[AUTHEN_METHOD_MAX] = 
{
    "none",
    "enable",
    "line",
    "local",
    "radius",
    "tacplus",
};

const char *author_method_strs[AUTHOR_METHOD_MAX] = 
{
    "none",
    "local",
    "radius",
    "tacplus",
};

const char *account_method_strs[ACCOUNT_METHOD_MAX] = 
{
    "none",
    "radius",
    "tacplus",
};

const char *accountcmd_method_strs[ACCOUNTCMD_METHOD_MAX] = 
{
    "none",
    "tacplus",
};

const char *key_type_strs[KEY_TYPE_MAX] = 
{
    "public",
    "private",
};

const char *agent_proc_option_strs[AGENT_PROC_OPTION_MAX] = 
{
    "append",
    "replace",
    "keep",
    "drop",
};

const char *dhcbinding_state_strs[DHCBINDING_STATE_MAX] =
{
    "request",
    "running",
    "all"
};

const char *dhcbinding_type_strs[DHCBINDING_TYPE_MAX] =
{
    "learning",
    "configed",
    "all"
};

const char *auth_server_type_strs[AUTH_SERVER_TYPE_MAX] =
{
    "radius",
    "tacplus"
};

const char *auth_session_status_strs[AUTH_SESSION_STATUS_MAX] =
{
    "initialize",
    "inprogress",
    "success",
    "failed",
    "denied"
};

const char *snmp_version_strs[SNMP_VERSION_MAX] =
{
    "v1",
    "v2c",
    "v3",
    "all"
};

const char *snmp_view_class_strs[SNMP_VIEW_CLASS_MAX] =
{
    "included",
    "excluded"
};

const char *snmp_community_strs[SNMP_COMMUNITY_MAX] =
{
    "read-only",
    "read-write"
};

const char *snmp_usm_level_strs[SNMP_LEVEL_MAX] =
{
    "none",
    "noauth",
    "auth",
    "priv"
};

const char *snmp_authen_strs[SNMP_AUTHENTICATION_MAX] = 
{
    "none",
    "md5",
    "sha"
};

const char *snmp_privacy_strs[SNMP_PRIVACY_MAX] = 
{
    "none",
    "des",
    "aes"
};

const char *scheduler_mode_strs[SCHEDULER_MODE_MAX] =
{
    "sp",
    "dwrr"
};

const char *mstp_fea_state_strs[GLB_BR_PORT_STATE_MAX] =
{
    "DISABLED",
    "LISTENING",
    "LEARNING",
    "FORWARDING",
    "BLOCKING"
};

const char *mstp_port_role_strs[MSTP_PORT_ROLE_MAX] =
{
    "Disabled",
    "Alternate",
    "Rootport",
    "Designated",
    "Backup",
    "Masterport"
};

const char *mstp_bpdu_role_strs[BPDU_ROLE_MAX] =
{
    "Unknown",
    "Alternate/Backup",
    "Rootport",
    "Designated"
};

const char *mstp_port_state_strs[MSTP_PORT_STATE_MAX] =
{
    "discarding",
    "listening",
    "learning",
    "forwarding",
    "blocking"
};

const char *mstp_tc_state_strs[MSTP_TC_STATE_MAX] =
{
    "tc_inactive",
    "tc_active"
};

const char *mstp_type_strs[MSTP_TYPE_MAX] =
{
    "STP",
    "RSTP",
    "MSTP"
};

const char *mstp_type_lower_strs[MSTP_TYPE_MAX] =
{
    "stp",
    "rstp",
    "mstp"
};

const char *mstp_edgeport_strs[MSTP_EDGEPORT_MAX] =
{
    "disable",
    "enable",
    "default"
};

const char *mstp_path_cost_strs[MSTP_PATH_COST_MAX] =
{
    "dot1t",
    "dot1d-1998"
};

const char *mstp_link_strs[MSTP_LINK_MAX] =
{
    "shared",
    "p2p",
    "auto"
};

const char *mstp_info_strs[MSTP_INFO_MAX] =
{
    "disabled",
    "mine",
    "aged",
    "received"
};

const char *mstp_rcvd_info_strs[MSTP_RCVD_INFO_MAX] =
{
    "superior_designated",
    "repeated_designated",
    "inferior_designated",
    "inferior_rootalternate",
    "other"
};

const char *pti_state_strs[PTI_STATE_MAX] =
{
    "TICK",
    "ONE_SECOND"
};

const char *prx_state_strs[PRX_STATE_MAX] =
{
    "DISCARD",
    "RECEIVE"
};

const char *ppm_state_strs[PPM_STATE_MAX] =
{
    "CHECKING_RSTP",
    "SELECTING_STP",
    "SENSING"
};

const char *bdm_state_strs[BDM_STATE_MAX] =
{
    "NOT_EDGE",
    "EDGE"
};

const char *ptx_state_strs[PTX_STATE_MAX] =
{
    "TRANSMIT_INIT",
    "TRANSMIT_PERIODIC",
    "IDLE",
    "TRANSMIT_RSTP",
    "TRANSMIT_TCN",
    "TRANSMIT_CONFIG",
};

const char *pim_state_strs[PIM_STATE_MAX] =
{
    "DISABLED",
    "AGED",
    "UPDATE",
    "CURRENT",
    "OTHER",
    "NOT_DESIGNATED",
    "INFERIOR_DESIGNATED",
    "REPEATED_DESIGNATED",
    "SUPERIOR_DESIGNATED",
    "RECEIVE"
};

const char *prs_state_strs[PRS_STATE_MAX] =
{
    "INIT_BRIDGE",
    "ROLE_SELECTION"
};

const char *prt_state_strs[PRT_STATE_MAX] =
{
    "INIT_PORT",
#if 0
    /* We omit disable port state */
    "DISABLED_PORT",
#endif
    "PRT_DISABLED_PORT",
    "MASTER_PROPOSED",
    "MASTER_AGREED",
    "MASTER_SYNCED",
    "MASTER_RETIRED",
    "MASTER_FORWARD",
    "MASTER_LEARN",
    "MASTER_DISCARD",
    "MASTER_PORT",
    "ROOT_PROPOSED",
    "ROOT_AGREED",
    "REROOT",
    "ROOT_PORT",
    "REROOTED",
    "ROOT_LEARN",
    "ROOT_FORWARD",
    "DESIGNATED_PROPOSE",
    "DESIGNATED_SYNCED",
    "DESIGNATED_RETIRED",
    "DESIGNATED_PORT",
    "DESIGNATED_DISCARD",
    "DESIGNATED_LEARN",
    "DESIGNATED_FORWARD",
    "ALTERNATE_PROPOSED",
    "ALTERNATE_AGREED",
    "ALTERNATE_PORT",
    "BACKUP_PORT",
    "BLOCK_PORT"
};

const char *pst_state_strs[PST_STATE_MAX] =
{
    "Discarding",
    "Listening",
    "Learning",
    "Forwarding"
};

const char *tcm_state_strs[TCM_STATE_MAX] =
{
    "INACTIVE",
    "DETECTED",
    "ACTIVE",
    "ACKNOWLEDGED",
    "PROPAGATING",
    "NOTIFIED_TC",
    "NOTIFIED_TCN"
};

const char *pti_event_strs[PTI_EVENT_MAX] =
{
    "BEGIN",
    "TickTrue"
};

const char *prx_event_strs[PRX_EVENT_MAX] =
{
    "BEGIN",
    "RcvdBpduPortEnabled",
    "RcvdBpduPortEnabledNotRcvdMsg"
};

const char *ppm_event_strs[PPM_EVENT_MAX] =
{
    "BEGIN",
    "MCheck",
    "SendRSTPRcvdSTP",
    "RstpVersionNotSendRstpRcvdRstp"
};

const char *bdm_event_strs[BDM_EVENT_MAX] =
{
    "BEGINAdminEdge",
    "BEGINNotAdminEdge",
    "EdgedelayTimeoutEdge",
    "RecvBPDU"
};

const char *ptx_event_strs[PTX_EVENT_MAX] =
{
    "BEGIN",
    "HelloWhenZero",
    "SendRstp",
    "SendTcn",
    "SendConfig"
};

const char *pim_event_strs[PIM_EVENT_MAX] =
{
    "BEGIN",
    "RcvdMsg",
    "SelectedUpdtInfo",
    "RcvdInfoWhileZero",
    "PortEnabled",
    "RcvdMsgNotUpdtInfo"
};

const char *prs_event_strs[PRS_EVENT_MAX] =
{
    "BEGIN",
    "Reselect"
};
  
const char *prt_event_strs[PRT_EVENT_MAX] =
{
    "BEGIN",
    "RoleDisabled",
    "RoleMasterport",
    "RoleRootport",
    "RoleDesignated",
    "RoleAlternate",
    "RoleBackup",
    "DesignatedSynced",
    "DesignatedDiscard",
    "DesignatedLearn",
    "DesignatedForward",
    "MasterSynced",
    "MasterDiscard",
    "MasterLearn",
    "MasterForward",
    "RootLearn",
    "RootForward",
    "Porposing",
    "ProposedNotAgree",
    "AllSyncedNotAgree"
};
  
const char *pst_event_strs[PST_EVENT_MAX] =
{
    "BEGIN",
    "Listen",
    "Learn",
    "NotLearn",
    "Forward",
    "NotForward"
};

const char *tcm_event_strs[TCM_EVENT_MAX] =
{
    "BEGIN",
    "RcvdTc",
    "RcvdTcn",
    "RcvdTcAck",
    "TcProp",
    "TcPropNotOperEdge",
    "RoleForwardNotOperEdge",
    "OperEdge",
    "RoleNotRootPortNotDesignatedPort",
    "LearnNotFdbFlush"
};

const char *errdis_rsn_strs[ERRDIS_RSN_MAX] =
{
    "arp-numberlimit",
    "arp-ratelimit",
    "bpduguard",
    "bpduloop",
    "fdb-loop",
    "link-monitor-failure",
    "link-flap",
    "loopback-detection",
    "monitor-link",
    "oam-remote-failure",
    "port-security",
    "reload-delay",
    "udld"
};

const char *lldp_if_mode_strs[LLDP_IF_MODE_MAX] =
{
    "disable",
    "txrx",
    "txonly",
    "rxonly"
};

const char *glb_reboot_type_strs[GLB_REBOOT_TYPE_MAX] =
{
    "other",
    "cold",
    "warm"
};

const char *glb_warmboot_type_strs[GLB_WARMBOOT_TYPE_MAX] =
{
    "normal",
    "kernel",
    "fast",
    "hitless"
};

const char *flush_fdb_type_strs[FLUSH_FDB_TYPE_MAX] =
{
    "fdb",
    "macfilter",
    "psfdb"
};

const char *flush_fdb_subtype_strs[FLUSH_FDB_SUBTYPE_MAX] =
{
    "static",
    "dynamic",
    "multicast"
};

const char *flush_fdb_mode_strs[FLUSH_FDB_MODE_MAX] =
{
    "all",
    "port",
    "vlan",
    "port_vlan",
    "mac"
};

const char *glb_br_port_state_strs[GLB_BR_PORT_STATE_MAX] =
{
    "disabled",
    "listening",
    "learning",
    "forwarding",
    "blocking"
};

const char *erps_state_strs[ERPS_STATE_MAX] =
{
    "disable",
    "enable",
    "enable",
    "enable",
    "enable",
    "enable",
    "enable"
};

const char *erps_msg_type_strs[ERPS_MAX_PKT] =
{
    "null",
    "null",
    "null",
    "null",
    "null",
    "hello",
    "ring-up-flush-fdb",
    "ring-down-flush-fdb",
    "link-down",
    "edge-hello",
    "major-fault"
};

const char *erps_node_role_strs[ERPS_NODE_ROLE_MAX] =
{
    "null",
    "master",
    "transit"
};

const char *erps_edge_node_role_strs[ERPS_EDGE_NODE_ROLE_MAX] =
{
    "null",
    "edge",
    "assistant-edge"
};

const char *erps_ring_level_strs[ERPS_RING_LEVEL_MAX] =
{
    "primary",
    "sub"
};

const char *vlanclass_rule_protype_strs[VLANCLASS_PROTYPE_MAX] =
{
    "null",
    "ip",
    "ipv6",
    "mpls",
    "mpls-mcast",
    "arp",
    "rarp",
    "eapol",
    "8021ag",
    "slow-protocol",
    "pppoe",
    "flexl3",
};

const char *ospf_nssa_translate_role_strs[OSPF_NSSA_TRANSLATE_ROLE_MAX] = 
{
    "default (translate-candidate)",
    "translate-candidate",
    "translate-never",
    "translate-always"
};

const char *ospf_vlink_auth_type_strs[OSPF_VLINK_AUTH_TYPE_MAX] = 
{
    "default (null)",
    "null",
    "simple",
    "md5"
};

const char *glb_vty_mode_strs[GLB_VTY_MODE_MAX] =
{
    "exec",
    "config",
    "interface",
    "router"
};

/*xx.xxx to uint32, expand 1000*/
int32 
cdb_percent_point_str2value(char *str, uint32 *value_out)
{
    int32 value = 0;
    uint32 len = 0;
    while (str[len] != '\0')        
    {
        if (str[len] != '.')
        {
            len++;                        
            continue;
        }
        str[len] = '\0';
        value = sal_atoi(str);
        *value_out = value;
        str[len] = '.';
        len++;
        break;
    }
    
    if  (len < sal_strlen(str))
    {
        value = sal_atoi(str + len);
        /* A.B */
        if (1 == sal_strlen(str + len))
        {
            *value_out = *value_out * 1000 + value * 100;
        }
        else if (2 == sal_strlen(str + len))
        {
            *value_out= *value_out * 1000 + value * 10;
        }
        else if (3 == sal_strlen(str + len))
        {
            *value_out= *value_out * 1000 + value;
        }
        else 
        {
            return PM_E_FAIL;
        }
    } 
    else 
    {
        value = sal_atoi(str);
        *value_out = value * 1000;
    }

    return PM_E_NONE;
}

const char*
cdb_if_speed_str(uint32 speed, uint32 oper_speed)
{
    switch(speed)
    {
    case GLB_SPEED_AUTO:
        switch(oper_speed)
        {
        case GLB_SPEED_10M:
            return "a-10";

        case GLB_SPEED_100M:
            return "a-100";

        case GLB_SPEED_1G:
            return "a-1000";

        case GLB_SPEED_2_5G:
            return "a-2500";

        case GLB_SPEED_5G:
            return "a-5000";
            
        case GLB_SPEED_10G:
            return "a-10000";

        case GLB_SPEED_40G:
            return "a-40000";

        case GLB_SPEED_100G:
            return "a-100000";
            
        case GLB_SPEED_50G:
            return "a-50000";

        case GLB_SPEED_25G:
            return "a-25000";
            
        default :
            return "auto";
        }

    case GLB_SPEED_10M:
        return "10";

    case GLB_SPEED_100M:
        return "100";

    case GLB_SPEED_1G:
        return "1000";

    case GLB_SPEED_2_5G:
        return "2500";

    case GLB_SPEED_5G:
        return "5000";
        
    case GLB_SPEED_10G:
        return "10000";

    case GLB_SPEED_40G:
        return "40000";

    case GLB_SPEED_100G:
        return "100000";

    case GLB_SPEED_50G:
        return "50000";

    case GLB_SPEED_25G:
        return "25000";

    default:
        return "unknown";
    }
}

const char*
cdb_if_duplex_str(uint32 duplex, uint32 oper_duplex)
{
    switch(duplex)
    {
    case GLB_DUPLEX_AUTO:
        switch(oper_duplex)
        {
            case GLB_DUPLEX_HALF:
                return "a-half";

            case GLB_DUPLEX_FULL:
                return "a-full";

            default :
                return "auto";
        }

    case GLB_DUPLEX_HALF:
        return "half";

    case GLB_DUPLEX_FULL:
        return "full";

    default :
        return "unknown";
    }
}

int32
cdb_int_str2val(const char *str, int32 *ret)
{
    int32 i;
    int32 len;
    int32 digit = 0;
    int32 minus = 0;
    uint32 limit, remain, hex = 0;
    uint32 max = GLB_MAX_UINT32_VALUE;
    uint32 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen (str);
    if (*str == '+')
    {
        str++;
        len--;
    }
    else if (*str == '-')
    {
        str++;
        len--;
        minus = 1;
        max = max / 2 + 1;
    }
    else if (*str == '0')
    {
        str++;
        len--;
        if (*str =='x')
        {
            str++;
            len--;
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

    limit = max / 10;
    remain = max % 10;

    if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        if (*str < '0' || (*str > '9' && *str < 'A') || (*str > 'F' && *str < 'a') ||*str >'f')
        {
            return -1;
        }

        if (*str <= '9')
        {
            digit = (*str++) - '0';
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            digit = (*str++) - 'A' + 10;
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            digit = (*str++) - 'a' + 10;
        }

        if(total  > limit || (total == limit && digit > remain))
        {
            return -1;
        }
        if (hex == 1)
        {
            total = total * 16 + digit;
        }
        else
        {
            total = total * 10 + digit;
        }
    }

    *ret = 0;
    if(minus && (total == 0))
    {
        return 0;
    }
    if (minus)
    {
        return -total;
    }
    else
    {
        return total;
    }
}


uint32
cdb_uint_str2val(const char *str, int32 *ret)
{
    int32 i;
    int32 len;
    int32 digit = 0;
    uint32 limit, remain, hex = 0;
    uint32 max = GLB_MAX_UINT32_VALUE;
    uint32 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen (str);
    if (*str == '+')
    {
        str++;
        len--;
    }
    else if (*str == '-')
    {
        return 0;
    }
    else if (*str == '0')
    {
        str++;
        len--;
        if (*str =='x')
        {
            str++;
            len--;
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

    limit = max / 10;
    remain = max % 10;

    if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        if (*str < '0' || (*str > '9' && *str < 'A') || (*str > 'F' && *str < 'a') ||*str >'f')
        {
            return -1;
        }

        if (*str <= '9')
        {
            digit = (*str++) - '0';
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            digit = (*str++) - 'A' + 10;
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            digit = (*str++) - 'a' + 10;
        }

        if(total  > limit || (total == limit && digit > remain))
        {
            return -1;
        }
        if (hex == 1)
        {
            total = total * 16 + digit;
        }
        else
        {
            total = total * 10 + digit;
        }
    }

    *ret = 0;

    return total;

}

uint64
cdb_uint64_str2val(const char *str, int32 *ret)
{
    
    int32 i;
    int32 len;
    int32 digit = 0;
    uint64 limit, remain, hex = 0;
    uint64 max = GLB_MAX_UINT64_VALUE;
    uint64 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen (str);
    if (*str == '+')
    {
        str++;
        len--;
    }
    else if (*str == '-')
    {
        return 0;
    }
    else if (*str == '0')
    {
        str++;
        len--;
        if (*str =='x')
        {
            str++;
            len--;
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

    limit = max / 10;
    remain = max % 10;

    if (len < DECIMAL_STRLEN_MIN || len > UINT64_DECIMAL_STRLEN_MAX)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        if (*str < '0' || (*str > '9' && *str < 'A') || (*str > 'F' && *str < 'a') ||*str >'f')
        {
            return -1;
        }

        if (*str <= '9')
        {
            digit = (*str++) - '0';
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            digit = (*str++) - 'A' + 10;
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            digit = (*str++) - 'a' + 10;
        }

        if(total  > limit || (total == limit && digit > remain))
        {
            return -1;
        }
        if (hex == 1)
        {
            total = total * 16 + digit;
        }
        else
        {
            total = total * 10 + digit;
        }
    }

    *ret = 0;

    return total;

}

int64
cdb_int64_str2val(const char *str, int32 *ret)
{
    int32 i;
    int32 len;
    int32 digit = 0;
    int32 minus = 0;
    int64 limit, remain, hex = 0;
    int64 max = GLB_MAX_INT64_VALUE;
    int64 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen (str);
    if (*str == '+')
    {
        str++;
        len--;
    }
    else if (*str == '-')
    {
        str++;
        len--;
        minus = 1;
        max = max / 2 + 1;
    }
    else if (*str == '0')
    {
        str++;
        len--;
        if (*str =='x')
        {
            str++;
            len--;
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

    limit = max / 10;
    remain = max % 10;

    if (len < DECIMAL_STRLEN_MIN || len > UINT64_DECIMAL_STRLEN_MAX)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        if (*str < '0' || (*str > '9' && *str < 'A') || (*str > 'F' && *str < 'a') ||*str >'f')
        {
            return -1;
        }

        if (*str <= '9')
        {
            digit = (*str++) - '0';
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            digit = (*str++) - 'A' + 10;
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            digit = (*str++) - 'a' + 10;
        }

        if(total  > limit || (total == limit && digit > remain))
        {
            return -1;
        }
        if (hex == 1)
        {
            total = total * 16 + digit;
        }
        else
        {
            total = total * 10 + digit;
        }
    }

    *ret = 0;
    if(minus && (total == 0))
    {
        return -1;
    }
    if (minus)
    {
        return -total;
    }
    else
    {
        return total;
    }
}

double
cdb_double_str2val(const char *str, int32 *ret)
{
    int32 i;
    int32 len;
    int32 digit = 0;
    uint64 limit, remain, hex = 0;
    uint64 max = GLB_MAX_UINT64_VALUE;
    uint64 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen (str);
    if (*str == '+')
    {
        str++;
        len--;
    }
    else if (*str == '-')
    {
        return 0;
    }
    else if (*str == '0')
    {
        str++;
        len--;
        if (*str =='x')
        {
            str++;
            len--;
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

    limit = max / 10;
    remain = max % 10;

    if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        if (*str < '0' || (*str > '9' && *str < 'A') || (*str > 'F' && *str < 'a') ||*str >'f')
        {
            return -1;
        }

        if (*str <= '9')
        {
            digit = (*str++) - '0';
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            digit = (*str++) - 'A' + 10;
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            digit = (*str++) - 'a' + 10;
        }

        if(total  > limit || (total == limit && digit > remain))
        {
            return -1;
        }
        if (hex == 1)
        {
            total = total * 16 + digit;
        }
        else
        {
            total = total * 10 + digit;
        }
    }

    *ret = 0;

    return total;

}


int32
cdb_enum_str2val(const char **array, uint32 array_size, const char *str)
{
    uint32 i = 0;
    
    for (i = 0; i < array_size; i++)
    {
        if (!sal_strcmp(array[i], str))
        {
            return i;
        }
    }
    return -1;
}

const char*
cdb_enum_val2str(const char **array, uint32 array_size, uint32 value)
{
    if (value >= array_size)
    {
        return "invalid";
    }
    return array[value];
}

#define CHECK_RANGE_VALUE(_value, _desc, _min, _max, _err_desc) \
if ((_value) < (_min) || (_value) > (_max)) \
{ \
    sal_snprintf((_err_desc), MAX_CMD_RET_LEN, "%s is out of [%d, %d]", (_desc), (_min), (_max)); \
    return PM_E_CFG_INVALID_PARAM; \
}

static inline int32
get_char_count(const char *str, const char *c)
{
    int32 count = 0;
    int32 len = sal_strlen(str);
    int32 i = 0;

    for (i = 0; i < len; i++)
    {
        if (str[i] == c[0])
        {
            count++;
        }
    }

    return count;
}

int32
_cdb_bitmap_split_token(char *str, const char *delimiter, uint32 max_token, char **token_array, uint32 *token_count)
{
    char *token = NULL;
    char *saveptr = NULL;
    uint32 count = 0;
    
    token = sal_strtok_r(str, delimiter, &saveptr);
    while (NULL != token)
    {
        if (count >= max_token)
        {
            return PM_E_FAIL;
        }
        token_array[count] = token;
        count++;
        token = sal_strtok_r(NULL, delimiter, &saveptr);
    }
    *token_count = count;

    return PM_E_NONE;
}

int32
cdb_bitmap_str2val(const char *str, uint32 min, uint32 max, uint32 *bmp, uint32 bmp_len, char *err_desc)
{
    char copy_str[MAX_CMD_STR_LEN];
    char *token[MAX_RANGE_SEGMENTS];
    uint32 token_num = 0;
    const char *comma = ",";
    const char *hyphen = "-";
    char *segment = NULL;
    char *segment_stop = NULL;
    int32 rc = PM_E_NONE;
    uint32 i = 0;
    uint32 hyphen_count = 0;
    uint32 start = 0;
    uint32 stop = 0;
    uint32 offset = 0;

    sal_memset(token, 0, sizeof(token));
    if (sal_strlen(str) >= MAX_CMD_STR_LEN)
    {
        SAL_ASSERT(0);
    }

    sal_strncpy(copy_str, str, MAX_CMD_STR_LEN);

    if (sal_strspn(copy_str, "0123456789-,") != sal_strlen(copy_str))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "String only support characters '0-9,-'");
        return PM_E_CFG_INVALID_PARAM;
    }
    if (!sal_isdigit(copy_str[0]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "The first character should be '0-9'");
        return PM_E_CFG_INVALID_PARAM;
    }
    if (!sal_isdigit(copy_str[sal_strlen(copy_str)-1]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "The last character should be '0-9'");
        return PM_E_CFG_INVALID_PARAM;
    }

    rc = _cdb_bitmap_split_token(copy_str, comma, MAX_RANGE_SEGMENTS, token, &token_num);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Range segment delimited with ',' exceeds max number %u", MAX_RANGE_SEGMENTS);
        return PM_E_CFG_INVALID_PARAM;
    }

    for (i = 0; i < token_num; i++)
    {
        segment = token[i];
        hyphen_count = get_char_count(segment, hyphen);
        if (0 == hyphen_count)
        {
            start = sal_atoi(segment);
            CHECK_RANGE_VALUE(start, "Vlan", min, max, err_desc);
            GLB_BMP_SET(bmp, start);
        }
        else if (1 == hyphen_count)
        {
            start = sal_atoi(segment);
            segment_stop = sal_strstr(segment, hyphen);
            segment_stop++;
            stop = sal_atoi(segment_stop);
            CHECK_RANGE_VALUE(start, "Vlan", min, max, err_desc);
            CHECK_RANGE_VALUE(stop, "Vlan", min, max, err_desc);
            if (start >= stop)
            {
                sal_snprintf(err_desc, MAX_CMD_RET_LEN, "End value %u is not larger than start value %u", stop, start);
                return PM_E_CFG_INVALID_PARAM;
            }
            for (offset = start; offset <= stop; offset++)
            {
                GLB_BMP_SET(bmp, offset);
            }
        }
        else 
        {
            sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Range segment %s has %d count of '-'", segment, hyphen_count);
            return PM_E_CFG_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}

char*
cdb_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len)
{
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    char tmp[256];
    uint32 used = 0;
    uint32 tmp_len = 0;

    str[0] = 0;
    for (offset = 0; offset < GLB_UINT32_BITS * bmp_len; offset++)
    {
        if (GLB_BMP_ISSET(bmp, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(bmp, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
    }
    return str;
}

/*added by wanh for bug 52727, 2019-09-16*/
int32
cdb_mpls_label_bitmap_str2val(const char *str, uint32 min, uint32 max, uint32 *bmp, uint32 bmp_len, char *err_desc)
{
    char copy_str[MAX_CMD_STR_LEN];
    char *token[MAX_RANGE_SEGMENTS];
    uint32 token_num = 0;
    const char *comma = ",";
    const char *hyphen = "-";
    char *segment = NULL;
    char *segment_stop = NULL;
    int32 rc = PM_E_NONE;
    uint32 i = 0;
    uint32 hyphen_count = 0;
    uint32 start = 0;
    uint32 stop = 0;
    uint32 offset = 0;

    sal_memset(token, 0, sizeof(token));
    if (sal_strlen(str) >= MAX_CMD_STR_LEN)
    {
        SAL_ASSERT(0);
    }

    sal_strncpy(copy_str, str, MAX_CMD_STR_LEN);

    if (sal_strspn(copy_str, "0123456789-,") != sal_strlen(copy_str))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "String only support characters '0-9,-'");
        return PM_E_CFG_INVALID_PARAM;
    }
    if (!sal_isdigit(copy_str[0]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "The first character should be '0-9'");
        return PM_E_CFG_INVALID_PARAM;
    }
    if (!sal_isdigit(copy_str[sal_strlen(copy_str)-1]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "The last character should be '0-9'");
        return PM_E_CFG_INVALID_PARAM;
    }

    rc = _cdb_bitmap_split_token(copy_str, comma, MAX_RANGE_SEGMENTS, token, &token_num);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Range segment delimited with ',' exceeds max number %u", MAX_RANGE_SEGMENTS);
        return PM_E_CFG_INVALID_PARAM;
    }

    for (i = 0; i < token_num; i++)
    {
        segment = token[i];
        hyphen_count = get_char_count(segment, hyphen);
        if (0 == hyphen_count)
        {
            start = sal_atoi(segment);
            CHECK_RANGE_VALUE(start, "mpls label", min, max, err_desc);
            GLB_BMP_SET(bmp, start);
        }
        else if (1 == hyphen_count)
        {
            start = sal_atoi(segment);
            segment_stop = sal_strstr(segment, hyphen);
            segment_stop++;
            stop = sal_atoi(segment_stop);
            CHECK_RANGE_VALUE(start, "mpls label", min, max, err_desc);
            CHECK_RANGE_VALUE(stop, "mpls label", min, max, err_desc);
            if (start >= stop)
            {
                sal_snprintf(err_desc, MAX_CMD_RET_LEN, "End value %u is not larger than start value %u", stop, start);
                return PM_E_CFG_INVALID_PARAM;
            }
            for (offset = start; offset <= stop; offset++)
            {
                GLB_BMP_SET(bmp, offset);
            }
        }
        else 
        {
            sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Range segment %s has %d count of '-'", segment, hyphen_count);
            return PM_E_CFG_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}
/*wanh end*/

/*added by wanh for bug 52727, 2019-09-16*/
char*
cdb_mpls_label_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len)
{
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    char tmp[256];
    uint32 used = 0;
    uint32 tmp_len = 0;

    str[0] = 0;
    for (offset = 0; offset < GLB_UINT32_BITS * bmp_len; offset++)
    {
        if (GLB_BMP_ISSET(bmp, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(bmp, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 8);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 16);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
    }
    return str;
}
/*wanh end*/

int32
cdb_mac_addr_str2val(uint8 *mac, const char *str)
{
    if (3 != sal_sscanf(str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

    return PM_E_NONE;
}

int32
cdb_mac_addr_masked(uint8 *mac, uint8 *mask)
{
    int i;
    for (i = 0; i < 6; i++)
    {
        mac[i] &= ~mask[i];
        mac[i] &= ~mask[i];
    }

    return PM_E_NONE;
}

int32
cdb_ip_addr_masked(uint32 *add, uint32 *add_mask)
{
    *add &= ~(*add_mask);
    return PM_E_NONE;
}

char*
cdb_mac_addr_val2str(char *str, uint32 str_len, const uint8 *mac)
{
    sal_sprintf(str, "%.04hx.%.04hx.%.04hx",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]));
    return str;
}

char*
cdb_mac_addr_vtysh_val2str(char *str, uint32 str_len, const uint8 *mac)
{
    sal_sprintf(str, "%hx:%hx:%hx:%hx:%hx:%hx",
        ((mac[0])),
        ((mac[1])),
        ((mac[2])),
        ((mac[3])),
        ((mac[4])), 
        ((mac[5])));
    return str;
}

char*
cdb_addr_ipv4_val2str(char *str, uint32 str_len, const addr_ipv4_t *addr)
{
    str[0] = CMD_ZERO_CHAR;
    sal_inet_ntop(AF_INET, (void*)addr, str, str_len);
    return str;
}

int32
cdb_addr_ipv4_str2val(addr_ipv4_t *addr, const char *str, uint32 str_len)
{
    /* try IPv4 */
    sal_memset(addr, 0, sizeof(addr_ipv4_t));
    if (sal_inet_pton(AF_INET, str, addr) == 1) {
        return PM_E_NONE;
    }

    return PM_E_FAIL;
}

int32
cdb_prefix_ipv4_str2val(char *addr_str, char *masklen_str, prefix_ipv4_t *p_prefix, char *err_desc)
{
    int32 value = 0;
    int32 rc = PM_E_NONE;

    p_prefix->family = AF_INET;
    p_prefix->pad = 0;

    rc = sal_inet_pton(AF_INET, addr_str, &p_prefix->prefix);
    if (rc != 1)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid IP address %s", addr_str);
        return PM_E_CFG_INVALID_PARAM;
    }

    value = cdb_int_str2val(masklen_str, &rc);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Can't convert %s to integer", masklen_str);
        return PM_E_CFG_INVALID_PARAM;
    }

    if (IPV4_MIN_PREFIXLEN > value || IPV4_MAX_PREFIXLEN < value)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid prefix length %d", value);
        return PM_E_CFG_INVALID_PARAM;
    }
    p_prefix->prefixlen = value;
    
    return PM_E_NONE;
}

int32
cdb_prefix_ipv4_str2val_wo_masklen(char *str, prefix_ipv4_t *p_prefix)
{
    
    return PM_E_NONE;
}

char*
cdb_prefix_ipv4_val2str(char *str, uint32 str_len, const prefix_ipv4_t *prefix)
{
    uint32 len = 0;
    
    str[0] = CMD_ZERO_CHAR;
    sal_inet_ntop(AF_INET, (void*)&prefix->prefix, str, str_len);
    len = sal_strlen(str);
    
    sal_sprintf(str + len, CMD_KEY_DIV_STR"%u", prefix->prefixlen);
    return str;
}

int32
cdb_prefix_str2val(char *addr_str, char *masklen_str, prefix_t *p_prefix, char *err_desc)
{
    int32 value = 0;
    int32 rc = PM_E_NONE;

    sal_memset(p_prefix, 0, sizeof(prefix_t));
    
    p_prefix->family = AF_INET;
    p_prefix->pad = 0;

    rc = sal_inet_pton(AF_INET, addr_str, &p_prefix->u.prefix4);
    if (rc == 1)
    {
        value = cdb_int_str2val(masklen_str, &rc);
        if (rc < 0)
        {
            sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Can't convert %s to integer", masklen_str);
            return PM_E_CFG_INVALID_PARAM;
        }

        if (IPV4_MIN_PREFIXLEN > value || IPV4_MAX_PREFIXLEN < value)
        {
            sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid prefix length %d", value);
            return PM_E_CFG_INVALID_PARAM;
        }
        p_prefix->prefixlen = value;
    }
    else
    {
        /*ipv6*/
        p_prefix->family = AF_INET6;
        p_prefix->pad = 0;
        
        rc = sal_inet_pton(AF_INET6, addr_str, &p_prefix->u.prefix6);
        if (1 == rc)
        {
            value = cdb_int_str2val(masklen_str, &rc);
            if (rc < 0)
            {
                sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Can't convert %s to integer", masklen_str);
                return PM_E_CFG_INVALID_PARAM;
            }

            if (IPV6_MIN_PREFIXLEN > value || IPV6_MAX_PREFIXLEN < value)
            {
                sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid prefix length %d", value);
                return PM_E_CFG_INVALID_PARAM;
            }
            p_prefix->prefixlen = value;
        }
        else
        {
            sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid IP address %s", addr_str);
            return PM_E_CFG_INVALID_PARAM;
        }
    }
    
    return PM_E_NONE;
}

int32
cdb_prefix_str2val_wo_masklen(char *str, prefix_t *p_prefix)
{
    if (AF_INET == p_prefix->family)
    {
        return cdb_addr_ipv4_str2val(&p_prefix->u.prefix4, str, 0);
    }
    else if (AF_INET6 == p_prefix->family)
    {
        sal_memset(&p_prefix->u.prefix6, 0, sizeof(p_prefix->u.prefix6));
        if (sal_inet_pton(AF_INET6, str, &p_prefix->u.prefix6) == 1)
        {
            return PM_E_NONE;
        }
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

char*
cdb_addr_ipv6_val2str(char *str, uint32 str_len, const addr_ipv6_t *addr)
{
    str[0] = CMD_ZERO_CHAR;
    sal_inet_ntop(AF_INET6, (void*)addr, str, str_len);
    return str;
}

int32
cdb_addr_ipv6_str2val(const char *str, addr_ipv6_t *addr)
{
    /* try IPv6 */
    sal_memset(addr, 0, sizeof(addr_ipv6_t));
    if (sal_inet_pton(AF_INET6, str, addr) == 1) {
        return PM_E_NONE;
    }

    return PM_E_FAIL;
}

/*cdb_prefix_val2str is xxx#xx, prefix_val2str is xxx/x*/
char*
cdb_prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix)
{
    uint32 len = 0;

    if (AF_INET == prefix->family)
    {
        /* IPv4 */
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET, (void*)&(prefix->u.prefix4), str, str_len);
        len = sal_strlen(str);
    }
    else if (AF_INET6 == prefix->family)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET6, (void*)&(prefix->u.prefix6), str, str_len);
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len, CMD_KEY_DIV_STR"%u", prefix->prefixlen);
    return str;
}

char*
cdb_addr_val2str(char *str, uint32 str_len, const addr_t *addr)
{
    str[0] = CMD_ZERO_CHAR;
    if (addr->family == AF_INET) {
        /* IPv4 */
        sal_inet_ntop(AF_INET, (void*)&(addr->u.prefix4), str, str_len);
    } else if(addr->family == AF_INET6) {
        /* IPv6 */
        sal_inet_ntop(AF_INET6, (void*)&(addr->u.prefix6), str, str_len);
    }
    return str;
}

int32
cdb_addr_str2val(addr_t *addr, const char *str, uint32 str_len)
{
    /* try IPv4 */
    sal_memset(addr, 0, sizeof(addr_t));
    if (sal_inet_pton(AF_INET, str, &addr->u.prefix4) == 1) {
        addr->family = AF_INET;
        return PM_E_NONE;
    } else if (sal_inet_pton(AF_INET6, str, &addr->u.prefix6) == 1) {
        addr->family = AF_INET6;
        return PM_E_NONE;
    }
    return PM_E_FAIL;
}

char*
cdb_nexthop_key_u_val2str(char *str, uint32 str_len, const nexthop_key_u *prefix, uint8 family)
{
    //uint32 len = 0;

    if (AF_INET6 == family)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET6, (void*)&(prefix->ipv6), str, str_len);
        //len = sal_strlen(str);
    }
    /*default is ipv4*/
    else
    {
        /* IPv4 */
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET, (void*)&(prefix->ipv4), str, str_len);
        //len = sal_strlen(str);
    }

    return str;
}

int32
cdb_mac_str2val_dhcs(uint8 *mac, const char *str, uint32 str_len)
{
    int32 rc = PM_E_NONE;
    rc = cdb_fdb_mac_address_is_valid(str);
    if (rc < 0)
    {
        return rc;
    }
    if (3 != sal_sscanf(str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

    return PM_E_NONE;
}

int32
cdb_fdb_mac_address_is_valid(const char *mac_addr)
{
    const char *p = mac_addr;
    int32 i = 0;
    int32 j = 0;

    while (*p)
    {
        if ((*p) == '.')
        {
            i++;
            j = 0;
            if (i > 2)
            {
              return PM_E_CFG_INVALID_PARAM;
            }

            p++;
            continue;
        }

        if (((*p) >= '0' && (*p) <= '9') ||
            ((*p) >= 'a' && (*p) <= 'f') ||
            ((*p) >= 'A' && (*p) <= 'F'))
        {
            p++;
            j++;
            if (4 < j)
            {
                return PM_E_CFG_INVALID_PARAM;
            }
            continue;
        }
        else
        {
            return PM_E_CFG_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}

int32
cdb_arp_mac_address_is_valid(const char *mac_addr)
{
    const char *p = mac_addr;
    int32 i = 0;
    int32 j = 0;

    while (*p)
    {
        if ((*p) == '.')
        {
            i++;
            j = 0;
            if (i > 2)
            {
              return PM_E_CFG_INVALID_PARAM;
            }

            p++;
            continue;
        }

        if (((*p) >= '0' && (*p) <= '9') ||
            ((*p) >= 'a' && (*p) <= 'f') ||
            ((*p) >= 'A' && (*p) <= 'F'))
        {
            p++;
            j++;
            if (4 < j)
            {
                return PM_E_CFG_INVALID_PARAM;
            }
            continue;
        }
        else
        {
            return PM_E_CFG_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}

int32
cdb_clock_id_address_is_valid(const char *clockid_addr)
{
    const char *p = clockid_addr;
    int32 i = 0;
    int32 j = 0;

    while (*p)
    {
        if ((*p) == '.')
        {
            i++;
            j = 0;
            if (i > 3)
            {
              return PM_E_CFG_INVALID_PARAM;
            }

            p++;
            continue;
        }

        if (((*p) >= '0' && (*p) <= '9') ||
            ((*p) >= 'a' && (*p) <= 'f') ||
            ((*p) >= 'A' && (*p) <= 'F'))
        {
            p++;
            j++;
            if (4 < j)
            {
                return PM_E_CFG_INVALID_PARAM;
            }
            continue;
        }
        else
        {
            return PM_E_CFG_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}


int32
cdb_fdb_key_str2val(fdb_key_t *fdb_key, const char *str, char *err_desc)
{
    uint8 *mac = fdb_key->mac;
    char *mac_str = NULL;
    char *fid_str = NULL;
    char buf[64];
    int32 rc = PM_E_NONE;

    sal_strncpy(buf, str, 64);
    fid_str = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == fid_str)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    mac_str = buf;
    *fid_str = CMD_ZERO_CHAR;
    fid_str += 1;

    rc = cdb_fdb_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Unable to translate MAC address %s", mac_str);
        return rc;
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid MAC address %s", mac_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

    fdb_key->fid = cdb_int_str2val(fid_str, &rc);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid VLAN %s", fid_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

char*
cdb_fdb_key_val2str(char *str, uint32 str_len, const fdb_key_t *fdb_key)
{
    const uint8 *mac = fdb_key->mac;

    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%u",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        fdb_key->fid);
    return str;
}

int32
cdb_l2edit_key_str2val(const char *str, l2edit_key_t *l2edit_key)
{
    return PM_E_NONE;
}

char*
cdb_l2edit_key_val2str(char *str, uint32 str_len, const l2edit_key_t *l2edit_key)
{
    const uint8 *src_mac = l2edit_key->src_mac;
    const uint8 *dest_mac = l2edit_key->dest_mac;

    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%u",
        sal_ntoh16(((uint16*)src_mac)[0]),
        sal_ntoh16(((uint16*)src_mac)[1]), 
        sal_ntoh16(((uint16*)src_mac)[2]),
        sal_ntoh16(((uint16*)dest_mac)[0]),
        sal_ntoh16(((uint16*)dest_mac)[1]), 
        sal_ntoh16(((uint16*)dest_mac)[2]),
        l2edit_key->fid);
    
    return str;
}

char*
cdb_g8032_fea_ring_val2str(char *str, uint32 str_len, const fea_g8032_ring_key_t *g8032_fea_ring_key)
{
    uint32 len = 0;
    
    str[0] = CMD_ZERO_CHAR;
    len = sal_strlen(str);
    
    sal_sprintf(str + len, CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u", g8032_fea_ring_key->instance_id, g8032_fea_ring_key->ifindex);
    return str;
}

int32
cdb_g8032_fea_ring_str2val(char *str, fea_g8032_ring_key_t *g8032_fea_ring_key)
{
    return PM_E_NONE;
}

char*
cdb_igsp_group_val2str(char *str, uint32 str_len, const igsp_group_key_t *igsp_group_key)
{
    uint32 len = 0;
    
    str[0] = CMD_ZERO_CHAR;
    sal_inet_ntop(AF_INET, (void*)&igsp_group_key->ip, str, str_len);
    len = sal_strlen(str);
    
    sal_sprintf(str + len, CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u", igsp_group_key->vid, igsp_group_key->ifindex);
    return str;
}

int32
cdb_igsp_group_str2val(char *str, igsp_group_key_t *igsp_group_key)
{
    return PM_E_NONE;
}
    
char*
cdb_lldp_msap_id_val2str(char *str, uint32 str_len, const lldp_msap_id_t *msap_id)
{
    char tmp[CMD_BUF_256];
    uint8 *mac = NULL;
    char *name = NULL;

    tmp[0] = CMD_ZERO_CHAR;
    if (LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS == msap_id->chassis_id.type)
    {
        mac = (uint8*)msap_id->chassis_id.id;
        sal_sprintf(str, "chassis_id:%u"CMD_KEY_DIV_STR"%.04hX.%.04hX.%.04hX,",
            msap_id->chassis_id.type,
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]));
    }
    else
    {
        sal_sprintf(str, "chassis_id:%u"CMD_KEY_DIV_STR"todo,",
            msap_id->chassis_id.type);
    }

    if (LLDP_PORT_ID_SUBTYPE_IF_NAME == msap_id->port_id.type)
    {
        name = (char*)msap_id->port_id.id;
        sal_sprintf(tmp, "port_id:%u"CMD_KEY_DIV_STR"%s",
            msap_id->port_id.type, name);
    }
    else
    {
        sal_sprintf(tmp, "port_id:%u"CMD_KEY_DIV_STR"todo",
            msap_id->port_id.type);
    }
    
    sal_strcat(str, tmp);
    
    return str;
}

int32
cdb_lldp_msap_id_str2val(char *str, lldp_msap_id_t *msap_id)
{
    return PM_E_NONE;
}

int32
cdb_ptp_foreign_master_str2val(char *str, const ptp_foreign_master_t *ptp_foreign_master)
{
    return PM_E_NONE;
}

char*
cdb_ptp_foreign_master_val2str(char *str, uint32 str_len, const ptp_foreign_master_t *ptp_foreign_master)
{
    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "foreign_master_port_identity.clock_id %s"CMD_KEY_DIV_STR"foreign_master_port_identity.port_num %u"CMD_KEY_DIV_STR"ifindex %u", 
        ptp_desc_clockid(ptp_foreign_master->foreign_master_port_identity.clock_id), 
        ptp_foreign_master->foreign_master_port_identity.port_num, 
        ptp_foreign_master->ifindex);
    
    return str;
}

int32
cdb_ptp_residence_time_cache_str2val(char *str, const ptp_residence_time_cache_t *ptp_residence_time_cache)
{
    return PM_E_NONE;
}

char*
cdb_ptp_residence_time_cache_val2str(char *str, uint32 str_len, const ptp_residence_time_cache_t *ptp_residence_time_cache)
{
    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%hu"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%hhu", 
        ptp_desc_clockid(ptp_residence_time_cache->source_port_id.clock_id), 
        ptp_residence_time_cache->source_port_id.port_num,
        ptp_residence_time_cache->ifindex_belong_to, 
        ptp_residence_time_cache->is_sync_ingress);
        
    return str;
}

char*
cdb_ace_config_key_val2str(char *str, uint32 str_len, const ace_config_key_t *ace_config_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%d",
        ace_config_key->acl_name, ace_config_key->seq_no);
    return str;
}

int32
cdb_ace_config_key_str2val(char *str, ace_config_key_t *ace_config_key)
{
    return PM_E_NONE;
}

char*
cdb_class_in_policy_config_key_val2str(char *str, uint32 str_len, class_in_policy_config_key_t *class_in_policy_config_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        class_in_policy_config_key->policy_name, class_in_policy_config_key->class_name);
    return str;
}

int32
cdb_class_in_policy_config_key_str2val(char *str, class_in_policy_config_key_t *class_in_policy_config_key)
{

    return PM_E_NONE;
}

char*
cdb_acl_in_class_config_key_val2str(char *str, uint32 str_len, acl_in_class_config_key_t *acl_in_class_config_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        acl_in_class_config_key->class_name, acl_in_class_config_key->acl_name);
    return str;
}

int32
cdb_acl_in_class_config_key_str2val(char *str, acl_in_class_config_key_t *acl_in_class_config_key)
{

    return PM_E_NONE;
}

char*
cdb_class_map_action_config_key_val2str(char *str, uint32 str_len, class_map_action_config_key_t *class_map_action_config_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        class_map_action_config_key->policy_name, class_map_action_config_key->class_name);
    return str;
}

int32
cdb_class_map_action_config_key_str2val(char *str, class_map_action_config_key_t *class_map_action_config_key)
{

    return PM_E_NONE;
}

char*
cdb_tap_group_ingress_key_val2str(char *str, uint32 str_len, tap_group_ingress_key_t *tap_group_ingress_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        tap_group_ingress_key->tap_group, tap_group_ingress_key->if_name);
    return str;
}

int32
cdb_tap_group_ingress_key_str2val(char *str, tap_group_ingress_key_t *tap_group_ingress_key)
{

    return PM_E_NONE;
}

char*
cdb_tap_group_ingress_flow_key_val2str(char *str, uint32 str_len, tap_group_ingress_flow_key_t *tap_group_ingress_flow_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%s",
        tap_group_ingress_flow_key->tap_group, tap_group_ingress_flow_key->if_name, tap_group_ingress_flow_key->flow_name);
    return str;
}

int32
cdb_tap_group_ingress_flow_key_str2val(char *str, tap_group_ingress_flow_key_t *tap_group_ingress_flow_key)
{

    return PM_E_NONE;
}

char*
cdb_tap_group_egress_key_val2str(char *str, uint32 str_len, tap_group_egress_key_t *tap_group_egress_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        tap_group_egress_key->tap_group, tap_group_egress_key->if_name);
    return str;
}

int32
cdb_tap_group_egress_key_str2val(char *str, tap_group_egress_key_t *tap_group_egress_key)
{

    return PM_E_NONE;
}

char*
cdb_tap_five_tuple_key_val2str(char *str, uint32 str_len, tap_five_tuple_key_t *tap_five_tuple_key)
{
    char ipsa[16] =  {0};
    char ipda[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&tap_five_tuple_key->ip_sa, ipsa, sizeof(ipsa));
    sal_inet_ntop(AF_INET, (void *)&tap_five_tuple_key->ip_da, ipda, sizeof(ipda));

    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%d"CMD_KEY_DIV_STR"%d"CMD_KEY_DIV_STR"%d"CMD_KEY_DIV_STR"%d",
        ipsa, ipda, tap_five_tuple_key->l4_src_port, tap_five_tuple_key->l4_dst_port, tap_five_tuple_key->l4_protocol, tap_five_tuple_key->keysel_type);
    return str;
}

int32
cdb_tap_five_tuple_key_str2val(char *str, tap_five_tuple_key_t *tap_five_tuple_key)
{

    return PM_E_NONE;
}

char*
cdb_tap_tuple_acl_entry_id_val2str(char *str, uint32 str_len, tap_tuple_acl_entry_id_t *acl_entry_id)
{
    int i = 0;
    int len = 0;
    for (i = 0; i < GLB_AGG_IFINDEX_MAX; i++)
    {
        sal_sprintf(str + len,"%d"CMD_COMMA_CHAR_STR, acl_entry_id->id[i]);
        len = sal_strlen(str);
    }

    for (i = 0; i < GLB_AGG_IFINDEX_MAX; i++)
    {
        sal_sprintf(str + len,"%d"CMD_COMMA_CHAR_STR, acl_entry_id->ifindex[i]);
        len = sal_strlen(str);
    }
    return str;
}

int32
cdb_tap_tuple_acl_entry_id_str2val(char *str, tap_tuple_acl_entry_id_t *acl_entry_id)
{

    return PM_E_NONE;
}


char*
cdb_msti_port_key_val2str(char *str, uint32 str_len, const msti_port_key_t *msti_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%u", msti_key->name, msti_key->instance);
    return str;
}

int32
cdb_msti_port_key_str2val(char *str,  msti_port_key_t *msti_key)
{ 
    return PM_E_NONE;
}

char*
cdb_ptp_port_id_val2str(char *str, uint32 str_len, const glb_ptp_port_id_t *ptp_port_id)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%u", ptp_desc_clockid(ptp_port_id->clock_id), ptp_port_id->port_num);
    return str;
}

int32
cdb_ptp_port_id_str2val(char *str, glb_ptp_port_id_t *ptp_port_id)
{
    return PM_E_NONE;
}

int32
cdb_auth_server_key_str2val(auth_server_key_t *key, const char *str, char *err_desc)
{
    char *ip_str = NULL;
    char *type_str = NULL;
    char buf[128];
    int32 rc = PM_E_NONE;

    sal_strncpy(buf, str, 128);
    type_str = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == type_str || '\0' == *(type_str + 1)) {
        return PM_E_CFG_INVALID_PARAM;
    }

    ip_str = buf;
    *type_str = CMD_ZERO_CHAR;
    type_str += 1;

    rc = cdb_addr_str2val(&key->addr, ip_str, strlen(ip_str));
    if (rc < 0) {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Unable to translate ip address %s", ip_str);
        return rc;
    }

    if (0 == sal_memcmp(auth_server_type_strs[AUTH_SERVER_TYPE_RADIUS], type_str, sal_strlen("radius")))
    {
        key->type = AUTH_SERVER_TYPE_RADIUS;
        type_str += sal_strlen("radius");
        type_str += 1;

        sal_sscanf(type_str, "%d", &key->sin_port);
    }
    else if (0 == sal_memcmp(auth_server_type_strs[AUTH_SERVER_TYPE_TACPLUS], type_str, sal_strlen("tacplus")))
    {
        key->type = AUTH_SERVER_TYPE_TACPLUS;
        type_str += sal_strlen("tacplus");
        type_str += 1;
        
        sal_sscanf(type_str, "%d", &key->sin_port);
    }
    else
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid authentication server type %s", type_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    /*key->type = cdb_enum_str2val(auth_server_type_strs, AUTH_SERVER_TYPE_MAX, type_str);
    if (key->type < 0) {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid authentication server type %s", type_str);
        return PM_E_CFG_INVALID_PARAM;
    }*/
    
    return PM_E_NONE;
}

char*
cdb_auth_server_key_val2str(char *str, uint32 str_len, const auth_server_key_t *key)
{
    char buf[128];

    sal_snprintf(str, str_len, "%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%d", 
        cdb_addr_val2str(buf, 128, &key->addr), 
        cdb_enum_val2str(auth_server_type_strs, AUTH_SERVER_TYPE_MAX, key->type),
        key->sin_port);
    return str;
}

/* key format: name#index#type */
int32
cdb_auth_session_key_str2val(auth_session_key_t *key, const char *str, char *err_desc)
{
    char index[16];
    char *p;
    char *q;
    u_int32_t nvalue;

    memset(key, 0x00, sizeof(auth_session_key_t));

    p = sal_strstr(str, CMD_KEY_DIV_STR);
    if (NULL == p || '\0' == *(p + 1)) {
        return PM_E_CFG_INVALID_PARAM;
    }
    
    if ((p - str) >= M_AUTH_NAME_SIZE) {
        sprintf(err_desc, 
                "Authentication session key name length range is [1, %d)",
                M_AUTH_NAME_SIZE);
        return PM_E_CFG_INVALID_PARAM;
    }
    memcpy(key->name, str, (p - str));

    p++;
    q = sal_strstr(p, CMD_KEY_DIV_STR);
    if (NULL == q || '\0' == *(q + 1) || (q - p) >= 16) {
        sprintf(err_desc, "Authentication session key index");
        return PM_E_CFG_INVALID_PARAM;
    }
    memcpy(index, p, (q - p));
    index[q - p] = '\0';

    errno = 0;
    nvalue = strtoul(index, NULL, 10);
    if (strspn(index, "0123456789") != strlen(index) 
            || (0 == nvalue && EINVAL == errno)) {
        sprintf(err_desc, "Invalid authentication session key index");
        return PM_E_CFG_INVALID_PARAM;
    }
    key->index = nvalue;
    
    q++;
    key->type = cdb_enum_str2val(auth_server_type_strs, AUTH_SERVER_TYPE_MAX, q);
    if (key->type < 0) {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid authentication session type %s", q);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

char*
cdb_auth_session_key_val2str(char *str, uint32 str_len, const auth_session_key_t *key)
{
    sal_snprintf(str, str_len, "%s"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s",
        key->name, key->index,
        cdb_enum_val2str(auth_server_type_strs, AUTH_SERVER_TYPE_MAX, key->type));
    return str;
}
   
int32
cdb_binding_key_str2val(binding_key_t *key, const char *str, char *err_desc)
{
    uint8 *mac = key->mac;
    char *mac_str = NULL;
    char buf[64];
    char fid[64];
    int32 rc = PM_E_NONE;

    char *temp = NULL;
    char *vid_str = NULL;

    sal_strncpy(buf, str, 64);
    temp = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    mac_str = buf;
    *temp = CMD_ZERO_CHAR;
    temp += 1;

    rc = cdb_fdb_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Unable to translate MAC address %s", mac_str);
        return rc;
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid MAC address %s", mac_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);
    
    vid_str = sal_strstr(temp, CMD_KEY_DIV_STR);
    if (NULL == vid_str)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    
    sal_memcpy(fid, temp, vid_str - temp);
    fid[vid_str - temp] = '\0';
    
    key->ipv6 = cdb_int_str2val(fid, &rc);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid ipv6 flag %s", fid);
        return PM_E_CFG_INVALID_PARAM;
    }

    vid_str++;
    key->vid= cdb_int_str2val(vid_str, &rc);
    if (rc < 0)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid vlan id %s", vid_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    if (sal_strspn(vid_str, "0123456789") != sal_strlen(vid_str)
                || key->vid < 1 || key->vid > 4094) 
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid vlan id %s", vid_str);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

char*
cdb_binding_key_val2str(char *str, uint32 str_len, const binding_key_t *key)
{
    const uint8 *mac = key->mac;

    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        key->ipv6, 
        key->vid);
    return str;
}

int32
cdb_route_node_key_str2val(char *str, route_node_key_t *rn_key)
{
    char buf[MAX_CMD_STR_LEN];
    char addr[MAX_CMD_STR_LEN];
    char prefixlen[MAX_CMD_STR_LEN];
    char nhaddr[MAX_CMD_STR_LEN];
    char ret[MAX_CMD_STR_LEN];
    char *temp = NULL;
    int rc;


    sal_strcpy(buf, str);
    temp = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    *temp = CMD_ZERO_CHAR;
    temp += 1;

    rn_key->vrf_id = cdb_int_str2val(buf, &rc);

    sal_strcpy(addr, temp);
    temp = sal_strstr(addr, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *temp = CMD_ZERO_CHAR;
    temp += 1;

    sal_strcpy(prefixlen, temp);
    temp = sal_strstr(prefixlen, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *temp = CMD_ZERO_CHAR;
    temp += 1;
    
    rc = cdb_prefix_ipv4_str2val(addr,prefixlen,(prefix_ipv4_t *)(&rn_key->p), ret);
    if(rc)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_strcpy(nhaddr, temp);
    
    rc = cdb_addr_ipv4_str2val(&rn_key->nh_addr.ipv4, nhaddr, 0);
    if(rc)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    return PM_E_NONE;
}


char*
cdb_route_node_key_val2str(char *str, uint32 str_len, const route_node_key_t *rn_key)
{
    uint32 len = 0;

    sal_sprintf(str, "%u"CMD_KEY_DIV_STR, rn_key->vrf_id);
    len = sal_strlen(str);
    cdb_prefix_val2str(str+len, str_len - len, &rn_key->p);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR);
    len = sal_strlen(str);
    if (rn_key->p.family == AF_INET)
    {
        sal_sprintf(str+len, "AF_INET"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str + len, str_len - len, &rn_key->nh_addr.ipv4);
    }
    else
    {
        sal_sprintf(str+len, "AF_INET6"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv6_val2str(str + len, str_len - len, &rn_key->nh_addr.ipv6);
    }
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR"ifindex_v6=%u"CMD_KEY_DIV_STR, rn_key->ifindex_v6);

    return str;
}

int32
cdb_route_node_key_with_ad_str2val(char *str, route_node_key_with_ad_t *rn_key)
{
    char buf[MAX_CMD_STR_LEN];
    char addr[MAX_CMD_STR_LEN];
    char prefixlen[MAX_CMD_STR_LEN];
    char nhaddr[MAX_CMD_STR_LEN];
    char ret[MAX_CMD_STR_LEN];
    char *temp = NULL;
    int rc;


    sal_strcpy(buf, str);
    temp = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    *temp = CMD_ZERO_CHAR;
    temp += 1;

    rn_key->vrf_id = cdb_int_str2val(buf, &rc);

    sal_strcpy(addr, temp);
    temp = sal_strstr(addr, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *temp = CMD_ZERO_CHAR;
    temp += 1;

    sal_strcpy(prefixlen, temp);
    temp = sal_strstr(prefixlen, CMD_KEY_DIV_STR);
    if (NULL == temp)
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *temp = CMD_ZERO_CHAR;
    temp += 1;
    
    rc = cdb_prefix_ipv4_str2val(addr,prefixlen,(prefix_ipv4_t *)(&rn_key->p), ret);
    if(rc)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    sal_strcpy(nhaddr, temp);
    
    rc = cdb_addr_ipv4_str2val(&rn_key->nh_addr.ipv4, nhaddr, 0);
    if(rc)
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    return PM_E_NONE;
}

char*
cdb_route_node_key_with_ad_val2str(char *str, uint32 str_len, const route_node_key_with_ad_t *rn_key)
{
    uint32 len = 0;

    sal_sprintf(str, "%u"CMD_KEY_DIV_STR, rn_key->vrf_id);
    len = sal_strlen(str);
    cdb_prefix_val2str(str+len, str_len - len, &rn_key->p);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR);
    len = sal_strlen(str);
    if (rn_key->p.family == AF_INET)
    {
        sal_sprintf(str+len, "AF_INET"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str + len, str_len - len, &rn_key->nh_addr.ipv4);
    }
    else
    {
        sal_sprintf(str+len, "AF_INET6"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv6_val2str(str + len, str_len - len, &rn_key->nh_addr.ipv6);
    }
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR"ifindex_v6=%u"CMD_KEY_DIV_STR, rn_key->ifindex_v6);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR"distance=%u"CMD_KEY_DIV_STR, rn_key->distance);

    return str;
}


char*
cdb_ospf_area_range_key_val2str(char *str, uint32 str_len, const ospf_area_range_key_t *ospf_area_range_key)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR, ospf_area_range_key->area_id);
    len = sal_strlen(str);
    cdb_prefix_val2str(str+len, str_len - len, &ospf_area_range_key->p);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR);

    return str;
}

char*
cdb_ospf6_area_range_key_val2str(char *str, uint32 str_len, const ospf6_area_range_key_t *ospf6_area_range_key)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR, ospf6_area_range_key->area_id);
    len = sal_strlen(str);
    cdb_prefix_val2str(str+len, str_len - len, &ospf6_area_range_key->p);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR);

    return str;
}

int32
cdb_ospf_area_range_key_str2val(char *str, ospf_area_range_key_t *ospf_area_range_key)
{
    return PM_E_NONE;
}

int32
cdb_ospf6_area_range_key_str2val(char *str, ospf6_area_range_key_t *ospf6_area_range_key)
{
    return PM_E_NONE;
}

char*
cdb_ospf_area_vlink_key_val2str(char *str, uint32 str_len, const ospf_area_vlink_key_t *ospf_area_vlink_key)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR, ospf_area_vlink_key->area_id);
    len = sal_strlen(str);
    cdb_addr_ipv4_val2str(str+len, str_len - len, &ospf_area_vlink_key->vl_peer);
    len = sal_strlen(str);
    sal_sprintf(str+len, CMD_KEY_DIV_STR);

    return str;
}

int32
cdb_ospf_area_vlink_key_str2val(char *str, ospf_area_vlink_key_t *ospf_area_vlink_key)
{
    return PM_E_NONE;
}

char*
cdb_rip_timers_val2str(char *str, uint32 str_len, const rip_timers_t *timers)
{
	sal_printf(str,"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u",timers->update,timers->timeout,timers->garbage);
	return str;
}

int32
cdb_rip_timers_str2val(char *str, const rip_timers_t *timers)
{
	return PM_E_NONE;
}

int32
cdb_nexthop_key_str2val(char *str, nexthop_key_t *nh_key)
{
    char *p = NULL;
    char *tmp = NULL;

    nh_key->type = NH_TYPE_MAX;

    if (sal_strstr(str, "ipv4"))
    {
        nh_key->type = NH_TYPE_IPV4;
        p = sal_strstr(str, CMD_KEY_DIV_STR);
        if (NULL == p)
        {
            return PM_E_INVALID_PARAM;
        }
        p++;
        
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        if(NULL == p)
        {
            return PM_E_NONE;
        }
        *tmp = '\0';
        if(cdb_addr_ipv4_str2val(&nh_key->u.ipv4, p, 0))
        {
            return PM_E_FAIL;
        }
        tmp++;
        sal_sprintf(nh_key->ifname, "%s", tmp);
    }
    else if (sal_strstr(str, "local"))
    {
        nh_key->type = NH_TYPE_LOCAL;
        p = sal_strstr(str, CMD_KEY_DIV_STR);
        if (NULL == p)
        {
            return PM_E_INVALID_PARAM;
        }
        p++;
        
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        if(NULL == p)
        {
            return PM_E_NONE;
        }
        *tmp = '\0';
        if(cdb_addr_ipv4_str2val(&nh_key->u.ipv4, p, 0))
        {
            return PM_E_FAIL;
        }
        tmp++;
        sal_sprintf(nh_key->ifname, "%s", tmp);
    }
    #if 0
    else if (sal_strstr(str, "ifindex"))
    {
        nh_key->type = NH_TYPE_IFINDEX;
        p = sal_strstr(str, CMD_KEY_DIV_STR);
        if (NULL == p)
        {
            return PM_E_INVALID_PARAM;
        }
        p++;
        nh_key->u.ifindex = cdb_int_str2val(p, &rc);
        if (rc < 0)
        {
            return rc;
        }
    }
    else
    {
        return PM_E_FAIL;
    }
    #endif    
    return PM_E_NONE;
}



char*
cdb_nexthop_key_val2str(char *str, uint32 str_len, const nexthop_key_t *nh_key)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    switch (nh_key->type)
    {
    case NH_TYPE_IPV4:
        sal_sprintf(str, "NH_TYPE_IPV4"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str+len, str_len - len, &nh_key->u.ipv4);
        len = sal_strlen(str);
        sal_sprintf(str+len, CMD_KEY_DIV_STR);
        sal_sprintf(str + len + 1, "%s", nh_key->ifname);        
        break;

    case NH_TYPE_LOCAL:
        sal_sprintf(str, "NH_TYPE_LOCAL"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str+len, str_len - len, &nh_key->u.ipv4);
        len = sal_strlen(str);
        sal_sprintf(str+len, CMD_KEY_DIV_STR);
        sal_sprintf(str + len + 1, "%s", nh_key->ifname);        
        break;

    case NH_TYPE_IPV6:
        sal_sprintf(str, "NH_TYPE_IPV6"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv6_val2str(str+len, str_len - len, &nh_key->u.ipv6);
        len = sal_strlen(str);
        sal_sprintf(str+len, CMD_KEY_DIV_STR);
        sal_sprintf(str + len + 1, "%s", nh_key->ifname);        
        break;

    case NH_TYPE_IPV6_IFINDEX:
        sal_sprintf(str, "NH_TYPE_IPV6_IFINDEX"CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        cdb_addr_ipv6_val2str(str+len, str_len - len, &nh_key->u.ipv6);
        len = sal_strlen(str);
        sal_sprintf(str+len, CMD_KEY_DIV_STR);
        sal_sprintf(str + len + 1, "%s", nh_key->ifname);        
        break;

    #if 0
    case NH_TYPE_IFINDEX:
        sal_sprintf(str, "ifindex"CMD_KEY_DIV_STR"%u", nh_key->u.ifindex);
        break;
    #endif
    default:
        return str;
    }

    return str;
}

char*
cdb_nexthop_ecmp_val2str(char *str, uint32 str_len, const nexthop_ecmp_t *nh_group_key)
{
    uint32 i   = 0;
    nexthop_key_t nh_key_c;
    char *ecmp_str = NULL;
    char str_buf[MAX_CMD_STR_LEN];

    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "rr_en=%d%snh_num=%d", nh_group_key->rr_en, CMD_KEY_DIV_STR, nh_group_key->nh_num);
    for (i = 0; i < nh_group_key->nh_num; i++)
    {
        sal_memset(&nh_key_c, 0, sizeof(nh_key_c));
        sal_memcpy(&nh_key_c, &nh_group_key->nh[i], sizeof(nh_key_c));
        sal_memset(str_buf, 0 , sizeof(str_buf));
        ecmp_str = cdb_nexthop_key_val2str(str_buf, str_len, &nh_key_c);
        if (NULL == ecmp_str)
        {
            return NULL;
        }
        
        sal_strcat(str, CMD_KEY_DIV_STR);
        sal_strcat(str, ecmp_str);
    }
    
    return str;
}

int32
cdb_nexthop_ecmp_str2val(char *str, nexthop_ecmp_t *nh_group_key)
{
    int32 i;
    int32 rc;
    char* tmp;
    char *p = NULL;

    if(NULL == str)
    {
        return PM_E_INVALID_PARAM;
    }

    tmp = sal_strstr(str, CMD_KEY_DIV_STR);
    if(NULL == tmp)
    {
        return PM_E_FAIL;
    }
    p = tmp + 1; /*point to first nexthop_key*/
    tmp = CMD_ZERO_CHAR;

    nh_group_key->nh_num = cdb_int_str2val(str, &rc);
    if(nh_group_key->nh_num == 0)
    {
        return PM_E_NONE;
    }

    tmp = sal_strstr(str, CMD_KEY_DIV_STR);
    if(NULL == tmp)
    {
        return PM_E_INVALID_PARAM;
    }
    for(i=0; i<nh_group_key->nh_num; i++)
    {
        PM_E_RETURN(cdb_nexthop_key_str2val(p, &nh_group_key->nh[i]));
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
    }
    
    return PM_E_NONE;
}

char*
cdb_nexthop_ecmp_routed_val2str(char *str, uint32 str_len, const nexthop_ecmp_routed_t *nh_group_key)
{
    uint32 i   = 0;
    nexthop_key_t nh_key_c;
    char *ecmp_str = NULL;
    char str_buf[MAX_CMD_STR_LEN];

    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "nh_num=%d", nh_group_key->nh_num);
    for (i = 0; i < nh_group_key->nh_num; i++)
    {
        sal_memset(&nh_key_c, 0, sizeof(nh_key_c));
        sal_memcpy(&nh_key_c, &nh_group_key->nh[i], sizeof(nh_key_c));
        sal_memset(str_buf, 0 , sizeof(str_buf));
        ecmp_str = cdb_nexthop_key_val2str(str_buf, str_len, &nh_key_c);
        if (NULL == ecmp_str)
        {
            return NULL;
        }
        
        sal_strcat(str, CMD_KEY_DIV_STR);
        sal_strcat(str, ecmp_str);
    }
    
    return str;
}

int32
cdb_nexthop_ecmp_routed_str2val(char *str, nexthop_ecmp_routed_t *nh_group_key)
{
    int32 i;
    int32 rc;
    char* tmp;
    char *p = NULL;

    if(NULL == str)
    {
        return PM_E_INVALID_PARAM;
    }

    tmp = sal_strstr(str, CMD_KEY_DIV_STR);
    if(NULL == tmp)
    {
        return PM_E_FAIL;
    }
    p = tmp + 1; /*point to first nexthop_key*/
    tmp = CMD_ZERO_CHAR;

    nh_group_key->nh_num = cdb_int_str2val(str, &rc);
    if(nh_group_key->nh_num == 0)
    {
        return PM_E_NONE;
    }

    tmp = sal_strstr(str, CMD_KEY_DIV_STR);
    if(NULL == tmp)
    {
        return PM_E_INVALID_PARAM;
    }
    for(i=0; i<nh_group_key->nh_num; i++)
    {
        PM_E_RETURN(cdb_nexthop_key_str2val(p, &nh_group_key->nh[i]));
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
        tmp = sal_strstr(p, CMD_KEY_DIV_STR);
        p = tmp + 1;
    }
    
    return PM_E_NONE;
}

int32
cdb_mac_str2val(uint8 *mac, const char *str)
{
    int32 rc = PM_E_NONE;
    rc = cdb_fdb_mac_address_is_valid(str);
    if (rc < 0)
    {
        return rc;
    }
    if (3 != sal_sscanf(str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);

    return PM_E_NONE;
}

char*
cdb_mac_val2str(char *str, uint32 str_len, const uint8 *mac)
{
    sal_sprintf(str, "%.04hX.%.04hX.%.04hX",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]));
    return str;
}

int32
cdb_mstp_digest_str2val(mstp_digest_t *digest, const char *str)
{
    uint32 data[MSTP_DIGEST_LEN];
    uint32 i = 0;

    if (MSTP_DIGEST_LEN != sal_sscanf(str, "0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        &data[0], &data[1], &data[2], &data[3], 
        &data[4], &data[5], &data[6], &data[7], 
        &data[8], &data[9], &data[10], &data[11], 
        &data[12], &data[13], &data[14], &data[15]))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    for (i = 0; i < MSTP_DIGEST_LEN; i++)
    {
        digest->data[i] = data[i];
    }
    return PM_E_NONE;
}

char*
cdb_mstp_digest_val2str(char *str, uint32 str_len, const mstp_digest_t *digest)
{
    sal_sprintf(str, "0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        digest->data[0], digest->data[1], digest->data[2], digest->data[3],
        digest->data[4], digest->data[5], digest->data[6], digest->data[7],
        digest->data[8], digest->data[9], digest->data[10], digest->data[11],
        digest->data[12], digest->data[13], digest->data[14], digest->data[15]);
    return str;
}

int32
cdb_ptp_default_str2val(const char *str, ptp_default_t *p_data)
{
    return PM_E_NONE;
}

char*
cdb_ptp_default_val2str(char *str, uint32 str_len, const ptp_default_t *p_data)//done
{
    sal_sprintf(str, "clock_identity %s"
        //CMD_KEY_DIV_STR"clk_accuracy %hhu"CMD_KEY_DIV_STR"clk_class %hhu"CMD_KEY_DIV_STR"offset_scaled_log_variance %hu"
        CMD_KEY_DIV_STR"clk_accuracy 0x%hhX"CMD_KEY_DIV_STR"clk_class %hhu"CMD_KEY_DIV_STR"offset_scaled_log_variance %hu"
        CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"domain_number %hhu" CMD_KEY_DIV_STR"number_ports %u"
        CMD_KEY_DIV_STR"primary_domain %hhu"CMD_KEY_DIV_STR"priority1 %hhu"CMD_KEY_DIV_STR"priority2 %hhu"
        CMD_KEY_DIV_STR"slave_only %s"CMD_KEY_DIV_STR"two_step_flag %s", 
        ptp_desc_clockid(p_data->clock_identity), 
        p_data->clock_quality.clk_accuracy, p_data->clock_quality.clk_class, p_data->clock_quality.offset_scaled_log_variance, 
        ptp_mechanism_strs[p_data->delay_mechanism], p_data->domain_number, p_data->number_ports, 
        p_data->primary_domain, p_data->priority1, p_data->priority2, 
        ptp_desc_bool(p_data->slave_only), ptp_desc_bool(p_data->two_step_flag));
        
    return str;
}

int32
cdb_ptp_current_str2val(const char *str, ptp_current_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_current_val2str(char *str, uint32 str_len, const ptp_current_t *p_data)//done
{
    sal_sprintf(str, "mean_path_delay %"PRId64""CMD_KEY_DIV_STR"steps_removed %hu"CMD_KEY_DIV_STR"offset_from_master %"PRId64"", 
        p_data->mean_path_delay, p_data->steps_removed, p_data->offset_from_master);
    
    return str;
}

int32
cdb_ptp_parent_str2val(const char *str, ptp_parent_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_parent_val2str(char *str, uint32 str_len, const ptp_parent_t *p_data)//done
{
    sal_sprintf(str, "grandmaster_clock_quality.clk_accuracy 0x%hhx"CMD_KEY_DIV_STR"grandmaster_clock_quality.clk_class %hhu"CMD_KEY_DIV_STR"grandmaster_clock_quality.offset_scaled_log_variance %hu"
        CMD_KEY_DIV_STR"grandmaster_identity %s"CMD_KEY_DIV_STR"grandmaster_priority1 %hhu"CMD_KEY_DIV_STR"grandmaster_priority2 %hhu"
        CMD_KEY_DIV_STR"observed_parent_clock_phase_change_rate %d"CMD_KEY_DIV_STR"observed_parent_offset_scaled_log_variance %hu"CMD_KEY_DIV_STR"parent_port_identity.clock_id %s"CMD_KEY_DIV_STR"parent_port_identity.port_num%hu"
        CMD_KEY_DIV_STR"%s", 
        p_data->grandmaster_clock_quality.clk_accuracy, p_data->grandmaster_clock_quality.clk_class, p_data->grandmaster_clock_quality.offset_scaled_log_variance,
        ptp_desc_clockid(p_data->grandmaster_identity), p_data->grandmaster_priority1, p_data->grandmaster_priority2, 
        p_data->observed_parent_clock_phase_change_rate, p_data->observed_parent_offset_scaled_log_variance, ptp_desc_clockid(p_data->parent_port_identity.clock_id), p_data->parent_port_identity.port_num,
        ptp_state_strs[p_data->parent_stats]);
        
    return str;
}

int32
cdb_ptp_time_properties_str2val(const char *str, ptp_time_properties_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_time_properties_val2str(char *str, uint32 str_len, const ptp_time_properties_t *p_data)/*done*/
{
    sal_sprintf(str, "current_utc_offset %u"CMD_KEY_DIV_STR"current_utc_offset_valid %u"CMD_KEY_DIV_STR"frequency_traceable %u"
        CMD_KEY_DIV_STR"leap59 %u"CMD_KEY_DIV_STR"leap61 %u"CMD_KEY_DIV_STR"ptp_timescale %u"CMD_KEY_DIV_STR"time_source %u"CMD_KEY_DIV_STR"time_traceable %u",
        p_data->current_utc_offset, p_data->current_utc_offset_valid, p_data->frequency_traceable, 
        p_data->leap59, p_data->leap61, p_data->ptp_timescale, p_data->time_source, p_data->time_traceable);
        
    return str;
}

int32
cdb_ptp_global_c_str2val(const char *str, ptp_global_c_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_global_c_val2str(char *str, uint32 str_len, const ptp_global_c_t *p_data)//done
{
    int i = 0;
    int len = 0;

    sal_sprintf(str + len,"path_delay_cache ");
    len = sal_strlen(str);
    
    sal_sprintf(str + len, "last_correction_field_with_mean_path_delay %"PRId64""CMD_KEY_DIV_STR"last_master_to_slave_delay %"PRId64""
        CMD_KEY_DIV_STR"last_sync_rx_ts %s"CMD_KEY_DIV_STR"last_sync_tx_ts %s"
        CMD_KEY_DIV_STR"mean_nns %d"CMD_KEY_DIV_STR"servo_nns_cnt %hhu", 
        (p_data->last_correction_field_with_mean_path_delay), (p_data->last_master_to_slave_delay), 
        ptp_desc_ptp_timestamp(&(p_data->last_sync_rx_ts)), ptp_desc_ptp_timestamp(&(p_data->last_sync_tx_ts)), 
        p_data->mean_nns, p_data->servo_nns_cnt);
        len = sal_strlen(str);

    sal_sprintf(str + len, CMD_KEY_DIV_STR"servo_nns_cache ");
    len = sal_strlen(str);
        
    for (i = 0; i < p_data->servo_nns_cnt; i++)
    {
        sal_sprintf(str + len, "%d"CMD_COMMA_CHAR_STR, p_data->servo_nns_cache[i]);
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len,
        CMD_KEY_DIV_STR"servo_nns_first_2_ignored %s"CMD_KEY_DIV_STR"servo_nns_lock %s"CMD_KEY_DIV_STR"servo_nns_lock_cnt %hhu"
        CMD_KEY_DIV_STR"servo_nns_pos %hhu"CMD_KEY_DIV_STR"servo_nns_ready %s"CMD_KEY_DIV_STR"servo_offset_cnt %hhu", 
        ptp_desc_bool(p_data->servo_nns_first_2_ignored), ptp_desc_bool(p_data->servo_nns_lock), p_data->servo_nns_lock_cnt, 
        p_data->servo_nns_pos, ptp_desc_bool(p_data->servo_nns_ready), p_data->servo_offset_cnt);
        len = sal_strlen(str);
        
    return str;
}

int32
cdb_ptp_port_str2val(const char *str, ptp_port_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_port_val2str(char *str, uint32 str_len, const ptp_port_t *p_data)//done
{
    sal_sprintf(str, "announce_receipt_timeout %hhu"CMD_KEY_DIV_STR"delay_mechanism %s"CMD_KEY_DIV_STR"faulty_flag %hhd"
        CMD_KEY_DIV_STR"log_announce_interval %hhd"CMD_KEY_DIV_STR"log_min_delay_req_interval %hhd"CMD_KEY_DIV_STR"log_min_pdelay_req_interval %hhd"CMD_KEY_DIV_STR"log_sync_interval %hhd"
        CMD_KEY_DIV_STR"peer_mean_path_delay %"PRId64""CMD_KEY_DIV_STR"port_identity_port_num %hu"CMD_KEY_DIV_STR"port_state %s"CMD_KEY_DIV_STR"version_number %hhu", 
        p_data->announce_receipt_timeout, ptp_mechanism_strs[p_data->delay_mechanism], p_data->faulty_flag, 
        p_data->log_announce_interval, p_data->log_min_delay_req_interval, p_data->log_min_pdelay_req_interval, p_data->log_sync_interval, 
        p_data->peer_mean_path_delay, p_data->port_identity_port_num, ptp_state_strs[p_data->port_state], p_data->version_number);
        
    return str;
}

int32
cdb_glb_ptp_port_address_str2val(const char *str, glb_ptp_port_address_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_glb_ptp_port_address_val2str(char *str, uint32 str_len, const glb_ptp_port_address_t *p_data)
{
    int len = 0;
    
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"length %u"CMD_KEY_DIV_STR, 
        ptp_net_proto_strs[p_data->protocol], p_data->length);
    len = sal_strlen(str);
    
    if (PTP_NET_PROTO_IEEE_802_3 == p_data->protocol)
    {
        sal_sprintf(str + len, "mac_addr ");
        len = sal_strlen(str);
        cdb_mac_addr_val2str(str+len, str_len - len, p_data->field.mac_addr);
        len = sal_strlen(str);
    }
    else if (PTP_NET_PROTO_UDP_IPV4 == p_data->protocol)
    {
        sal_sprintf(str + len, "ip_addr ");
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str+len, str_len - len, &(p_data->field.ip_addr));
        len = sal_strlen(str);
    }
    else if (PTP_NET_PROTO_UDP_IPV4_UNICAST == p_data->protocol)
    {
        sal_sprintf(str + len, "ip_addr ");
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str+len, str_len - len, &(p_data->field.ip_addr));
        len = sal_strlen(str);
        sal_sprintf(str + len, CMD_KEY_DIV_STR"dest_ip_addr");
        len = sal_strlen(str);
        cdb_addr_ipv4_val2str(str+len, str_len - len, &(p_data->dest_ip_addr));
        len = sal_strlen(str);
    }
        
    return str;
}

int32
cdb_glb_ptp_timeinterval_str2val(const char *str, glb_ptp_timeinterval_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_glb_ptp_timeinterval_val2str(char *str, uint32 str_len, const glb_ptp_timeinterval_t *p_data)//done
{
    sal_sprintf(str, "v %"PRId64""CMD_KEY_DIV_STR"msb %d"CMD_KEY_DIV_STR"lsb %u"CMD_KEY_DIV_STR">>PTP_SCALED_NS_BITS %"PRId64"",
        p_data->v, p_data->s.msb, p_data->s.lsb, p_data->v >> PTP_SCALED_NS_BITS);
        
    return str;
}

 int32
tpoam_desc_dm_interval_str2val(char* interval_str,  uint32* interval)
{
    if (0 == sal_strcmp(interval_str, "100ms"))
    {
        *interval = TPOAM_DM_INTERVAL_100MS;
    }
    else if (0 == sal_strcmp(interval_str, "1s"))
    {
        *interval = TPOAM_DM_INTERVAL_1S;
    }
    else if (0 == sal_strcmp(interval_str, "10s"))
    {
        *interval = TPOAM_DM_INTERVAL_10S;
    }
    
    return PM_E_NONE;
}

char *
tpoam_desc_dm_interval_val2str(uint32 interval)
{
    switch (interval)
    {
    case TPOAM_DM_INTERVAL_100MS:
        return "100ms";
    case TPOAM_DM_INTERVAL_1S:
        return "1s";
    case TPOAM_DM_INTERVAL_10S:
        return "10s";
    default:
        return "error";
    }
}

char *
tpoam_desc_dm_interval_val2str_without_s(uint32 interval)
{
    switch (interval)
    {
    case TPOAM_DM_INTERVAL_100MS:
        return "100ms";
    case TPOAM_DM_INTERVAL_1S:
        return "1";
    case TPOAM_DM_INTERVAL_10S:
        return "10";
    default:
        return "error";
    }
}

int32
cdb_tpoam_dm_str2val(const char *str, tpoam_dm_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_dm_val2str(char *str, uint32 str_len, const tpoam_dm_t*p_data)
{
    sal_sprintf(str, CMD_KEY_DIV_STR"per_dm_status %hhu"CMD_KEY_DIV_STR"max_count %u"
        CMD_KEY_DIV_STR"exp %hhu"CMD_KEY_DIV_STR"interval %hhu"CMD_KEY_DIV_STR"repeat_cnt %hhu"CMD_KEY_DIV_STR"type %hhu",
        p_data->per_dm_status, p_data->rx1dm.max_count, 
        p_data->para.exp, p_data->para.interval, p_data->para.repeat_cnt, p_data->para.type);
        
    return str;
}

int32
cdb_tpoam_lm_str2val(const char *str, tpoam_lm_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_lm_val2str(char *str, uint32 str_len, const tpoam_lm_t*p_data)
{
    sal_sprintf(str, "local_loss_threshold %u"CMD_KEY_DIV_STR"remote_loss_threshold %u"CMD_KEY_DIV_STR"sd_enable %u",
        p_data->local_loss_threshold, p_data->remote_loss_threshold, p_data->sd_enable);
        
    return str;
}

int32
cdb_tpoam_lb_str2val(const char *str, tpoam_lb_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_lb_val2str(char *str, uint32 str_len, const tpoam_lb_t*p_data)
{
    sal_sprintf(str, "active %hhu"CMD_KEY_DIV_STR"per_lb_status %hhu"CMD_KEY_DIV_STR"payload_len %hu"CMD_KEY_DIV_STR"lbm_next_trans_id %u"
        CMD_KEY_DIV_STR"lbm_tx_count %u"CMD_KEY_DIV_STR"lbr_rx_count %u"CMD_KEY_DIV_STR"lbr_out_order_count %u",
        p_data->active, p_data->per_lb_status, p_data->payload_len, p_data->lbm_next_trans_id,
        p_data->lbm_tx_count, p_data->lbr_rx_count, p_data->lbr_out_order_count);
        
    return str;
}

int32
cdb_tpoam_event_str2val(const char *str, tpoam_event_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_event_val2str(char *str, uint32 str_len, const tpoam_event_t*p_data)
{
    sal_sprintf(str, "max_count %u"CMD_KEY_DIV_STR, p_data->max_count);
        
    return str;
}

int32
cdb_tpoam_status_str2val(const char *str, tpoam_status_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_status_val2str(char *str, uint32 str_len, const tpoam_status_t*p_data)
{
    //sal_sprintf(str, "v %"CMD_KEY_DIV_STR"msb %d"CMD_KEY_DIV_STR"lsb %u"CMD_KEY_DIV_STR">>PTP_SCALED_NS_BITS %"PRId64"",
    //    p_data-, p_data->s.msb, p_data->s.lsb, p_data->v >> PTP_SCALED_NS_BITS);
        
    return str;
}

int32
cdb_tpoam_stats_str2val(const char *str, tpoam_stats_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_stats_val2str(char *str, uint32 str_len, const tpoam_stats_t*p_data)
{
    //sal_sprintf(str, "v %"CMD_KEY_DIV_STR"msb %d"CMD_KEY_DIV_STR"lsb %u"CMD_KEY_DIV_STR"%",
    //    p_data-, p_data->s.msb, p_data->s.lsb, p_data->v >> PTP_SCALED_NS_BITS);
        
    return str;
}

int32
cdb_tpoam_list_str2val(const char *str, tpoam_list_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_list_val2str(char *str, uint32 str_len, const tpoam_list_t*p_data)
{
    //sal_sprintf(str, "v %"CMD_KEY_DIV_STR"msb %d"CMD_KEY_DIV_STR"lsb %u"CMD_KEY_DIV_STR"%",
    //    p_data-, p_data->s.msb, p_data->s.lsb, p_data->v >> PTP_SCALED_NS_BITS);
        
    return str;
}

int32
cdb_tpoam_rc_count_str2val(const char *str, tpoam_rc_count_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_rc_count_val2str(char *str, uint32 str_len, const tpoam_rc_count_t*p_data)
{
    sal_sprintf(str, "csf_cnt %u"CMD_KEY_DIV_STR"lmep_cnt %u"CMD_KEY_DIV_STR"megid_cnt %u"
    CMD_KEY_DIV_STR"mip_cnt %u"CMD_KEY_DIV_STR"rmep_cnt %u"CMD_KEY_DIV_STR"section_cnt %u",
        p_data->csf_cnt, p_data->lmep_cnt, p_data->megid_cnt, 
        p_data->mip_cnt, p_data->rmep_cnt, p_data->section_cnt);
        
    return str;
}

int32
cdb_tpoam_exec_str2val(const char *str, tpoam_exec_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_exec_val2str(char *str, uint32 str_len, const tpoam_exec_t*p_data)
{
    sal_sprintf(str, "exec_type %hhu"CMD_KEY_DIV_STR"session_id %u"CMD_KEY_DIV_STR, p_data->exec_type, p_data->session_id);
    
    return str;
}

int32
cdb_tpoam_trap_cache_str2val(const char *str, tpoam_trap_cache_t*p_data)
{

    return PM_E_NONE;
}

char *
cdb_summer_time_val2str(char *str,uint32 str_length,summer_time_t*p_data)
{
    sal_sprintf(str, "tm_sec %u"CMD_KEY_DIV_STR"tm_min %u"CMD_KEY_DIV_STR"tm_hour %u"
    CMD_KEY_DIV_STR"tm_mday %u"CMD_KEY_DIV_STR"tm_mon %u"CMD_KEY_DIV_STR"tm_year %u"
    CMD_KEY_DIV_STR"tm_wday %u"CMD_KEY_DIV_STR"tm_yday %u"CMD_KEY_DIV_STR"tm_isdst %u",
    p_data->tm_sec, p_data->tm_min, p_data->tm_hour, p_data->tm_mday, p_data->tm_mon, 
    p_data->tm_year, p_data->tm_wday, p_data->tm_yday,p_data->tm_isdst);
    return str;
}

char *
cdb_which_summer_time_val2str(char *str,uint32 str_length, which_summer_time_t*p_data)
{
    sal_sprintf(str, "wday_tm_sec %u"CMD_KEY_DIV_STR"wday_tm_min %u"CMD_KEY_DIV_STR"wday_tm_hour %u"
    CMD_KEY_DIV_STR"wday_tm_which %u"CMD_KEY_DIV_STR"wday_tm_wday %u"CMD_KEY_DIV_STR"wday_tm_mon %u"
    CMD_KEY_DIV_STR"wday_tm_year %u",p_data->wday_tm_sec, p_data->wday_tm_min, p_data->wday_tm_hour, 
    p_data->wday_tm_which, p_data->wday_tm_wday, p_data->wday_tm_mon, p_data->wday_tm_year);
    return str;
}
char *
cdb_summer_time_str2val(const char *str,summer_time_t*p_data)
{
    return PM_E_NONE;
}
char*
cdb_which_summer_time_str2val(const char *str,which_summer_time_t*p_data)
{
    return PM_E_NONE;
}
char*
cdb_tpoam_trap_cache_val2str(char *str, uint32 str_len, const tpoam_trap_cache_t*p_data)
{
    sal_sprintf(str, "max_count %u"CMD_KEY_DIV_STR"peak_count %u"CMD_KEY_DIV_STR"timer_count %u"CMD_KEY_DIV_STR"tx_count %u",
        p_data->max_count, p_data->peak_count, p_data->timer_count, p_data->tx_count);
        
    return str;
}

int32
cdb_tpoam_ratelimit_str2val(const char *str, tpoam_ratelimit_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_ratelimit_val2str(char *str, uint32 str_len, const tpoam_ratelimit_t*p_data)
{
    sal_sprintf(str, "slice %u"CMD_KEY_DIV_STR"token %u"CMD_KEY_DIV_STR, p_data->slice, p_data->token);
        
    return str;
}

int32
cdb_tpoam_lmm_exec_str2val(const char *str, tpoam_lmm_exec_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_lmm_exec_val2str(char *str, uint32 str_len, const tpoam_lmm_exec_t*p_data)
{
    sal_sprintf(str, "exist %u"CMD_KEY_DIV_STR"service_name %s"CMD_KEY_DIV_STR"service_type %u"CMD_KEY_DIV_STR
        "interval %u"CMD_KEY_DIV_STR"repeat_cnt %u"CMD_KEY_DIV_STR, 
        p_data->exist, p_data->service_name, p_data->service_type, 
        p_data->para.interval, p_data->para.repeat_cnt);
        
    return str;
}

int32
cdb_tpoam_lb_exec_str2val(const char *str, tpoam_lb_exec_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_lb_exec_val2str(char *str, uint32 str_len, const tpoam_lb_exec_t*p_data)
{
    sal_sprintf(str,  "exist %u"CMD_KEY_DIV_STR"service_name %s"CMD_KEY_DIV_STR"service_type %u"CMD_KEY_DIV_STR
        "exp %hhu"CMD_KEY_DIV_STR"is_req_mep_id %hhu"CMD_KEY_DIV_STR"lb_sub_type %hhu"CMD_KEY_DIV_STR"node_id %u"CMD_KEY_DIV_STR
        "opt_tlv_len %hu"CMD_KEY_DIV_STR"opt_tlv_type %hhu"CMD_KEY_DIV_STR"repeat_cnt %hhu"CMD_KEY_DIV_STR"rmep_id %hu"CMD_KEY_DIV_STR
        "test_tlv_mode %hu"CMD_KEY_DIV_STR"time_out %hhu"CMD_KEY_DIV_STR"ttl %hhu"CMD_KEY_DIV_STR"ttl_end %hhu"CMD_KEY_DIV_STR, 
        p_data->exist, p_data->service_name, p_data->service_type, 
        p_data->para.exp, p_data->para.is_req_mep_id, p_data->para.lb_sub_type, p_data->para.node_id,
        p_data->para.opt_tlv_len, p_data->para.opt_tlv_type, p_data->para.repeat_cnt, p_data->para.rmep_id,
        p_data->para.test_tlv_mode, p_data->para.time_out, p_data->para.ttl, p_data->para.ttl_end);
        
    return str;
}

int32
cdb_tpoam_1dm_dmm_exec_str2val(const char *str, tpoam_1dm_dmm_exec_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_tpoam_1dm_dmm_exec_val2str(char *str, uint32 str_len, const tpoam_1dm_dmm_exec_t*p_data)
{
    sal_sprintf(str,  "exist %u"CMD_KEY_DIV_STR"service_name %s"CMD_KEY_DIV_STR"service_type %u"CMD_KEY_DIV_STR
        "exp %hhu"CMD_KEY_DIV_STR"interval %hhu"CMD_KEY_DIV_STR"interval %hhu"CMD_KEY_DIV_STR"type %hhu"CMD_KEY_DIV_STR,
        p_data->exist, p_data->service_name, p_data->service_type, 
        p_data->para.exp, p_data->para.interval, p_data->para.repeat_cnt, p_data->para.type);
        
    return str;
}

int32
cdb_upload_tpoam_dm_str2val(const char *str, upload_tpoam_dm_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_upload_tpoam_dm_val2str(char *str, uint32 str_len, const upload_tpoam_dm_t*p_data)
{
    sal_sprintf(str,  "index %hhu"CMD_KEY_DIV_STR"bit_flag %u"CMD_KEY_DIV_STR"delay %u"CMD_KEY_DIV_STR
        "variation %u"CMD_KEY_DIV_STR,
        p_data->index, p_data->bit_flag, p_data->delay, p_data->variation);
        
    return str;
}

int32
cdb_upload_tpoam_lm_str2val(const char *str, upload_tpoam_lm_t*p_data)
{
    
    return PM_E_NONE;
}

char*
cdb_upload_tpoam_lm_val2str(char *str, uint32 str_len, const upload_tpoam_lm_t*p_data)
{
    sal_sprintf(str,  "index %hhu"CMD_KEY_DIV_STR"bit_flag %u"
        CMD_KEY_DIV_STR"lm_near_end_frame_loss %u"CMD_KEY_DIV_STR "lm_near_end_frame_loss_ratio %u"
        CMD_KEY_DIV_STR"lm_far_end_frame_loss %u"CMD_KEY_DIV_STR"lm_far_end_frame_loss_ratio %u"CMD_KEY_DIV_STR,
        p_data->index, p_data->bit_flag, 
        p_data->lm_near_end_frame_loss, p_data->lm_near_end_frame_loss_ratio,
        p_data->lm_far_end_frame_loss, p_data->lm_far_end_frame_loss_ratio);
        
    return str;
}

char*
tpoam_api_signal_status_desc(tpoam_status_t signal_status)
{
    switch (signal_status)
    {
    case TPOAM_STATUS_INIT:
        return "Init";
    case TPOAM_STATUS_FAIL:
        return "Fail";
    case TPOAM_STATUS_DEGRADE:
        return "Degr";    
    case TPOAM_STATUS_OK:
        return "OK";
    default:
        return "Invalid";
    }
}

int32
cdb_upload_tpoam_ccm_str2val(const char *str, upload_tpoam_ccm_t*p_data)
{

    return PM_E_NONE;
}

char*
cdb_upload_tpoam_ccm_val2str(char *str, uint32 str_len, const upload_tpoam_ccm_t*p_data)
{
    sal_sprintf(str,  "ccm_status %s"CMD_KEY_DIV_STR,
        tpoam_api_signal_status_desc(p_data->ccm_status));

    return str;
}

char*
tpoam_api_event_ext_desc(uint32 event)
{
    static char event_str[256] = {0};
    int len = 0;

    sal_sprintf(event_str + len, "TPOAM EVENT: ");
    len = sal_strlen(event_str);

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_NONE))
    {
        //sal_sprintf(event_str + len, "None. ");
        //len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LOC))
    {
        sal_sprintf(event_str + len, "LOC. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_UNEXP_MEP))
    {
        sal_sprintf(event_str + len, "Unexp RMEPID. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_UNEXP_PERIOD))
    {
        sal_sprintf(event_str + len, "Unexp Interval. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_MISMERGE))
    {
        sal_sprintf(event_str + len, "Unexp MEGID. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_MEG_LVL))
    {
        sal_sprintf(event_str + len, "Unexp Level. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_RDI_RX))
    {
        sal_sprintf(event_str + len, "RDI RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_RDI_TX))
    {
        sal_sprintf(event_str + len, "RDI TX. ");
        len = sal_strlen(event_str);
    }
    
#ifdef support_many_event
    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_RX))
    {
        sal_sprintf(event_str + len, "AIS RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_TX))
    {
        sal_sprintf(event_str + len, "AIS TX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_DEFECT))
    {
        sal_sprintf(event_str + len, "AIS Defect. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_CSF_DEFECT))
    {
        sal_sprintf(event_str + len, "CSF Defect. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_RX))
    {
        sal_sprintf(event_str + len, "LCK RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_TX))
    {
        sal_sprintf(event_str + len, "LCK TX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_DEFECT))
    {
        sal_sprintf(event_str + len, "LCK Defect. ");
        len = sal_strlen(event_str);
    }

    
    if (FLAG_ISSET(event, GLB_TPOAM_EVT_EVT_CACHE_FULL))
    {
        sal_sprintf(event_str + len, "Event Full. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_1DM_CACHE_FULL))
    {
        sal_sprintf(event_str + len, "1DM Full. ");
        len = sal_strlen(event_str);
    }
#endif

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_WAIT_1ST_CC))
    {
        sal_sprintf(event_str + len, "Wait 1st CCM. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_CRC_DEFECT))
    {
        sal_sprintf(event_str + len, "CRC RX. ");
        len = sal_strlen(event_str);
    }

    return event_str;
}

int32
cdb_ptp_vlan_info_str2val(const char *str, ptp_vlan_info_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_vlan_info_val2str(char *str, uint32 str_len, const ptp_vlan_info_t *p_data)//done
{
    //sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u", 
    sal_sprintf(str, "vid %hd"CMD_KEY_DIV_STR"cos %hd", 
        p_data->vid, p_data->cos);
        
    return str;
}

int32
cdb_ptp_dump_flag_str2val(const char *str, ptp_dump_flag_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_dump_flag_val2str(char *str, uint32 str_len, const ptp_dump_flag_t *p_data)//done
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%s",
        ptp_desc_bool(p_data->ptp_dump_flag_dec), ptp_desc_bool(p_data->ptp_dump_flag_raw), 
        ptp_desc_bool(p_data->ptp_dump_flag_rx),ptp_desc_bool( p_data->ptp_dump_flag_tx));
        
    return str;
}

char*
cdb_ptp_port_p_val2str(char *str, uint32 str_len, const ptp_port_p_t *p_data)//done
{
    sal_sprintf(str, "delay_m_s %"PRId64""CMD_KEY_DIV_STR"delay_req_receive_time %s"CMD_KEY_DIV_STR"delay_req_send_time %s"
        CMD_KEY_DIV_STR"delay_s_m %"PRId64""CMD_KEY_DIV_STR"last_delay_m_s %"PRId64""CMD_KEY_DIV_STR"last_delay_resp_correction_field %s"
        CMD_KEY_DIV_STR"last_pdelay_resp_correction_field %s"CMD_KEY_DIV_STR"last_sync_correction_field %s"CMD_KEY_DIV_STR"offset_adjusted %"PRId64""
        CMD_KEY_DIV_STR"pdelay_m_s %"PRId64""CMD_KEY_DIV_STR"pdelay_req_receive_time %s"CMD_KEY_DIV_STR"pdelay_req_send_time %s"
        CMD_KEY_DIV_STR"pdelay_resp_receive_time %s"CMD_KEY_DIV_STR"pdelay_resp_send_time %s"CMD_KEY_DIV_STR"pdelay_s_m %"PRId64""
        CMD_KEY_DIV_STR"recv_sync_sequence_id %hu"CMD_KEY_DIV_STR"sent_announce_sequence_id %hu"CMD_KEY_DIV_STR"sent_delay_req_sequence_id %hu"
        CMD_KEY_DIV_STR"sent_pdelay_req_sequence_id %hu"CMD_KEY_DIV_STR"sent_sync_sequence_id %hu"CMD_KEY_DIV_STR"sync_receive_time %s"
        CMD_KEY_DIV_STR"sync_send_time %s"CMD_KEY_DIV_STR"waiting_for_follow %u"CMD_KEY_DIV_STR"waiting_for_pdelay_resp_follow %u",
        p_data->delay_m_s, ptp_desc_ptp_timestamp(&p_data->delay_req_receive_time), ptp_desc_ptp_timestamp(&p_data->delay_req_send_time), 
        p_data->delay_s_m, p_data->last_delay_m_s, ptp_desc_ptp_timeinterval(&p_data->last_delay_resp_correction_field), 
        ptp_desc_ptp_timeinterval(&p_data->last_pdelay_resp_correction_field), ptp_desc_ptp_timeinterval(&p_data->last_sync_correction_field), p_data->offset_adjusted,
        p_data->pdelay_m_s, ptp_desc_ptp_timestamp(&p_data->pdelay_req_receive_time), ptp_desc_ptp_timestamp(&p_data->pdelay_req_send_time), 
        ptp_desc_ptp_timestamp(&p_data->pdelay_resp_receive_time), ptp_desc_ptp_timestamp(&p_data->pdelay_resp_send_time), p_data->pdelay_s_m,
        p_data->recv_sync_sequence_id, p_data->sent_announce_sequence_id, p_data->sent_delay_req_sequence_id,
        p_data->sent_pdelay_req_sequence_id, p_data->sent_sync_sequence_id, ptp_desc_ptp_timestamp(&p_data->sync_receive_time), 
        ptp_desc_ptp_timestamp(&(p_data->sync_send_time)), p_data->waiting_for_follow, p_data->waiting_for_pdelay_resp_follow);
        
    return str;
}

int32
cdb_ptp_port_p_str2val(const char *str, ptp_port_p_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_port_c_val2str(char *str, uint32 str_len, const ptp_port_c_t *p_data)//done
{
    int i = 0;
    int len = 0;

    sal_sprintf(str + len,"path_delay_cache ");
    len = sal_strlen(str);
        
    for (i = 0; i < PTP_SERVO_PATH_DELAY_CACHE_MAX; i++)
    {
        sal_sprintf(str + len,"%d"CMD_COMMA_CHAR_STR, p_data->path_delay_cache[i]);
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len,"servo_pdc_cnt %d"CMD_KEY_DIV_STR"servo_pdc_pos %d"CMD_KEY_DIV_STR"servo_pdc_pos %d"CMD_KEY_DIV_STR, 
        p_data->servo_pdc_cnt, p_data->servo_pdc_pos, p_data->servo_pdc_rty);
        
    return str;
}

int32
cdb_ptp_port_c_str2val(const char *str, ptp_port_c_t *p_data)
{

    return PM_E_NONE;
}

int32
cdb_ptp_port_stats_str2val(const char *str, ptp_port_stats_t *p_data)
{
    return PM_E_NONE;
}

char*
cdb_ptp_port_stats_val2str(char *str, uint32 str_len, ptp_port_stats_t *p_data)//done
{
    int i = 0;
    int len = 0;
    
    for (i = 0; i < PTP_STATS_MAX; i++)
    {
        if (i == 0)
        {
            sal_sprintf(str + len,"TX ");
            len = sal_strlen(str);
        }
        if (i == 1)
        {
            sal_sprintf(str + len,"RX ");
            len = sal_strlen(str);
        }
        if (i == 2)
        {
            sal_sprintf(str + len,"DROP ");
            len = sal_strlen(str);
        }
        sal_sprintf(str + len, "announce=%u"CMD_COMMA_CHAR_STR"sync=%u"CMD_COMMA_CHAR_STR"follow_up=%u"CMD_COMMA_CHAR_STR"delay_req=%u"CMD_COMMA_CHAR_STR"delay_resp=%u"CMD_COMMA_CHAR_STR"pdelay_req=%u"CMD_COMMA_CHAR_STR"pdelay_resp=%u"
            CMD_COMMA_CHAR_STR"pdelay_resp_follow_up=%u"CMD_COMMA_CHAR_STR"unknown=%u"CMD_KEY_DIV_STR,
            p_data[i]->announce, p_data[i]->sync, p_data[i]->follow_up, p_data[i]->delay_req, p_data[i]->delay_resp, p_data[i]->pdelay_req, p_data[i]->pdelay_resp, p_data[i]->pdelay_resp_follow_up, p_data[i]->unknown);

        len = sal_strlen(str);
    }
    
    return str;
}

char*
cdb_ptp_msg_proc_ops_val2str(char *str, uint32 str_len, const ptp_msg_proc_ops_t *p_data)
{
    return str;
}

int32
cdb_ptp_msg_proc_ops_str2val(const char *str, ptp_msg_proc_ops_t *p_data)
{
    return PM_E_NONE;
}

char*
cdb_kernel_if_ipv6_addr_val2str(char *str, uint32 str_len, const kernel_if_ipv6_addr_t *p_data)
{
    return str;
}

int32
cdb_kernel_if_ipv6_addr_str2val(const char *str, kernel_if_ipv6_addr_t *p_data)
{
    return PM_E_NONE;
}

char*
cdb_brg_allowed_vlan_val2str(char *str, uint32 str_len, const brg_allowed_vlan_t *allowed_vlan)
{
    char *p = str;
    
    switch (allowed_vlan->action)
    {
    case DS_BRGIF_ALLOWED_VLAN_ACT_ALL:
        sal_strcpy(p, "all");
        p += 3;
        break;
        
    case DS_BRGIF_ALLOWED_VLAN_ACT_NONE:
        sal_strcpy(p, "none");
        p += 4;
        break;

    default:
        break;
    }
    cdb_bitmap_val2str(p, MAX_CMD_STR_LEN, allowed_vlan->vlan_bmp, GLB_VLAN_BMP_WORD_MAX);
    return str;
}

int32
cdb_brg_allowed_vlan_str2val(char *str, brg_allowed_vlan_t *allowed_vlan)
{

    return PM_E_NONE;
}

char*
_ptp_common_message_header_val2str(char *str, uint32 str_len, const ptp_common_message_header_t *p_data)//done
{
    sal_sprintf(str, "control_field %u"CMD_KEY_DIV_STR"correction_field_msb %u"CMD_KEY_DIV_STR"correction_field_lsb %u"CMD_KEY_DIV_STR
        "domain_number %u"CMD_KEY_DIV_STR"flag_field_0_alternate_master_flag %s"CMD_KEY_DIV_STR"flag_field_0_ptp_profile_specific_1 %u"CMD_KEY_DIV_STR
        "flag_field_0_ptp_profile_specific_2 %u"CMD_KEY_DIV_STR"flag_field_0_two_step_flag %s"CMD_KEY_DIV_STR"flag_field_0_unicast_flag %s"CMD_KEY_DIV_STR
        "flag_field_1_current_utc_offset_valid %s"CMD_KEY_DIV_STR"flag_field_1_frequency_traceable %s"CMD_KEY_DIV_STR"flag_field_1_leap59 %s"CMD_KEY_DIV_STR
        "flag_field_1_leap61 %s"CMD_KEY_DIV_STR"flag_field_1_ptp_timescale %u"CMD_KEY_DIV_STR"flag_field_1_ptp_timescale %s"CMD_KEY_DIV_STR
        "log_message_interval %u"CMD_KEY_DIV_STR"message_length %u"CMD_KEY_DIV_STR"message_type %u"CMD_KEY_DIV_STR
        "sequence_id %u"CMD_KEY_DIV_STR"source_port_identity_clock_id %s"CMD_KEY_DIV_STR"source_port_identity_port_num %u"CMD_KEY_DIV_STR
        "transport_specific %u"CMD_KEY_DIV_STR"version_ptp %u"CMD_KEY_DIV_STR, 
        p_data->control_field, p_data->correction_field_msb, p_data->correction_field_lsb, 
        p_data->domain_number, ptp_desc_bool(p_data->flag_field_0_alternate_master_flag), p_data->flag_field_0_ptp_profile_specific_1,
        p_data->flag_field_0_ptp_profile_specific_2, ptp_desc_bool(p_data->flag_field_0_two_step_flag), ptp_desc_bool(p_data->flag_field_0_unicast_flag),
        ptp_desc_bool(p_data->flag_field_1_current_utc_offset_valid), ptp_desc_bool(p_data->flag_field_1_frequency_traceable), ptp_desc_bool(p_data->flag_field_1_leap59),
        ptp_desc_bool(p_data->flag_field_1_leap61), p_data->flag_field_1_ptp_timescale, ptp_desc_bool(p_data->flag_field_1_time_traceable),
        p_data->log_message_interval, p_data->message_length, p_data->message_type,
        p_data->sequence_id, ptp_desc_clockid(p_data->source_port_identity_clock_id), p_data->source_port_identity_port_num,
        p_data->transport_specific, p_data->version_ptp);

    return str;
}

int32
cdb_ptp_common_message_header_str2val(char *str, const ptp_common_message_header_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_common_message_header_val2str(char *str, uint32 str_len, const ptp_common_message_header_t *p_data)
{
    sal_sprintf(str, "control_field %u"CMD_KEY_DIV_STR"correction_field_msb %u"CMD_KEY_DIV_STR"correction_field_lsb %u"CMD_KEY_DIV_STR
        "domain_number %u"CMD_KEY_DIV_STR"flag_field_0_alternate_master_flag %s"CMD_KEY_DIV_STR"flag_field_0_ptp_profile_specific_1 %u"CMD_KEY_DIV_STR
        "flag_field_0_ptp_profile_specific_2 %u"CMD_KEY_DIV_STR"flag_field_0_two_step_flag %s"CMD_KEY_DIV_STR"flag_field_0_unicast_flag %s"CMD_KEY_DIV_STR
        "flag_field_1_current_utc_offset_valid %s"CMD_KEY_DIV_STR"flag_field_1_frequency_traceable %s"CMD_KEY_DIV_STR"flag_field_1_leap59 %s"CMD_KEY_DIV_STR
        "flag_field_1_leap61 %s"CMD_KEY_DIV_STR"flag_field_1_ptp_timescale %u"CMD_KEY_DIV_STR"flag_field_1_ptp_timescale %s"CMD_KEY_DIV_STR
        "log_message_interval %u"CMD_KEY_DIV_STR"message_length %u"CMD_KEY_DIV_STR"message_type %u"CMD_KEY_DIV_STR
        "sequence_id %u"CMD_KEY_DIV_STR"source_port_identity_clock_id %s"CMD_KEY_DIV_STR"source_port_identity_port_num %u"CMD_KEY_DIV_STR
        "transport_specific %u"CMD_KEY_DIV_STR"version_ptp %u"CMD_KEY_DIV_STR, 
        p_data->control_field, p_data->correction_field_msb, p_data->correction_field_lsb, 
        p_data->domain_number, ptp_desc_bool(p_data->flag_field_0_alternate_master_flag), p_data->flag_field_0_ptp_profile_specific_1,
        p_data->flag_field_0_ptp_profile_specific_2, ptp_desc_bool(p_data->flag_field_0_two_step_flag), ptp_desc_bool(p_data->flag_field_0_unicast_flag),
        ptp_desc_bool(p_data->flag_field_1_current_utc_offset_valid), ptp_desc_bool(p_data->flag_field_1_frequency_traceable), ptp_desc_bool(p_data->flag_field_1_leap59),
        ptp_desc_bool(p_data->flag_field_1_leap61), p_data->flag_field_1_ptp_timescale, ptp_desc_bool(p_data->flag_field_1_time_traceable),
        p_data->log_message_interval, p_data->message_length, p_data->message_type,
        p_data->sequence_id, ptp_desc_clockid(p_data->source_port_identity_clock_id), p_data->source_port_identity_port_num,
        p_data->transport_specific, p_data->version_ptp);

    return str;
}

int32
cdb_ptp_msg_announce_str2val(char *str, const ptp_msg_announce_t *p_data)
{

    return PM_E_NONE;
}

char*
cdb_ptp_msg_announce_val2str(char *str, uint32 str_len, const ptp_msg_announce_t *p_data)
{
    sal_sprintf(str, "current_utc_offset %hd"CMD_KEY_DIV_STR"clk_accuracy 0x%hhx"CMD_KEY_DIV_STR"clk_class %hhu"CMD_KEY_DIV_STR"offset_scaled_log_variance %hu"CMD_KEY_DIV_STR
        "grandmaster_identity %s"CMD_KEY_DIV_STR"grandmaster_priority1 %hhu"CMD_KEY_DIV_STR"grandmaster_priority2 %hhu"CMD_KEY_DIV_STR
        "origin_timestamp %s"CMD_KEY_DIV_STR"steps_removed %hu"CMD_KEY_DIV_STR"time_source %s"CMD_KEY_DIV_STR,
        p_data->current_utc_offset, 
        p_data->grandmaster_clock_quality.clk_accuracy, p_data->grandmaster_clock_quality.clk_class, p_data->grandmaster_clock_quality.offset_scaled_log_variance, 
        ptp_desc_clockid(p_data->grandmaster_identity), p_data->grandmaster_priority1, p_data->grandmaster_priority2, 
        ptp_desc_ptp_timestamp(&p_data->origin_timestamp), p_data->steps_removed, ptp_desc_time_source(p_data->time_source));
        
    return str;
}

char*
cdb_sal_time_val2str(char *str, uint32 str_len, const sal_time_t *time)
{
    sal_systime_t current_tv;
    sal_time_t tp;

    str[0] = CMD_ZERO_CHAR;
    sal_gettime(&current_tv);
    tp = current_tv.tv_sec + *time - ctc_time_boottime_sec(NULL);
#if 0
    char *p = NULL;
    p = sal_ctime(&tp);
    sal_strcpy(str, p);
#else
    sal_snprintf(str, str_len, "%u", (uint32)tp);
#endif
    return str;
}

int32
cdb_sal_time_str2val(char *str, sal_time_t *time)
{

    return PM_E_NONE;
}

char*
cdb_timestamp_val2str(char *str, uint32 str_len, const timestamp_t *timestamp)
{
    sal_systime_t current_tv;
    sal_time_t tp;

    str[0] = CMD_ZERO_CHAR;
    sal_gettime(&current_tv);
    tp = current_tv.tv_sec + timestamp->tv_sec - ctc_time_boottime_sec(NULL);
#if 0
    char *p = NULL;
    p = sal_ctime(&tp);
    sal_strcpy(str, p);
#else
    sal_snprintf(str, str_len, "%u.%06u", (uint32)tp, (uint32)timestamp->tv_usec);
#endif
    return str;
}

int32
cdb_timestamp_str2val(char *str, timestamp_t *timestamp)
{

    return PM_E_NONE;
}

char*
cdb_vlan_member_port_val2str(char *str, uint32 str_len, vlan_memberport_t *bmp)
{
    tbl_interface_t *p_db_if = NULL;
    port_bmp_t new_bmp;
    uint32 ifindex = 0;
    
    /* remove LAG member port */
    sal_memcpy(new_bmp, bmp, sizeof(new_bmp));
    GLB_BMP_ITER_BEGIN(new_bmp, ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_db_if)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            GLB_BMP_UNSET(new_bmp, ifindex);
        }
    }
    GLB_BMP_ITER_END(new_bmp, ifindex);
    
    return cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, new_bmp, sizeof(port_bmp_t)/4);
}

int32
cdb_vlan_member_port_str2val(char *str, vlan_memberport_t *bmp)
{
    return PM_E_NONE;
}

char*
cdb_board_type_val2str(char *str, uint32 str_len, const glb_board_type_t *type)
{
    str[0] = CMD_ZERO_CHAR;
    sal_sprintf(str, "%u#%u", type->series, type->type);
    return str;
}

int32
cdb_board_type_str2val(char *str, glb_board_type_t *type)
{

    return PM_E_NONE;
}

char*
cdb_int8_array_val2str(char *str, uint32 str_len, const int8 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%d,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_int8_array_str2val(char *str, int8 *array)
{
    return PM_E_NONE;
}

char*
cdb_uint8_array_val2str(char *str, uint32 str_len, const uint8 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%u,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_uint8_array_str2val(char *str, uint8 *array)
{
    return PM_E_NONE;
}

char*
cdb_int32_array_val2str(char *str, uint32 str_len, const int32 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%d,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_int32_array_str2val(char *str, int32 *array)
{
    return PM_E_NONE;
}

char*
cdb_uint32_array_val2str(char *str, uint32 str_len, const uint32 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%u,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_uint32_array_str2val(char *str, uint32 *array)
{
    return PM_E_NONE;
}

char*
cdb_uint64_array_val2str(char *str, uint32 str_len, const uint64 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%"PRIu64",", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_uint64_array_str2val(char *str, uint64 *array)
{
    return PM_E_NONE;
}


char*
cdb_double_array_val2str(char *str, uint32 str_len, const double *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%f,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_double_array_str2val(char *str, double *array)
{
    return PM_E_NONE;
}

char*
cdb_string_array_val2str(char *str, uint32 str_len, const char *array, uint32 len, uint32 num)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    const char *p = NULL;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < num; i++)
    {
        p = &(array[i * len]);
        sal_sprintf(buf, "%s,", p);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

int32
cdb_string_array_str2val(char *str, char **array)
{
    return PM_E_NONE;
}

int32
cdb_check_str(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        sal_sprintf(err_desc, "String length should not exceed %u", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }
    
    
    /* check the chars */
    p = (char *)name;
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }
        
        sal_sprintf(err_desc, "Character should be \"0-9A-Za-z.-_\"");
        return CDB_NAME_ERR_INVALID_CHAR;
    }
    
    return CDB_NAME_ERR_NONE;
}


int32
cdb_check_name(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }
    
    /*check the first character*/    
    if(!sal_isalpha(*name))
    {
        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_FIRST_CHAR;
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }

        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_INVALID_CHAR;
    }

    /*check the last character*/
    p--;
    if(!(sal_isdigit(*p) || sal_isalpha(*p)))
    {
        sal_sprintf(err_desc, "The last character should be 0-9 or a-z or A-Z and the max len is %d.", max_len);
        return CDB_NAME_ERR_LAST_CHAR;
    }
    
    return CDB_NAME_ERR_NONE;
}

int32
cdb_check_snmp_name(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        sal_sprintf(err_desc, "The name max len is %d.", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }
    
    /*check the first character*/    
    if(!sal_isalpha(*name))
    {
        sal_sprintf(err_desc, "The name's first character should be a-z or A-Z.");
        return CDB_NAME_ERR_FIRST_CHAR;
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }

        sal_sprintf(err_desc, "The name's first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_ and the max len is %d.", max_len);
        return CDB_NAME_ERR_INVALID_CHAR;
    }

    return CDB_NAME_ERR_NONE;
}


int32
cdb_check_snmp_contact(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_@ and the max len is %d.", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }
    
    /*check the first character*/    
    if(!sal_isalpha(*name))
    {
        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_@ and the max len is %d.", max_len);
        return CDB_NAME_ERR_FIRST_CHAR;
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || ('@' == *p))
        {
            p++;
            continue;
        }

        /*modified by yejl to fix bug41406, 2016-12-15*/
        sal_sprintf(err_desc, "The first character should be a-z or A-Z, character only can be 0-9A-Za-z.-_@ and the max len is %d.", max_len);
        return CDB_NAME_ERR_INVALID_CHAR;
    }
    
    return CDB_NAME_ERR_NONE;
}

char*
cdb_reference_val2str(char *str, uint32 str_len, cdb_reference_t *ref)
{
    tbl_interface_t *p_if = NULL;
    char *p = str;
    uint32 used = 0;
    
    if (NULL == ref->obj)
    {
        return "";
    }

    switch (ref->id)
    {
    case TBL_INTERFACE:
        p_if = ref->obj;
        used = sal_strlen(p_if->key.name) + 1;
        sal_snprintf(p, str_len, "%s,", p_if->key.name);
        p += used;
        break;
    default:
        SAL_ASSERT(0);
    }
    
    return str;
}

char*
cdb_reference_list_val2str(char *str, uint32 str_len, cdb_reference_list_t *ref_list)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slist_t *p_list = NULL;
    tbl_interface_t *p_if = NULL;
    tbl_fdb_t *p_fdb = NULL;
    tbl_msti_port_t *p_msti = NULL;
    tbl_erps_ring_t *p_erps_ring = NULL;
    tbl_dot1x_radius_t *p_dot1x_radius = NULL;
    char buf[MAX_CMD_STR_LEN];
#if 0    
    tbl_ace_t *p_ace = NULL;
    char tmp[64];    
#endif    
    void *p_object = NULL;
    char *p = str;
    uint32 used = 0;
    uint32 loop = 0;
    uint32 total_used = 0;
    
    p_list = ref_list->obj_list;

    if (CTCLIB_SLIST_ISEMPTY(p_list))
    {
        return "";
    }

    if (CDB_NODE_TBL == ref_list->type)
    {
        CTCLIB_SLIST_LOOP(p_list, p_object, listnode)
        {
            switch (ref_list->id)
            {
            case TBL_INTERFACE:
                p_if = p_object;
                used = sal_strlen(p_if->key.name) + 1;
                sal_snprintf(p, str_len, "%s,", p_if->key.name);
                p += used;
                break;
            case TBL_FDB:
                p_fdb = p_object;
                buf[0] = CMD_ZERO_CHAR;
                cdb_fdb_key_val2str(buf, MAC_ADDR_LEN, &p_fdb->key);
                used = sal_strlen(buf) + 1;
                sal_snprintf(p, str_len, "%s,", buf);
                p += used;
                break;
            case TBL_MSTI_PORT:
                p_msti = p_object;
                buf[0] = CMD_ZERO_CHAR;
                cdb_msti_port_key_val2str(buf, MAX_CMD_STR_LEN, &p_msti->key);
                used = sal_strlen(buf) + 1;
                sal_snprintf(p, str_len, "%s,", buf);
                p += used;
                break;
            case TBL_ERPS_RING:
                p_erps_ring = p_object;
                buf[0] = CMD_ZERO_CHAR;
                cdb_erps_ring_key_val2str(buf, MAX_CMD_STR_LEN, &p_erps_ring->key);
                used = sal_strlen(buf) + 1;
                sal_snprintf(p, str_len, "%s,", buf);
                p += used;
                break;
            case TBL_DOT1X_RADIUS:
                p_dot1x_radius = p_object;
                buf[0] = CMD_ZERO_CHAR;
                cdb_dot1x_radius_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_radius->key);
                used = sal_strlen(buf) + 1;
                sal_snprintf(p, str_len, "%s,", buf);
                p += used;
                break;
#if 0                
            case TBL_ACE:
                p_ace = p_object;
                tmp[0] = CMD_ZERO_CHAR;
                sal_snprintf(tmp, 64, "%s"CMD_KEY_DIV_STR"%u", p_ace->key.acl_name, p_ace->key.seq_num);
                used = sal_strlen(tmp) + 1;
                sal_snprintf(p, str_len, "%s,", tmp);
                p += used;
                break;
#endif                
            default:
                SAL_ASSERT(0);
            }
            loop++;
            total_used += used;
            if (total_used > str_len)
            {
                SAL_ASSERT(0);
            }
        }
    }

    if (CDB_NODE_DS == ref_list->type)
    {
        CTCLIB_SLIST_LOOP(p_list, p_object, listnode)
        {
            switch (ref_list->id)
            {

            default:
                SAL_ASSERT(0);
            }
        }
    }
            
    
    return str;
}


char *
cdb_str_tolower (char *str)
{
  int i;
  char *ptr = str;
  int len = sal_strlen (str);

  for (i = 0; i < len; i++, ptr++)
    if (sal_char_isalpha ((int) *ptr))
      *ptr = (char) sal_char_tolower ((int) *ptr);

  return str;
}

int
cdb_month_str2digit (char *month_str)
{
  int len = sal_strlen (month_str);
  char *str = cdb_str_tolower (month_str);

  /* More equal three charactors required. */
  if (len < 3)
    return -1;

  if (sal_strncmp ("january", str, len) == 0)
    return JANUARY;
  else if (sal_strncmp ("february", str, len) == 0)
    return FEBRUARY;
  else if (sal_strncmp ("march", str, len) == 0)
    return MARCH;
  else if (sal_strncmp ("april", str, len) == 0)
    return APRIL;
  else if (sal_strncmp ("may", str, len) == 0)
    return MAY;
  else if (sal_strncmp ("june", str, len) == 0)
    return JUNE;
  else if (sal_strncmp ("july", str, len) == 0)
    return JULY;
  else if (sal_strncmp ("august", str, len) == 0)
    return AUGUST;
  else if (sal_strncmp ("september", str, len) == 0)
    return SEPTEMBER;
  else if (sal_strncmp ("october", str, len) == 0)
    return OCTOBER;
  else if (sal_strncmp ("november", str, len) == 0)
    return NOVEMBER;
  else if (sal_strncmp ("december", str, len) == 0)
    return DECEMBER;
  else
    return -1;
}

int32
cdb_get_weekday(char *str, weekday_t *day)
{
     int len = sal_strlen (str);
     
  /* More equal three charactors required. */
  if (len < 3)
    return -1;
  
   if (sal_strncasecmp ("monday", str, len) == 0)
    {
       *day = MONDAY;
    }
  else if (sal_strncasecmp ("tuesday", str, len) == 0)
    {
       *day = TUESDAY;
    }
  else if (sal_strncasecmp ("wednesday", str, len) == 0)
    {
       *day = WEDNESDAY;
    }
  else if (sal_strncasecmp ("thursday", str, len) == 0)
    {
       *day = THURSDAY;
    }
  else if (sal_strncasecmp ("friday", str, len) == 0)
    {
       *day = FRIDAY;
    }
  else if (sal_strncasecmp ("saturday", str, len) == 0)
    {
       *day = SATURDAY;
    }
  else if (sal_strncasecmp ("sunday", str, len) == 0)
    {
       *day = SUNDAY;
    }
  else
    {
      return -1;
    }

  return 0;
}

/* Convert HH:MM:SS MON DAY YEAR to sal_time_t value.  -1 is returned when
   given string is malformed. 
   -2 is returned when given month strng is malformed.
   */
sal_time_t 
cdb_time_range_str2time (char *time_str, char *day_str,
            char *month_str, char *year_str)
{
  char *colon;
  struct sal_tm tm;
  sal_time_t time;
  int32 sec, min, hour;
  int32 day, month, year;
  char *endptr = NULL;

  /* Check hour field of time_str. */
  colon = sal_strchr (time_str, ':');
  if (colon == NULL)
    return -1;
  *colon = '\0';

  /* Hour must be between 0 and 23. */
  hour = sal_strtou32(time_str, &endptr, 10);
  if (hour == UINT32_MAX || *endptr != '\0' || hour < 0 || hour > 23)
    return -1;

  /* Check min field of time_str. */
  time_str = colon + 1;
  colon = sal_strchr (time_str, ':');
  if (*time_str == '\0' || colon == NULL)
    return -1;
  *colon = '\0';

  /* Min must be between 0 and 59. */
  min = sal_strtou32(time_str, &endptr, 10);
  if (min == UINT32_MAX || *endptr != '\0' || min < 0 || min > 59)
    return -1;

  /* Check sec field of time_str. */
  time_str = colon + 1;
  if (*time_str == '\0')
    return -1;

  /* Sec must be between 0 and 59. */
  sec = sal_strtou32 (time_str, &endptr, 10);
  if (sec == UINT32_MAX || *endptr != '\0' || sec < 0 || sec > 59)
    return -1;

  /* Check day_str.  Day must be <1-31>. */
  day = sal_strtou32 (day_str, &endptr, 10);
  if (day == UINT32_MAX || *endptr != '\0' || day < 1 || day > 31)
    return -1;

  /*modified by yejl for bug 49932, 2018-11-19*/
  /* Check month_str.  Day must be <1-12>. */
  month = sal_strtou32 (month_str, &endptr, 10);
  if (month == UINT32_MAX || *endptr != '\0' || month < 1 || month > 12)
    return -2;

  /* Check month_str
  if ((month = cdb_month_str2digit (month_str)) < 0)
    return -2;
  */
  /*end*/

  /* Check year_str.  Year must be <1993-2035>. */
  year = sal_strtou32 (year_str, &endptr, 10);
  if (year == UINT32_MAX || *endptr != '\0' || year < 1993 || year > 2037)
    return -1;


  /*check day*/
  switch(month)
    {
      case 4:
      case 6:
      case 9:
      case 11:
        if(day == 31)
            return -3;
        break;
      case 2:        
        if(IS_LEAP_YEAR(year))
          {
            if(day > 29)
              {
                return -3;
              }              
          }            
        else
          {
            if(day > 28)
              {
                return -3;
              }
           }
    default:
        break;
  }

  /**/

  sal_memset(&tm, 0, sizeof (struct sal_tm));
  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mon = month -1; /*modified by yejl for bug 49932, 2018-11-19*/
  tm.tm_mday = day;
  tm.tm_year = year - 1900;
  tm.tm_isdst = -1;

  time = sal_time_mk (&tm);

  return time;
}

int32
cdb_time_range_get_time(char *str, uint8 *hour, uint8 *min)
{
    char *colon;
    char *endptr = NULL;
    uint32 ul_hour = 0;
    uint32 ul_min = 0;
    
    /* Check hour field of time_str. */
    colon = sal_strchr (str, ':');
    if (colon == NULL)
    return -1;
    *colon = '\0';

    /* Hour must be between 0 and 23. */
    ul_hour = sal_strtou32(str, &endptr, 10);
    if (ul_hour == UINT8_MAX || *endptr != '\0' || ul_hour < 0 || ul_hour > 23)
    return -1;

    /* Check min field of time_str. */
    str = colon + 1;
    if (*str == '\0')
    return -1;


    /* Min must be between 0 and 59. */
    ul_min = sal_strtou32(str, &endptr, 10);
    if (ul_min == UINT8_MAX || *endptr != '\0' || ul_min < 0 || ul_min > 59)
    return -1;

    *hour = ul_hour;
    *min = ul_min;
    return 0;
}

int32
sal_time_loc (sal_time_t * tp, struct sal_tm *loc)
{
  struct sal_tm *tmp;
  /*
  ** This isn't thread safe, but Linux doesn't matter since the
  ** threads are run by our own scheduler and each daemon uses
  ** different memory space.  RTOS can't assume so.
  */
  tmp = localtime (tp);
  if (tmp)
    {
      sal_memcpy (loc, tmp, sizeof (struct sal_tm));
    }
  return (tmp ? 0 : -1);
};

int
cdb_strftime (char *buf, int32 bufsiz, sal_time_t *time)
{
  struct sal_tm tm;
  size_t len;

  sal_time_loc (time,&tm);

  len = sal_time_strf (buf, bufsiz, "%T %m %d %Y", &tm); /*modified by yejl for bug 49932, 2018-11-19*/

  return len;
}

char*
cdb_time_range_timer_val2str(char *str, uint32 str_len, time_range_timer_t* value)
{
    return str;
}

int32
cdb_time_range_timer_str2val(char *str, time_range_timer_t* value)
{
    return PM_E_NONE;
}

char*
cdb_time_range_value_val2str(char *str, uint32 str_len, time_range_value_t *value)
{
    char* range_type_str[] = {"none", "absolute", "periodic"};
    char* weekday_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "daily", "weekdays", "weekend"};
    char buf[MAX_CMD_STR_LEN];
    uint32 len = 0;

    sal_memset(buf, 0, MAX_CMD_STR_LEN);
    sal_memset(str, 0, str_len);
    if (GLB_TIME_RANGE_TYPE_ABSOLUTE == value->type)
    {
        sal_sprintf(str + len, "  %s ", range_type_str[value->type]);
        len = sal_strlen(str);
    
        if (value->ab_time.start)
        {
            cdb_strftime(buf, MAX_CMD_STR_LEN, &value->ab_time.start);
            sal_sprintf(str + len, "start %s ", buf);
            len = sal_strlen(str);
            sal_memset(buf, 0, MAX_CMD_STR_LEN);
        }
        if (value->ab_time.end)
        {
            cdb_strftime(buf, MAX_CMD_STR_LEN, &value->ab_time.end);
            sal_sprintf(str + len, "end %s ", buf);
            len = sal_strlen(str);
            sal_memset(buf, 0, MAX_CMD_STR_LEN);
        }        
    }
    else if (GLB_TIME_RANGE_TYPE_PERIODIC == value->type)
    {
        sal_sprintf(str + len, "  %s ", range_type_str[value->type]);
        len = sal_strlen(str);
    
      sal_sprintf(str + len, "%02d:", value->per_time.start.hour);
      len = sal_strlen(str);
      sal_sprintf(str + len, "%02d ", value->per_time.start.min); 
      len = sal_strlen(str);
      sal_sprintf(str + len, "%s ", weekday_str[value->per_time.start.day]);
      len = sal_strlen(str);
      sal_sprintf(str + len, "to ");      
      len = sal_strlen(str);
      sal_sprintf(str + len, "%02d:", value->per_time.end.hour);      
      len = sal_strlen(str);
      sal_sprintf(str + len, "%02d ", value->per_time.end.min);
      len = sal_strlen(str);
      if(value->per_time.end.day < DAILY)
      {
          sal_sprintf(str + len, "%s ", weekday_str[value->per_time.end.day]);
      }        
   }
    return str;
}

int32
cdb_time_range_value_str2val(char *str, time_range_value_t *value)
{
    return PM_E_NONE;
}


char*
cdb_glb_stats_val2str(char *str, uint32 str_len, const glb_stats_t* value)
{
    return "";
}

char*
cdb_glb_if_stats_val2str(char *str, uint32 str_len, const glb_if_stats_t* value)
{
    char load_intval_str[64] = "  5 minute";
    char byte_str[UINT64_STR_LEN];
    char pkt_str[UINT64_STR_LEN];
    uint32 len = 0;

    if (!value)
    {
        return "";
    }
    sal_memset(str, 0 , str_len);
    
    cds_uint64_to_str(value->octets_rcv_rate, byte_str);
    cds_uint64_to_str(value->pkts_rcv_rate, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "  %s input rate %s bits/sec, %s packets/sec\n",load_intval_str, byte_str, pkt_str);
    
    cds_uint64_to_str(value->octets_send_rate, byte_str);
    cds_uint64_to_str(value->pkts_send_rate, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "  %s output rate %s bits/sec, %s packets/sec\n",load_intval_str, byte_str, pkt_str);
    
    cds_uint64_to_str(value->octets_rcv, byte_str);
    cds_uint64_to_str(value->pkts_rcv, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    %s packets input, %s bytes\n", pkt_str, byte_str);

    cds_uint64_to_str(value->uc_pkts_rcv, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    Received %s unicast,", pkt_str);

    cds_uint64_to_str(value->brdc_pkts_rcv, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s broadcast,", pkt_str);

    /*Modified by yanxa, for bug 17571, 2012-01-14*/
//    value->mc_pkts_rcv += value->oam_pkts_rcv;
    cds_uint64_to_str(value->mc_pkts_rcv + value->oam_pkts_rcv, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s multicast\n",pkt_str);

    cds_uint64_to_str(value->undersize_pkts, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    %s runts,",pkt_str);

    cds_uint64_to_str(value->oversize_pkts, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s giants,",pkt_str);

    cds_uint64_to_str(value->mac_rcv_error, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s input errors,", pkt_str);

    cds_uint64_to_str(value->bad_crc, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s CRC\n",pkt_str);

    cds_uint64_to_str(value->frame_error, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    %s frame,", pkt_str);

    cds_uint64_to_str(value->drop_events, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s overrun,", pkt_str);

    cds_uint64_to_str(value->pause_rcv, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s pause input\n", pkt_str);

    cds_uint64_to_str(value->octets_send, byte_str);
    len = sal_strlen(str);
    cds_uint64_to_str(value->pkts_send, pkt_str);
    sal_sprintf(str + len,  "    %s packets output, %s bytes\n", pkt_str, byte_str);

    cds_uint64_to_str(value->uc_pkts_send, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    Transmitted %s unicast,", pkt_str);

    cds_uint64_to_str(value->brdc_pkts_send, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s broadcast,", pkt_str);

    /*Modified by yanxa, for bug 17571, 2012-01-14*/
    //value->mc_pkts_send += value->oam_pkts_send;
    cds_uint64_to_str(value->mc_pkts_send + value->oam_pkts_send, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s multicast\n",pkt_str);

    cds_uint64_to_str(value->underruns, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  "    %s underruns,", pkt_str);

    cds_uint64_to_str(value->mac_transmit_err, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s output errors,", pkt_str);

    cds_uint64_to_str(value->pause_send, pkt_str);
    len = sal_strlen(str);
    sal_sprintf(str + len,  " %s pause output\n", pkt_str);
    
    return str;
}

int32
cdb_glb_if_stats_str2val(char *str, glb_if_stats_t* value)
{
    return PM_E_NONE;
}

char*
cdb_ds_connected_key_val2str(char *str, uint32 str_len, const void *ifc_key)
{
    return "";
}

int32
cdb_ds_connected_key_str2val(char *str, void *ifc_key)
{
    return PM_E_NONE;
}

char*
cdb_ds_connected_v6_key_val2str(char *str, uint32 str_len, const void *ifc_key)
{
    return "";
}

int32
cdb_ds_connected_v6_key_str2val(char *str, void *ifc_key)
{
    return PM_E_NONE;
}

char*
cdb_ds_static_route_dup_ipv4_nh_key_val2str(char *str, uint32 str_len, const void *ifc_key)
{
    return "";
}

int32
cdb_ds_static_route_dup_ipv4_nh_key_str2val(char *str, void *ifc_key)
{
    return PM_E_NONE;
}

int
cds_uint64_to_str(uint64_t src, char dest[UINT64_STR_LEN])
{
    uint64_t i = 20, j = 0;;
    uint64_t value, sum;

    sal_memset(dest, 0, UINT64_STR_LEN);
    if(0 == src)
    {
        dest[0] = 48;
        return 0;
    }
    
    sum = src;
    while(sum)
    {
       value =  sum%10;
        if(i > 0)
            dest[i--] = value + 48;

       sum = sum/10;
    }

    /*move the string to the front*/
    for(j = 0; j < (UINT64_STR_LEN - 1 - i); j++)
        dest[j] = dest[i+j+1];

    for(; j <= UINT64_STR_LEN -1; j++)
        dest[j] = 0;
    
    return 0;
}

char*
cdb_l4_port_val2str(char *str, uint32 str_len, const glb_flow_l4_port_t *l4_port)
{
    sal_memset(str, 0, str_len);
    if(l4_port->operator == GLB_FLOW_L4PORT_OPERATOR_EQ)
    {
        sal_sprintf(str, "eq %d", l4_port->port);
    }
    else if(l4_port->operator == GLB_FLOW_L4PORT_OPERATOR_GT)
    {
        sal_sprintf(str, "gt %d", l4_port->port);
    }
    else if(l4_port->operator == GLB_FLOW_L4PORT_OPERATOR_LT)
    {
        sal_sprintf(str, "lt %d", l4_port->port);
    }
    else if(l4_port->operator == GLB_FLOW_L4PORT_OPERATOR_NEQ)
    {
        sal_sprintf(str, "neq %d", l4_port->port);
    }
    else if(l4_port->operator == GLB_FLOW_L4PORT_OPERATOR_RANGE)
    {
        sal_sprintf(str, "range %d %d", l4_port->port, l4_port->port2);
    }
    return str;
}

int32
cdb_l4_port_str2val(char *str, glb_flow_l4_port_t *l4_port)
{
    return PM_E_NONE;
}

char*
cdb_tcp_flag_val2str(char *str, uint32 str_len, const glb_flow_tcp_flag_t *flag)
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);

    if(flag->operator == GLB_FLOW_TCP_FLAG_OP_ANY && 
        GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_ACK) && 
        GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_RST) && 
        !GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_URG) && 
        !GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_PSH) && 
        !GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_SYN) && 
        !GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_FIN))
    {
        sal_sprintf(str, "established");
        return str;
    }
    if(flag->operator == GLB_FLOW_TCP_FLAG_OP_ANY)
    {
        sal_sprintf(str, "match-any");
    }
    else if(flag->operator == GLB_FLOW_TCP_FLAG_OP_ALL)
    {
        sal_sprintf(str, "match-all");
    }
    len = sal_strlen(str);
    
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_URG))
    {
        sal_sprintf(str+len, " urg");
        len = sal_strlen(str);
    }
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_ACK))
    {
        sal_sprintf(str+len, " ack");
        len = sal_strlen(str);
    }
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_PSH))
    {
        sal_sprintf(str+len, " psh");
        len = sal_strlen(str);
    }
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_RST))
    {
        sal_sprintf(str+len, " rst");
        len = sal_strlen(str);
    }
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_SYN))
    {
        sal_sprintf(str+len, " syn");
        len = sal_strlen(str);
    }
    if(GLB_FLAG_ISSET(flag->tcp_flags, GLB_FLOW_TCP_FLAG_FIN))
    {
        sal_sprintf(str+len, " fin");
        len = sal_strlen(str);
    }
    return str;
}

int32
cdb_tcp_flag_str2val(char *str, glb_flow_tcp_flag_t *flag)
{
    return PM_E_NONE;
}

char*
cdb_aclqos_if_queue_val2str(char *str, uint32 str_len, const aclqos_if_queue_t *queue)
{
    return str;
}

int32
cdb_aclqos_if_queue_str2val(char *str, aclqos_if_queue_t *queue)
{
    return PM_E_NONE;
}

char*
cdb_snmp_trap_val2str(char *str, uint32 str_len, const snmp_trap_key_t *snmp_trap)
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);
    char szIpBuf[47] = {0};
    char *ip_str = NULL;
    
    if(snmp_trap->is_mgmt_if)
    {
        sal_sprintf(str+len, "%d,",snmp_trap->is_mgmt_if);
        len = sal_strlen(str);
    }

    ip_str = cdb_addr_val2str(szIpBuf, 47, &snmp_trap->ip_addr);
    sal_sprintf(str+len, "%s,", ip_str);
    len = sal_strlen(str);
 
    if(sal_strcmp(snmp_trap->community,""))
    {
        sal_sprintf(str+len, "%s,",snmp_trap->community);
        len = sal_strlen(str);
    }

    sal_sprintf(str+len, "%d,",snmp_trap->udpport);
    len = sal_strlen(str);
       
    return str;
}

int32
cdb_snmp_trap_str2val(char *str, snmp_trap_key_t *snmp_trap)
{
    return PM_E_NONE;
}


char*
cdb_snmp_view_val2str(char *str, uint32 str_len, const snmp_view_key_t *snmp_view)
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);
    //char szIpBuf[47] = {0};
    //char *ip_str = NULL;
    
    if(sal_strcmp(snmp_view->name,""))
    {
        sal_sprintf(str+len, "%s,",snmp_view->name);
        len = sal_strlen(str);
    }

    //sal_sprintf(str+len, "%d,",snmp_view->type);
    //len = sal_strlen(str);
    
    if(sal_strcmp(snmp_view->subtree,""))
    {
        sal_sprintf(str+len, "%s,",snmp_view->subtree);
        len = sal_strlen(str);
    }

    len = sal_strlen(str);
       
    return str;
}

int32
cdb_snmp_view_str2val(char *str, snmp_view_key_t *snmp_view)
{
    return PM_E_NONE;
}


char*
cdb_snmp_group_usm_val2str(char *str, uint32 str_len, const snmp_group_usm_key_t *snmp_usm_group)
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);
    
    if(sal_strcmp(snmp_usm_group->grp_name,""))
    {
        sal_sprintf(str+len, "%s,",snmp_usm_group->grp_name);
        len = sal_strlen(str);
    }
    
    if(sal_strcmp(snmp_usm_group->security_name,""))
    {
        sal_sprintf(str+len, "%s,",snmp_usm_group->security_name);
        len = sal_strlen(str);
    }

    len = sal_strlen(str);
       
    return str;
}

int32
cdb_snmp_group_usm_str2val(char *str, snmp_group_usm_key_t *snmp_usm_group)
{
    return PM_E_NONE;
}


char*
cdb_snmp_info_val2str(char *str, uint32 str_len, const snmp_info_t *item)
{
    if (sal_b64_ntop((unsigned char *)item->info, strlen(item->info), 
            str, str_len) == -1) {
        str[0] = '\0';
        return str;
    }

    return str;

}

int32
cdb_snmp_info_str2val(snmp_info_t *item, const char *str, char *err_desc)
{
    char *p = NULL;
    
    memset(item, 0x00, sizeof(snmp_info_t));
    if (sal_b64_pton((const char *)str, (unsigned char *)item->info, SNMP_INFO_SIZE) == -1) {
        snprintf(err_desc, MAX_CMD_RET_LEN, 
                "Invalid string: The character should be \"0-9A-Za-z.-_@*\" The max len is %d.",SNMP_INFO_SIZE-1); 
        return PM_E_CFG_INVALID_PARAM;
    }

    /* added by kcao for bug 40167, check charactor */
    p = (char *)item->info;
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isalpha(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || ('@' == *p)
            || (' ' == *p)
            || ('*' == *p))
        {
            p++;
            continue;
        }
        
        snprintf(err_desc, MAX_CMD_RET_LEN, 
            "Invalid string: The character should be \"0-9A-Za-z.-_@*\". The max len is %d.",
            SNMP_INFO_SIZE-1);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

char*
cdb_user_val2str(char *str, uint32 str_len, const user_t *item)
{
    if (sal_b64_ntop((unsigned char *)item->name, strlen(item->name), 
            str, str_len) == -1) {
        str[0] = '\0';
        return str;
    }

    return str;
}

#define M_USER_STR_LIMIT "should be printable characters"
int32
cdb_user_str2val(user_t *item, const char *str, char *err_desc)
{
    char *p;
    memset(item, 0x00, sizeof(user_t));

    if (sal_b64_pton((const char *)str, (unsigned char *)item->name, USERNAME_SIZE) == -1) {
        goto err_out;

    }

    if (strlen(item->name) >= USERNAME_SIZE) {
        goto err_out;
    }

    /* check the chars */
    p = (char *)item->name;
    while ('\0' != *p) {
        if (!(sal_isprint(*p))) {
            goto err_out;
        }
        
        p++;
    }
    
    return PM_E_NONE;

err_out:
    snprintf(err_desc, MAX_CMD_RET_LEN, 
            "Invalid user name: %s, Its length range is [1, %d).", 
            M_USER_STR_LIMIT, USERNAME_SIZE); 
    return PM_E_CFG_INVALID_PARAM;    
}

char*
cdb_snmp_inform_val2str(char *str, uint32 str_len, const snmp_inform_key_t *snmp_inform)
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);
    char szIpBuf[47] = {0};
    char *ip_str = NULL;
    
    if(snmp_inform->is_mgmt_if)
    {
        sal_sprintf(str+len, "%d,",snmp_inform->is_mgmt_if);
        len = sal_strlen(str);
    }

    ip_str = cdb_addr_val2str(szIpBuf, 47, &snmp_inform->ip_addr);
    sal_sprintf(str+len, "%s,", ip_str);
    len = sal_strlen(str);
 
    if(sal_strcmp(snmp_inform->community,""))
    {
        sal_sprintf(str+len, "%s,",snmp_inform->community);
        len = sal_strlen(str);
    }

    sal_sprintf(str+len, "%d,",snmp_inform->udpport);
    len = sal_strlen(str);
       
    return str;
}

int32
cdb_snmp_inform_str2val(char *str, snmp_inform_key_t *snmp_inform)
{
    return PM_E_NONE;
}

char*
cdb_cem_mail_val2str(char *str, uint32 str_len, const mail_t *cem_mail)    
{
    uint32 len = 0;
    sal_memset(str, 0, str_len);
 
    if(sal_strcmp(cem_mail->server, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->server);
        len = sal_strlen(str);
    }
    if(sal_strcmp(cem_mail->from, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->from);
        len = sal_strlen(str);
    }
    if(sal_strcmp(cem_mail->to, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->to);
        len = sal_strlen(str);
    }
    if(sal_strcmp(cem_mail->cc, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->cc);
        len = sal_strlen(str);
    }
    if(sal_strcmp(cem_mail->subject, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->subject);
        len = sal_strlen(str);
    }    
    if(sal_strcmp(cem_mail->body, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->body);
        len = sal_strlen(str);
    } 
    if(sal_strcmp(cem_mail->attach, ""))
    {
        sal_sprintf(str+len, "%s,",cem_mail->attach);
        len = sal_strlen(str);
    }
       
    return str;
}

int32
cdb_cem_mail_str2val(char *str, mail_t *cem_mail) 
{
    return PM_E_NONE;
}

char*
cdb_rsa_keystr_val2str(char *str, uint32 str_len, const rsa_keystr_t *keystr)
{
    size_t len = strlen(keystr->buf);

    str[0] = '\0';
    if (len > 0) {
        if (sal_b64_ntop((unsigned char *)keystr->buf, len, str, str_len) == -1) {
            assert(0);
            return str;
        }
    }
    return str;
}

int32
cdb_rsa_keystr_str2val(char *str, rsa_keystr_t *keystr)
{

    return PM_E_NONE;
}

char*
cdb_mstp_brg_id_val2str(char *str, uint32 str_len, const mstp_brg_id_t *brg_id)
{
    sal_sprintf(str, "%u#%.04hx.%.04hx.%.04hx",
        brg_id->priority,
        sal_ntoh16(((uint16*)brg_id->address)[0]),
        sal_ntoh16(((uint16*)brg_id->address)[1]), 
        sal_ntoh16(((uint16*)brg_id->address)[2]));

    return str;
}

int32
cdb_mstp_brg_id_str2val(char *str, mstp_brg_id_t *brg_id)
{
    return PM_E_NONE;
}


int32
cdb_ns_port_forwarding_key_str2val(ns_port_forwarding_key_t *key, const char *str, char *err_desc)
{    
    int32 rc = PM_E_NONE;
    int proto;
    int type;
    uint16 port;
    char mapped_addr[16] =  {0};

    /* Not used. */
    err_desc = err_desc;
    
#define CDB_NS_PORT_FORWARDING_ARG_NUM 4

    rc = sscanf(str, "%d"CMD_KEY_DIV_STR"%d"CMD_KEY_DIV_STR"%hu"CMD_KEY_DIV_STR"%s", 
                &type, &proto, &port, (char *)mapped_addr);
    if (CDB_NS_PORT_FORWARDING_ARG_NUM != rc)
    {
        return PM_E_INVALID_PARAM;
    }

    key->type = type;
    key->proto = proto;
    key->port = port;

    rc = sal_inet_pton(AF_INET, mapped_addr, &key->mapped_addr);
    if (rc != 1)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid mapped IP address %s", mapped_addr);
        return PM_E_CFG_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

char*
cdb_ns_port_forwarding_key_val2str(char *str, uint32 str_len, const ns_port_forwarding_key_t *key)
{
    uint32 len = 0;
    char mapped_addr[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&key->mapped_addr, mapped_addr, sizeof(mapped_addr));
    
    str[0] = CMD_ZERO_CHAR;

    sal_sprintf(str + len, "%d"CMD_KEY_DIV_STR"%d"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s", 
                key->type, key->proto, key->port, mapped_addr);

    return str;
}

char*
cdb_erps_ring_key_val2str(char *str, uint32 str_len, const erps_ring_key_t *erps_ring_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u",
        erps_ring_key->domain_id,
        erps_ring_key->ring_id);
    return str;
}

int32
cdb_erps_ring_key_str2val(char *str,  erps_ring_key_t *erps_ring_key)
{
    return PM_E_NONE;
}

char*
cdb_lldp_civic_address_t_val2str(char *str, uint32 str_len, const lldp_civic_address_t *civic)
{
    int i = 0;
    
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s",
        civic->len, civic->what, civic->ca_num, civic->countrycode);

    for (i = 0; i < civic->ca_num; i++)
    {
          sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u",
        civic->ca_value[i], civic->ca_type[i], civic->ca_length[i]);
    }
    return str;
}

int32
cdb_lldp_civic_address_t_str2val(char *str, lldp_civic_address_t *civic)
{
    return PM_E_NONE;
}

char*
cdb_dot1x_radius_key_val2str(char *str, uint32 str_len, const dot1x_radius_key_t *key)
{
    char buf[128];

    sal_snprintf(str, str_len, "%s"CMD_KEY_DIV_STR"%d", 
        cdb_addr_val2str(buf, 128, &key->addr), key->sin_port);
    return str;
}

int32
cdb_dot1x_radius_key_str2val(char *str, dot1x_radius_key_t *key)
{
    return PM_E_NONE;
}

char*
cdb_sflow_collector_key_val2str(char *str, uint32 str_len, const sflow_collector_key_t *key)
{
    char buf[128];

    sal_snprintf(str, str_len, "%s"CMD_KEY_DIV_STR"%d", 
        cdb_addr_val2str(buf, 128, &key->addr), key->udp_port);
    return str;
}

int32
cdb_sflow_collector_key_str2val(char *str, sflow_collector_key_t *key)
{
    return  PM_E_NONE;
}

char*
cdb_dot1x_mac_key_val2str(char *str, uint32 str_len, const dot1x_mac_key_t *key)
{
    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%d",
        sal_ntoh16(((uint16*)key->mac)[0]),
        sal_ntoh16(((uint16*)key->mac)[1]), 
        sal_ntoh16(((uint16*)key->mac)[2]),
        key->ifindex);
    return str;
}

int32
cdb_dot1x_mac_key_str2val(char *str, dot1x_mac_key_t *key)
{

    return PM_E_NONE;
}

char*
cdb_mirror_mac_escape_key_val2str(char *str, uint32 str_len, const mirror_mac_escape_key_t *key)
{
    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%.04hX.%.04hX.%.04hX",
        sal_ntoh16(((uint16*)key->mac)[0]),
        sal_ntoh16(((uint16*)key->mac)[1]), 
        sal_ntoh16(((uint16*)key->mac)[2]),
        sal_ntoh16(((uint16*)key->mask)[0]),
        sal_ntoh16(((uint16*)key->mask)[1]), 
        sal_ntoh16(((uint16*)key->mask)[2]));
    return str;
}

int32
cdb_mirror_mac_escape_key_str2val(char *str, mirror_mac_escape_key_t *key)
{

    return PM_E_NONE;
}

char*
cdb_ns_route_key_val2str(char *str, uint32 str_len, const ns_route_key_t *key)
{
    uint32 len = 0;
    char mapped_addr[GLB_IPV6_STR_LEN] =  {0};

    if (AF_INET == key->ip.family)
    {
        sal_inet_ntop(AF_INET, (void *)&key->ip.u.prefix4, mapped_addr, sizeof(mapped_addr));
    }
    else
    {
        sal_inet_ntop(AF_INET6, (void*)&key->ip.u.prefix6, mapped_addr, sizeof(mapped_addr));
    }
    
    str[0] = CMD_ZERO_CHAR;

    sal_sprintf(str + len, "%d"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s", 
                key->proto, key->port, mapped_addr);

    return str;
}

int32
cdb_ns_route_key_str2val(char *str, ns_route_key_t *key)
{
    if (1 == sal_inet_pton(AF_INET, str, &key->ip.u.prefix4))
    {  
        key->ip.family = AF_INET;
        return PM_E_NONE;
    }
    else if (1 == sal_inet_pton(AF_INET6, str, &key->ip.u.prefix6))
    {
        key->ip.family = AF_INET6;
        return PM_E_NONE;
    }
    
    return PM_E_FAIL;
}

char*
cdb_openflow_manager_key_val2str(char *str, uint32 str_len, const openflow_manager_key_t *key)
{
    uint32 len = 0;
    char mapped_addr[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&key->ip, mapped_addr, sizeof(mapped_addr));
    
    str[0] = CMD_ZERO_CHAR;

    sal_sprintf(str + len, "%d"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s", 
                key->proto, key->port, mapped_addr);

    return str;
}

int32
cdb_openflow_manager_key_str2val(char *str, openflow_manager_key_t *key)
{

    return PM_E_NONE;
}

char*
cdb_inband_snat_key_val2str(char *str, uint32 str_len, const inband_snat_key_t *key)
{
    uint32 len = 0;
    char mapped_addr[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&key->ip, mapped_addr, sizeof(mapped_addr));
    
    str[0] = CMD_ZERO_CHAR;

    sal_sprintf(str + len, "%d"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s", 
                key->proto, key->port, mapped_addr);

    return str;
}

int32
cdb_inband_snat_key_str2val(char *str, inband_snat_key_t *key)
{
    return PM_E_NONE;
}

char*
cdb_arpace_config_key_val2str(char *str, uint32 str_len, const arpace_config_key_t *arpace_config_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%d",
        arpace_config_key->arpacl_name, arpace_config_key->seq_no);
    return str;
}

int32
cdb_arpace_config_key_str2val(char *str, arpace_config_key_t *arpace_config_key)
{

    return PM_E_NONE;
}

char*
cdb_ssm_clock_id_val2str(char *str, uint32 str_len, const uint8 *mac)
{
    sal_sprintf(str, "%.04hx.%.04hx.%.04hx.%.04hx",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        sal_ntoh16(((uint16*)mac)[3]));
    return str;
}

int32
cdb_ssm_clock_id_str2val(char *str, ssm_clock_id_t clock_id)
{
    int32 rc = PM_E_NONE;
    rc = cdb_clock_id_address_is_valid(str);
    if (rc < 0)
    {
        return rc;
    }

    if (4 != sal_sscanf(str, "%4hx.%4hx.%4hx.%4hx", (uint16*)&clock_id[0], (uint16*)&clock_id[2], 
                                                     (uint16*)&clock_id[4], (uint16*)&clock_id[6]))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    *(uint16*)&clock_id[0] = sal_hton16(*(uint16*)&clock_id[0]);
    *(uint16*)&clock_id[2] = sal_hton16(*(uint16*)&clock_id[2]);
    *(uint16*)&clock_id[4] = sal_hton16(*(uint16*)&clock_id[4]);
    *(uint16*)&clock_id[6] = sal_hton16(*(uint16*)&clock_id[6]);
    
    if (0 == sal_memcmp(synce_clock_id_all_0, clock_id, SSM_CLOCK_ID_LEN))
    {
        return PM_E_CFG_INVALID_PARAM;
    }
    if (0 == sal_memcmp(synce_clock_id_all_1, clock_id, SSM_CLOCK_ID_LEN))
    {
        return PM_E_CFG_INVALID_PARAM;
    }

    return PM_E_NONE;
}

char*
cdb_rmon_stats_history_key_val2str(char *str, uint32 str_len,
                               const rmon_stats_history_key_t *rmon_stats_history_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u",
        rmon_stats_history_key->history_index,
        rmon_stats_history_key->stats_index);
    return str;
}

int32
cdb_rmon_stats_history_key_str2val(char *str, rmon_stats_history_key_t *rmon_stats_history_key)
{

    return PM_E_NONE;
}

char*
cdb_ipsla_packet_stats_key_val2str(char *str, uint32 str_len,
                               const ipsla_packet_stats_key_t *ipsla_packet_stats_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u",
        ipsla_packet_stats_key->entry_id,
        ipsla_packet_stats_key->packet_index);
    return str;
}

int32
cdb_ipsla_packet_stats_key_str2val(char *str, ipsla_packet_stats_key_t *ipsla_packet_stats_key)
{

    return PM_E_NONE;
}

char*
cdb_ipsla_test_stats_key_val2str(char *str, uint32 str_len,
                             const ipsla_test_stats_key_t *ipsla_test_stats_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u",
        ipsla_test_stats_key->entry_id,
        ipsla_test_stats_key->test_index);
    return str;
}

int32
cdb_ipsla_test_stats_key_str2val(char *str, ipsla_test_stats_key_t *ipsla_test_stats_key)
{

    return PM_E_NONE;
}

char*
cdb_mrt_val2str(char *str, uint32 str_len, const mrt_key_t *mrt_key)
{
    char source[16] =  {0};
    char group[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&mrt_key->source, source, sizeof(source));
    sal_inet_ntop(AF_INET, (void *)&mrt_key->group, group, sizeof(group));
    
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%s",
        mrt_key->vrf_id, source, group);
    return str;
}

int32
cdb_mrt_str2val(char *str, mrt_key_t *mrt_key)
{

    return PM_E_NONE;
}

char*
cdb_mrt_key_val2str(char *str, uint32 str_len,
                               const mrt_key_t *mrt_key)
{
    char source[16] =  {0};
    char group[16] =  {0};

    sal_inet_ntop(AF_INET, (void *)&mrt_key->source, source, sizeof(source));
    sal_inet_ntop(AF_INET, (void *)&mrt_key->group, group, sizeof(group));
    
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%s",
        mrt_key->vrf_id, source, group);
    return str;
}

int32
cdb_mrt_key_str2val(char *str, mrt_key_t *mrt_key)
{

    return PM_E_NONE;
}

char*
cdb_vlan_mapping_entry_key_val2str(char *str, uint32 str_len,
                               const vlan_mapping_entry_key_t *vm_entry_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%s",
        vm_entry_key->raw_vid, vm_entry_key->table_name);
    return str;
}

int32
cdb_vlan_mapping_entry_key_str2val(char *str, vlan_mapping_entry_key_t *vm_entry_key)
{

    return PM_E_NONE;
}

char*
cdb_egress_mapping_entry_key_val2str(char *str, uint32 str_len,
                               const egress_mapping_entry_key_t *vm_entry_key)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%s",
        vm_entry_key->raw_cvid, vm_entry_key->raw_svid, vm_entry_key->table_name);
    return str;
}

int32
cdb_egress_mapping_entry_key_str2val(char *str, egress_mapping_entry_key_t *vm_entry_key)
{

    return PM_E_NONE;
}

char*
cdb_mapped_vlan_info_key_val2str(char *str, uint32 str_len,
                               const mapped_vlan_info_key_t *mapped_info_key)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%s",
        mapped_info_key->evc_name, mapped_info_key->table_name);
    return str;
}

int32
cdb_mapped_vlan_info_key_str2val(char *str, mapped_vlan_info_key_t *mapped_info_key)
{

    return PM_E_NONE;
}

int32
cdb_nd_prefix_key_str2val(const char *str, nd_prefix_key_t *key)
{
#if 0
    char *ip_str = NULL;
    char *type_str = NULL;
    char *len_str = NULL;
    char buf[128];
    int32 rc = PM_E_NONE;

    sal_strncpy(buf, str, 128);
    type_str = sal_strstr(buf, CMD_KEY_DIV_STR);
    if (NULL == type_str || '\0' == *(type_str + 1)) {
        return PM_E_CFG_INVALID_PARAM;
    }

    ip_str = buf;
    *type_str = CMD_ZERO_CHAR;
    type_str += 1;

    if (sal_inet_pton(AF_INET6, ip_str, &key->addr.u.prefix6) == 1) {
        key->addr.family = AF_INET6;
    }

    len_str =  sal_strstr(buf, CMD_DIVISION_CHAR);
    if (NULL == type_str || '\0' == *(type_str + 1)) {
        return PM_E_CFG_INVALID_PARAM;
    }

    key->addr.family = AF_INET6;
    key->addr.pad = 0;

    rc = sal_inet_pton(AF_INET, addr_str, &p_prefix->u.prefix4);
    if (rc != 1)
    {
        sal_snprintf(err_desc, MAX_CMD_RET_LEN, "Invalid IP address %s", addr_str);
        return PM_E_CFG_INVALID_PARAM;
    }

    value = cdb_int_str2val(len_str, &rc);

    rc = cdb_addr_str2val(&key->addr, ip_str, strlen(ip_str));
    if (rc < 0) {
        return rc;
    }

    sal_sscanf(type_str, "%d", &key->ifindex);
#endif

    return PM_E_NONE;
}

char*
cdb_nd_prefix_key_val2str(char *str, uint32 str_len, const nd_prefix_key_t *key)
{
    char buf[128];

    sal_snprintf(str, str_len, "%s"CMD_KEY_DIV_STR"%d", 
        cdb_prefix_val2str(buf, 128, &key->addr), key->ifindex);
    return str;
}

char*
cdb_l2pro_if_action_vlan_val2str(char *str, uint32 str_len, const l2pro_if_action_vlan_t *p_data)
{
    int i = 0;
    int len = 0;

    sal_sprintf(str + len, "l2action(");
    len = sal_strlen(str);
        
    for (i = 0; i < GLB_L2_PROTOCOL_NUM; i++)
    {
        if (i == GLB_L2_PROTOCOL_NUM - 1)
        {
            sal_sprintf(str + len,"%d) ", p_data->l2action[i]);
        }
        else
        {
            sal_sprintf(str + len,"%d"CMD_COMMA_CHAR_STR, p_data->l2action[i]);
        }
        
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len, "l2vlan(");
    len = sal_strlen(str);
    for (i = 0; i < GLB_L2_PROTOCOL_NUM; i++)
    {
        if (i == GLB_L2_PROTOCOL_NUM - 1)
        {
            sal_sprintf(str + len,"%d)", p_data->l2vlan[i]);
        }
        else
        {
            sal_sprintf(str + len,"%d"CMD_COMMA_CHAR_STR, p_data->l2vlan[i]);
        }
        len = sal_strlen(str);
    }
    return str;
}

int32
cdb_l2pro_if_action_vlan_str2val(const char *str, l2pro_if_action_vlan_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_qos_color_action_val2str(char *str, uint32 str_len, glb_qos_color_action_t *p_data)
{
    sal_sprintf(str, "(%u"CMD_COMMA_CHAR_STR"%u"CMD_COMMA_CHAR_STR"%u)", p_data->drop_packet, p_data->tc_valid, p_data->tc);
    return str;
}

int32
cdb_qos_color_action_str2val(const char *str, glb_qos_color_action_t *p_data)
{
    uint32 drop, valid, tc;
    sal_sscanf(str, "(%x"CMD_COMMA_CHAR_STR"%x"CMD_COMMA_CHAR_STR"%x)", &drop, &valid, &tc);
    p_data->drop_packet = drop;
    p_data->tc_valid = valid;
    p_data->tc = tc;
    return PM_E_NONE;
}

char *
cdb_qos_policer_res_key_val2str(char *str, uint32 str_len, qos_policer_res_key_t *p_data)
{
    sal_sprintf(str, "(%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u)", 
        p_data->cir, p_data->eir, p_data->cbs, p_data->ebs, p_data->mode, p_data->pps_en, p_data->direct, p_data->type);
    return str;
}

int32
cdb_qos_policer_res_key_str2val(const char *str, qos_policer_res_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_nat_session_key_val2str(char *str, uint32 str_len, nat_session_key_t *p_data)
{
    int32 len = 0;
    char addr[64];
    char proto[8];

    str[0] = 0;
    proto[0] = 0;
    switch (p_data->ip_protocol)
    {
        case IPPROTO_UDP:
            sal_sprintf(proto, "%s", "UDP");
            break;
        case IPPROTO_TCP:
            sal_sprintf(proto, "%s", "TCP");
            break;  
        case IPPROTO_ICMP:
            sal_sprintf(proto, "%s", "ICMP");
            break;  
        default:
            sal_sprintf(proto, "%u", p_data->ip_protocol);
            break;  
    }
    len += sal_snprintf(str+len, (str_len - len), "(ip_proto=%s"CMD_KEY_DIV_STR, proto);
    len += sal_snprintf(str+len, (str_len - len), "in_local_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->inside_local_ip.u.prefix4));
    len += sal_snprintf(str+len, (str_len - len), "in_glb_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->inside_global_ip.u.prefix4));
    len += sal_snprintf(str+len, (str_len - len), "out_glb_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->outside_global_ip.u.prefix4));
    len += sal_snprintf(str+len, (str_len - len), "out_local_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->outside_local_ip.u.prefix4));

    if ((IPPROTO_UDP == p_data->ip_protocol) || (IPPROTO_TCP == p_data->ip_protocol))
    {
        len += sal_snprintf(str+len, (str_len - len), "in_local_port=%u"CMD_KEY_DIV_STR, p_data->proto_info.l4_port.inside_local);
        len += sal_snprintf(str+len, (str_len - len), "in_glb_port=%u"CMD_KEY_DIV_STR, p_data->proto_info.l4_port.inside_global);
        len += sal_snprintf(str+len, (str_len - len), "out_glb_port=%u"CMD_KEY_DIV_STR, p_data->proto_info.l4_port.outside_global);
        len += sal_snprintf(str+len, (str_len - len), "out_local_port=%u"CMD_KEY_DIV_STR, p_data->proto_info.l4_port.outside_local);
    }

    if (IPPROTO_ICMP == p_data->ip_protocol)
    {
        len += sal_snprintf(str+len, (str_len - len), "icmp_type=%u"CMD_KEY_DIV_STR"icmp_code=%u"CMD_KEY_DIV_STR"icmp_id=%u)"CMD_KEY_DIV_STR, 
                                   p_data->proto_info.icmp.type, p_data->proto_info.icmp.code, p_data->proto_info.icmp.ID);
    }

    len += sal_snprintf(str+len, (str_len - len), "%s", ")");
    return str;
}

int32
cdb_nat_session_key_str2val(const char *str, nat_session_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_poe_cfg_val2str(char *str, uint32 str_len,glb_poe_cfg_t *p_data)
{
    sal_sprintf(str, "(%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u)", 
        p_data->enable, p_data->force_power, p_data->mode, p_data->priority, p_data->power_limit);    
    return str;
}

int32
cdb_poe_cfg_str2val(const char *str, glb_poe_cfg_t *p_data)
{
    return PM_E_NONE;
}



char *
cdb_nat_rule_key_val2str(char *str, uint32 str_len, nat_rule_key_t *p_data)
{
    int32 len = 0;
    char addr[64];

    str[0] = 0;
    len += sal_sprintf(str+len,"(flags=0x%x"CMD_KEY_DIV_STR, p_data->flags);
    len += sal_sprintf(str+len,"proto=%u"CMD_KEY_DIV_STR, p_data->protocol);
    len += sal_sprintf(str+len,"local_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->in_local_ip.u.prefix4));
    len += sal_sprintf(str+len,"local_port=%u"CMD_KEY_DIV_STR, p_data->in_local_port);
    len += sal_sprintf(str+len,"glb_ip=%s"CMD_KEY_DIV_STR, cdb_addr_ipv4_val2str(addr, 64, &p_data->in_glb_ip.u.prefix4));
    len += sal_sprintf(str+len,"glb_port=%u"CMD_KEY_DIV_STR, p_data->in_glb_port);
    len += sal_sprintf(str+len,"glb_port2=%u"CMD_KEY_DIV_STR, p_data->in_glb_port_hi);
    if (0 != p_data->oif[0])
    {
        len += sal_sprintf(str+len,"oif=%s"CMD_KEY_DIV_STR, p_data->oif);
    }

    if (0 != p_data->acl[0])
    {
        len += sal_sprintf(str+len,"acl=%s"CMD_KEY_DIV_STR, p_data->acl);
    }

    if (0 != p_data->pool[0])
    {
        len += sal_sprintf(str+len,"pool=%s"CMD_KEY_DIV_STR, p_data->pool);
    }
    len += sal_sprintf(str+len,"%s", ")");
    return str;
}

int32
cdb_nat_rule_key_str2val(const char *str, nat_rule_key_t *p_data)
{
    return PM_E_NONE;
}

char*
cdb_opb_val2str(char *str, uint32 str_len, const ctclib_opb_t *opb)
{
    uint32 used_len = 0;
    
    sal_snprintf(str, str_len, "desc:%s"CMD_KEY_DIV_STR"words:%u"CMD_KEY_DIV_STR"start:%u"CMD_KEY_DIV_STR"max_size:%u",
        opb->desc, opb->words, opb->start_offset, opb->max_size);
    used_len = sal_strlen(str);
    cdb_bitmap_val2str((str+used_len), (str_len-used_len), opb->data, opb->words);
    
    return str;
}

int32
cdb_opb_str2val(const char *str, ctclib_opb_t *p_data)
{
    return PM_E_NONE;
}

int32
cdb_nat_proxy_arp_key_str2val(const char *str, nat_proxy_arp_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_nat_proxy_arp_key_val2str(char *str, uint32 str_len, nat_proxy_arp_key_t *p_data)
{
    int32 len = 0;
    char addr[64];

    str[0] = 0;
    len += sal_sprintf(str+len,"(ifindex=%u"CMD_KEY_DIV_STR, p_data->ifindex);
    len += sal_sprintf(str+len,"in_local_ip=%s)", cdb_addr_ipv4_val2str(addr, 64, &p_data->inside_global_ip.u.prefix4));
    return str;
}

char *
cdb_cfm_ma_key_val2str(char *str, uint32 str_len, cfm_ma_key_t *p_data)
{
    sal_sprintf(str, "%s"CMD_KEY_DIV_STR"%u", p_data->name, p_data->vlan_id);
    return str;
}

int32
cdb_cfm_ma_key_str2val(const char *str, cfm_ma_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_cfm_lmep_key_val2str(char *str, uint32 str_len, cfm_lmep_key_t *p_data)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u", p_data->level, p_data->vlan_id, p_data->ifindex);
    return str;
}

int32
cdb_cfm_lmep_key_str2val(const char *str, cfm_lmep_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_cfm_rmep_key_val2str(char *str, uint32 str_len, cfm_rmep_key_t *p_data)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u"CMD_KEY_DIV_STR"%u", p_data->level, p_data->vlan_id, p_data->rmep_id);
    return str;
}

int32
cdb_cfm_rmep_key_str2val(const char *str, cfm_rmep_key_t *p_data)
{
    return PM_E_NONE;
}

char *
cdb_cfm_time_key_val2str(char *str, uint32 str_len, cfm_time_key_t *p_data)
{
    sal_sprintf(str, "%u"CMD_KEY_DIV_STR"%u", p_data->time, p_data->seq);
    return str;
}

int32
cdb_cfm_time_key_str2val(const char *str, cfm_time_key_t *p_data)
{
    return PM_E_NONE;
}

/*add by zhw for mc*/
char*
cdb_pim_sm_gprefix_val2str(char *str, uint32 str_len, const pim_sm_gprefix_t *gprefix)
{    
    sal_printf(str,"%s"CMD_KEY_DIV_STR"%u", gprefix->ip, gprefix->masklen);
    return str;
}    
     
char*
cdb_pim_static_rp_addr_val2str(char *str, uint32 str_len, const pim_static_rp_addr_t *gprefix)
{    
    sal_printf(str,"%s"CMD_KEY_DIV_STR"%s"CMD_KEY_DIV_STR"%u", gprefix->ip, gprefix->group, gprefix->masklen);
    return str;
}    
int32 cdb_pim_sm_gprefix_str2val(char *str, const pim_sm_gprefix_t *gprefix)
{    
    return PM_E_NONE;
}    
     
int32 cdb_pim_static_rp_addr_str2val(char *str, const pim_static_rp_addr_t *gprefix)
{    
    return PM_E_NONE;
     
}    
/*end add zhw*/

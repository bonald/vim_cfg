
#ifndef __GENLOG_ID_H__
#define __GENLOG_ID_H__

typedef enum
{
    LOG_FAN_3_FAIL                           = 0,
    LOG_FAN_4_OK                             = 1,
    LOG_INTERFACE_4_CREATE                   = 2,
    LOG_INTERFACE_4_UPDOWN                   = 3,
    LOG_INTERFACE_4_ERRDISABLE_ENTER         = 4,
    LOG_INTERFACE_4_BANDWIDTH_OVERFLOW       = 5,
    LOG_INTERFACE_4_BANDWIDTH_RESUME         = 6,
    LOG_INTERFACE_4_DROP_OVERFLOW            = 7,
    LOG_INTERFACE_4_DROP_RESUME              = 8,
    LOG_INTERFACE_4_ERRDISABLE_EXIT          = 9,
    LOG_INTERFACE_4_CRCPKT                   = 10,
    LOG_LAG_4_ADD                            = 11,
    LOG_LAG_4_DEL                            = 12,
    LOG_LAG_4_ATTACH_MEMBER                  = 13,
    LOG_LAG_4_DETACH_MEMBER                  = 14,
    LOG_LAG_4_CHANGE_ACTIVE                  = 15,
    LOG_LAG_4_CHANGE_INACTIVE                = 16,
    LOG_LLDP_4_ADD_NEIGH                     = 17,
    LOG_LLDP_4_DEL_NEIGH                     = 18,
    LOG_MLAG_4_ESTABLISH                     = 19,
    LOG_MLAG_3_BREAKOFF                      = 20,
    LOG_OSPF_4_NEIGHCHG                      = 21,
    LOG_PSU_3_ABSENT                         = 22,
    LOG_PSU_4_PRESENT                        = 23,
    LOG_PSU_3_ABNORMAL                       = 24,
    LOG_PSU_3_ALERT                          = 25,
    LOG_PSU_4_NORMAL                         = 26,
    LOG_PSU_4_NOALERT                        = 27,
    LOG_PSU_4_PMBUS                          = 28,
    LOG_ROUTE_4_FIB_OVERFLOW                 = 29,
    LOG_ROUTE_4_STATIC_ROUTE_OVERFLOW        = 30,
    LOG_ROUTE_6_FIB_OVERFLOW                 = 31,
    LOG_ROUTE_6_STATIC_ROUTE_OVERFLOW        = 32,
    LOG_ROUTE_4_INDIRECT_ROUTE_OVERFLOW      = 33,
    LOG_ROUTE_6_INDIRECT_ROUTE_OVERFLOW      = 34,
    LOG_ROUTE_3_STM_INC_DEC_ERR              = 35,
    LOG_ROUTE_4_ECMPGROUP_OVERFLOW           = 36,
    LOG_ARP_4_OVERFLOW                       = 37,
    LOG_SYSTEM_4_REBOOT                      = 38,
    LOG_SYSTEM_4_STARTUP                     = 39,
    LOG_SYSTEM_4_WRITECONFIGURE              = 40,
    LOG_SYSTEM_4_USERLOGIN                   = 41,
    LOG_SYSTEM_4_TIMECHANGE                  = 42,
    LOG_SYSTEM_4_TIMEUPDATE                  = 43,
    LOG_SYSTEM_4_TIMEUPDATE_BY_PTP           = 44,
    LOG_SYSTEM_4_TIMEZONE_SET                = 45,
    LOG_SYSTEM_4_TIMEZONE_CLEAR              = 46,
    LOG_SYSTEM_4_CPU_THRESHOLD               = 47,
    LOG_SYSTEM_4_CPU_RESUME                  = 48,
    LOG_SYSTEM_1_LOAD_CRITICAL               = 49,
    LOG_SYSTEM_2_LOAD_ALERT                  = 50,
    LOG_SYSTEM_3_LOAD_WARNING                = 51,
    LOG_SYSTEM_4_LOAD_NORMAL                 = 52,
    LOG_SYSTEM_0_MEM_CRITICAL                = 53,
    LOG_SYSTEM_1_MEM_ALERT                   = 54,
    LOG_SYSTEM_3_MEM_WARNING                 = 55,
    LOG_SYSTEM_4_MEM_NORMAL                  = 56,
    LOG_SYSTEM_3_MEM_PACKETDISCARD           = 57,
    LOG_TEMPERATURE_3_ALARM                  = 58,
    LOG_TEMPERATURE_4_NORMAL                 = 59,
    LOG_TRANSCEIVER_4_INSERT                 = 60,
    LOG_TRANSCEIVER_4_REMOVE                 = 61,
    LOG_TRANSCEIVER_3_ALARM                  = 62,
    LOG_TRANSCEIVER_4_WARN                   = 63,
    LOG_TRANSCEIVER_4_NORMAL                 = 64,
    LOG_FDB_4_OVERFLOW                       = 65,
    LOG_FDB_4_FDBLOOP                        = 66,
    LOG_ERPS_4_BLOCK                         = 67,
    LOG_TIMERANGE_4_STATE                    = 68,
    LOG_BHM_1_REBOOT                         = 69,
    LOG_BHM_1_SHUTDOWN                       = 70,
    LOG_BHM_1_WARNING                        = 71,
    LOG_BHM_1_PROMPT                         = 72,
    LOG_AAA_4_FAIL                           = 73,
    LOG_DOT1X_4_FAIL                         = 74,
    LOG_ARPINSP_4_RESULT                     = 75,
    LOG_ARPINSP_3_CHECK                      = 76,
    LOG_ARPINSP_5_ACL                        = 77,
    LOG_SFLOW_4_FAIL                         = 78,
    LOG_ARPRATELIMIT_4_EXCEED                = 79,
    LOG_ARPRATELIMIT_4_ABNORMAL              = 80,
    LOG_ARPNUMBERLIMIT_4_EXCEED              = 81,
    LOG_ARPNUMBERLIMIT_4_ABNORMAL            = 82,
    LOG_MONITOR_3_LATENCY_START              = 83,
    LOG_MONITOR_4_LATENCY_END                = 84,
    LOG_MONITOR_3_BUFFER_START               = 85,
    LOG_MONITOR_4_BUFFER_END                 = 86,
    LOG_VRRP_4_VRRP_STATE                    = 87,
    LOG_OSPF_4_LSA_ROUTE                     = 88,
    LOG_SSM_5_PORT_SELECT                    = 89,
    LOG_SSM_4_STATE_CHANGE                   = 90,
    LOG_PTP_4_PTP_STATE_CHANGE               = 91,
    LOG_PTP_4_PTP_MASTER_CHANGE              = 92,
    LOG_PTP_4_PTP_ACCURACY_CHANGE            = 93,
    LOG_IPSLA_4_IPSLA_EVENT                  = 94,
    LOG_TRACK_4_TRACK_EVENT                  = 95,
    LOG_TRACK_4_STATIC_RT_UPDATE             = 96,
    LOG_OAM_4_TPOAM_CCM_STATE_CHANGE         = 97,
    LOG_OAM_4_TPOAM_LM_EVENT                 = 98,
    LOG_OAM_4_TPOAM_DM_EVENT                 = 99,
    LOG_OAM_4_G8131_LINK_CHANGE              = 100,
    LOG_ND_4_G8131_LINK_CHANGE               = 101,
    LOG_RMON_4_RISING_ALARM                  = 102,
    LOG_RMON_4_FALLING_ALARM                 = 103,
    LOG_OAM_4_G8032_STATE_CHANGE             = 104,
    LOG_OAM_4_G8032_CONFIG_ALARM             = 105,
    LOG_ID_MAX
} E_LOGID;

#endif /* !__GENLOG_ID_H__ */


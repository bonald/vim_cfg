
#ifndef __SNMPAGT_DEFINE_H__
#define __SNMPAGT_DEFINE_H__

#include "proto_config.h"
#include "proto_error.h"
#include "proto_module.h"
#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_ip_define.h"
#include "glb_acl_define.h"
#include "glb_ptp_define.h"
#include "glb_qos_define.h"
#include "glb_tempfile_define.h"
#include "glb_nat_define.h"
#include "ctclib_slist.h"
#include "ctclib_list.h"
#include "ctclib_hash.h"
#include "ctclib_queue.h"
#include "ctclib_if.h"
#include "ctclib_memory.h"
#include "ctc_task.h"
#include "genlog.h"
#include "ctc_sock.h"
#include "cdb_const.h"
#include "cfg_cmd.h"
#include "cdb.h"
#include "ctclib_memory.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_manage_if.h"
#include "gen/tbl_sensor_define.h"
#include "gen/tbl_sensor.h"
#include "gen/tbl_fiber_define.h"
#include "gen/tbl_fiber.h"
#include "gen/tbl_psu_define.h"
#include "gen/tbl_psu.h"
#include "gen/tbl_fan_define.h"
#include "gen/tbl_fan.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "net-snmp/agent/agent_callbacks.h"
#include "net-snmp/agent/ds_agent.h"
#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_iterator.h>
#include <net-snmp/agent/table_data.h>
#include <net-snmp/agent/table_dataset.h>

#define SNMP_MAX_DEPTH      64

typedef struct
{
    char       *argv[SNMP_MAX_DEPTH];
    uint32      argc;
} snmpagt_parse_info_t;

typedef struct
{
    int colno;
    int type;
    int writable;
} snmpagt_row_t;

typedef union 
{
   long     integer;
   char     string[SNMP_INFO_SIZE];
   oid      objid;
   uint64   counter64;
   float    floatVal;
   double   doubleVal;
} snmpagt_vardata_t;

typedef struct
{
    char oid_str[CMD_BUF_256];
    struct variable *vars;
    uint32 var_num;
} snmpagt_nodes_t;

typedef struct
{
    char *oid_str;
    uint32 no_idx_col;
    uint32 index_type[4];
    uint32 index_count;
    snmpagt_row_t *row;
    uint32 row_count;
    void *nodehandler;  // Netsnmp_Node_Handler
} snmpagt_table_param_t;

typedef struct
{
    snmpagt_table_param_t param;
    netsnmp_table_data_set *p_dataset;
    uint32 _col_writable[2];
} snmpagt_table_t;

#define PUBLICMIB_OID       1, 3, 6, 1, 2, 1
#define CTCENTERPRISE_ID    27975               /* will be replace */
#define CTCENTERPRISE_OID   1, 3, 6, 1, 4, 1, CTCENTERPRISE_ID

/*
static oid SYSDESCR_OID []      = { PUBLICMIB_OID, 1, 1 };
static oid SYSUPTIME_OID []     = { PUBLICMIB_OID, 1, 3 };
static oid SYSCONTACT_OID []    = { PUBLICMIB_OID, 1, 4 };
static oid SYSNAME_OID []       = { PUBLICMIB_OID, 1, 5 };
static oid SYSLOCATION_OID []   = { PUBLICMIB_OID, 1, 6 };
static oid IFTABLE_OID []       = { PUBLICMIB_OID, 2, 2 };

*/

/*
################################################################################
#
# ifTable 1.3.6.1.2.1.2.2
#
################################################################################
*/
#define IF_TBL_MEM_INDEX               1
#define IF_TBL_MEM_DESCR               2
#define IF_TBL_MEM_TYPE                3
#define IF_TBL_MEM_MTU                 4
#define IF_TBL_MEM_SPEED               5
#define IF_TBL_MEM_PHYS_ADDRESS        6
#define IF_TBL_MEM_ADMIN_STATUS        7
#define IF_TBL_MEM_OPER_STATUS         8
#define IF_TBL_MEM_LAST_CHANGE         9
#define IF_TBL_MEM_IN_OCTECTS          10
#define IF_TBL_MEM_IN_UCASTPKTS        11
#define IF_TBL_MEM_IN_NUCASTPKTS       12
#define IF_TBL_MEM_IN_DISCARDS         13
#define IF_TBL_MEM_IN_ERRORS           14
#define IF_TBL_MEM_IN_UNKNOWN_PROTOS   15
#define IF_TBL_MEM_OUT_OCTECTS         16
#define IF_TBL_MEM_OUT_UCAST_PKTS      17
#define IF_TBL_MEM_OUT_NUCAST_PKTS     18
#define IF_TBL_MEM_OUT_DISCARDS        19
#define IF_TBL_MEM_OUT_ERRORS          20
#define IF_TBL_MEM_OUT_QLEN            21
#define IF_TBL_MEM_SPECIFIC            22

/*
################################################################################
#
# ifXTable 1.3.6.1.2.1.31.1.1
#
################################################################################
*/
#define IFX_TBL_MEM_NAME                        1
#define IFX_TBL_MEM_IN_MULTICAST_PKTS           2
#define IFX_TBL_MEM_IN_BROADCAST_PKTS           3
#define IFX_TBL_MEM_OUT_MULTICAST_PKTS          4
#define IFX_TBL_MEM_OUT_BROADCAST_PKTS          5
#define IFX_TBL_MEM_HC_IN_OCTETS                6
#define IFX_TBL_MEM_HC_IN_UCAST_PKTS            7
#define IFX_TBL_MEM_HC_IN_MULTICAST_PKTS        8
#define IFX_TBL_MEM_HC_IN_BROADCAST_PKTS        9
#define IFX_TBL_MEM_HC_OUT_OCTETS               10
#define IFX_TBL_MEM_HC_OUT_UCAST_PKTS           11
#define IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS       12
#define IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS       13
#define IFX_TBL_MEM_LINK_UP_DOWN_TRAP_ENABLE    14
#define IFX_TBL_MEM_HIGH_SPEED                  15
#define IFX_TBL_MEM_PROMISCUOUS_MODE            16
#define IFX_TBL_MEM_CONNECTOR_PRESENT           17
#define IFX_TBL_MEM_ALIAS                       18
#define IFX_TBL_MEM_COUNTER_DISCONTINUTY_TIME   19


/*
################################################################################
#
#interfaceTable
#
################################################################################
# Oid indexs
*/
#define INTERFACE_TBL_INDEX                      1
#define INTERFACE_TBL_NAME                       2
#define INTERFACE_TBL_SPEED                      7
#define INTERFACE_TBL_DUPLEX                     8
#define INTERFACE_TBL_RESET_CNT                  9
#define INTERFACE_TBL_BANDWIDTH                  15
#define INTERFACE_TBL_UNIDIRECTIONAL             31
#define INTERFACE_TBL_ENGRESS_FLOWNAME           32
#define INTERFACE_TBL_LINK_STATUS                33
#define INTERFACE_TBL_RESET_FLOWCNT              34
#define INTERFACE_TBL_RESET_EGS_FLOWCNT          35
#define INTERFACE_TBL_5MIN_IN_BITS_RATE          41
#define INTERFACE_TBL_5MIN_IN_PKTS_RATE          42
#define INTERFACE_TBL_5MIN_OUT_BITS_RATE         43
#define INTERFACE_TBL_5MIN_OUT_PKTS_RATE         44

/*
################################################################################
#
# ifXTable 1.3.6.1.2.1.47.1.1
#
################################################################################
*/
#define ENTPHY_TBL_SW_REV      10
#define ENTPHY_TBL_SERIAL_NO    11
#define ENTPHY_TBL_HW_TYPE      13

/*
################################################################################
#
#GLOBAL DEFINES
#
################################################################################
*/
#define GLB_STATE_ENABLE                         1
#define GLB_STATE_DISABLE                        2
#define GLB_UNIDIR_RX_ONLY                       3
#define GLB_UNIDIR_TX_ONLY                       4

#define GLB_COUNTER32_MAX                        0xffffffff //4294967295
 
//#define GLB_IFF_UP                               1
//#define GLB_IFF_RUNNING                          0x40
 
#define GLB_IF_TYPE_NULL                         0
#define GLB_IF_TYPE_PORT_IF                      1
#define GLB_IF_TYPE_LINKAGG_IF                   2
#define GLB_IF_TYPE_VLAN_IF                      3
#define GLB_IF_TYPE_SUB_IF                       4
#define GLB_IF_TYPE_LOOPBACK_IF                  5
#define GLB_IF_TYPE_TUNNEL_IF                    6
#define GLB_IF_TYPE_MAX                          7

#define GLB_IF_TYPE_ETHERNETCSMACD               6
#define GLB_IF_TYPE_SOFTWARELOOPBACK             24
#define GLB_IF_TYPE_TUNNEL                       131
#define GLB_IF_TYPE_VLANIF                       136
#define GLB_IF_TYPE_AGG                          161

/*
################################################################################
#
#fiberTable
#
################################################################################
*/
#define TBL_FIBER_INDEX                         1
#define TBL_FIBER_TX_PWR2                       2
#define TBL_FIBER_RX_PWR2                       3
#define TBL_FIBER_TX_HIGH_ALARM                 4
#define TBL_FIBER_RX_HIGH_ALARM                 5
#define TBL_FIBER_TX_HIGH_WARN                  6
#define TBL_FIBER_RX_HIGH_WARN                  7
#define TBL_FIBER_TX_LOW_ALARM                  8
#define TBL_FIBER_RX_LOW_ALARM                  9
#define TBL_FIBER_TX_LOW_WARN                   10
#define TBL_FIBER_RX_LOW_WARN                   11

/*
################################################################################
#
#fanTable
#
################################################################################
*/
#define TBL_FAN_INDEX                           1
#define TBL_FAN_MODULE                          3
#define TBL_FAN_STATUS                          4
#define TBL_FAN_SPEED                           5

/*
################################################################################
#
# tempr Tables
#
################################################################################
*/
#define TBL_TEMPR_INDEX                         1
#define TBL_TEMPR_TEMPERATURE                   2
#define TBL_TEMPR_LOW                           3
#define TBL_TEMPR_HIGH                          4
#define TBL_TEMPR_CRITICAL                      5
#define TBL_TEMPR_LOCATION                      6

#define TBL_TMP_POSISION_CPU                    1
#define TBL_TMP_POSISION_BEFOR_CHIP             2
#define TBL_TMP_POSISION_BEHIND_CHIP            3
#define TBL_TMP_POSISION_FAN                    4
#define TBL_TMP_POSISION_PSU                    5
#define TBL_TMP_POSISION_CHIP0                  6
#define TBL_TMP_POSISION_CHIP1                  7
#define TBL_TMP_POSISION_AROUND_CHIP            8

/*
################################################################################
#
# power Tables
#
################################################################################
*/
#define TBL_POWER_INDEX                         1
#define TBL_POWER_STATUS                        2
#define TBL_POWER_WORK_STATUS                   3
#define TBL_POWER_TYPE                          4
#define TBL_POWER_ALERT_STATUS                  7
   
#define TBL_POWER_PRESENT_VAL                   1
#define TBL_POWER_ABSENT_VAL                    2


/*
################################################################################
#
# RMON
#
################################################################################
*/

#define RMONMIBOID          1,3,6,1,2,1,16

/*
 * RMON MIB etherStatsTable 
 */
#define TBL_RMON_ETHER_STATS_INDEX                1
#define TBL_RMON_ETHER_STATS_DATASOURCE           2
#define TBL_RMON_ETHER_STATS_DROPEVENTS           3
#define TBL_RMON_ETHER_STATS_OCTETS               4
#define TBL_RMON_ETHER_STATS_PKTS                 5
#define TBL_RMON_ETHER_STATS_BROADCASTPKTS        6
#define TBL_RMON_ETHER_STATS_MULTICASTPKTS        7
#define TBL_RMON_ETHER_STATS_CRCALIGNERRORS       8
#define TBL_RMON_ETHER_STATS_UNDERSIZEPKTS        9
#define TBL_RMON_ETHER_STATS_OVERSIZEPKTS         10
#define TBL_RMON_ETHER_STATS_FRAGMENTS            11
#define TBL_RMON_ETHER_STATS_JABBERS              12
#define TBL_RMON_ETHER_STATS_COLLISIONS           13
#define TBL_RMON_ETHER_STATS_PKTS64OCTETS         14
#define TBL_RMON_ETHER_STATS_PKTS65TO127OCTETS    15
#define TBL_RMON_ETHER_STATS_PKTS128TO255OCTETS   16
#define TBL_RMON_ETHER_STATS_PKTS256TO511OCTETS   17
#define TBL_RMON_ETHER_STATS_PKTS512TO1023OCTETS  18
#define TBL_RMON_ETHER_STATS_PKTS1024TO1518OCTETS 19
#define TBL_RMON_ETHER_STATS_OWNER                20
#define TBL_RMON_ETHER_STATS_STATUS               21


/*
 * RMON MIB historyControlTable
 */
#define TBL_RMON_HISTORY_INDEX                     1
#define TBL_RMON_HISTORY_DATASOURCE                2
#define TBL_RMON_HISTORY_BUCKETSREQUESTED          3
#define TBL_RMON_HISTORY_BUCKETSGRANTED            4
#define TBL_RMON_HISTORY_INTERVAL                  5
#define TBL_RMON_HISTORY_OWNER                     6
#define TBL_RMON_HISTORY_STATUS                    7


/*
 * RMON MIB etherHistoryTable
 */
#define TBL_RMON_ETHER_HISTORY_INDEX                 1
#define TBL_RMON_ETHER_HISTORY_SAMPLEINDEX           2
#define TBL_RMON_ETHER_HISTORY_INTERVALSTART         3
#define TBL_RMON_ETHER_HISTORY_DROPEVENTS            4
#define TBL_RMON_ETHER_HISTORY_OCTETS                5
#define TBL_RMON_ETHER_HISTORY_PKTS                  6
#define TBL_RMON_ETHER_HISTORY_BROADCASTPKTS         7
#define TBL_RMON_ETHER_HISTORY_MULTICASTPKTS         8
#define TBL_RMON_ETHER_HISTORY_CRCALIGNERRORS        9
#define TBL_RMON_ETHER_HISTORY_UNDERSIZEPKTS         10
#define TBL_RMON_ETHER_HISTORY_OVERSIZEPKTS          11
#define TBL_RMON_ETHER_HISTORY_FRAGMENTS             12
#define TBL_RMON_ETHER_HISTORY_JABBERS               13
#define TBL_RMON_ETHER_HISTORY_COLLISIONS            14
#define TBL_RMON_ETHER_HISTORY_UTILIZATION           15


#define RISING_ALARM                            1
#define RISING_OR_FALLING_ALARM                 3

/*
 * RMON MIB alarmTable
 */
#define TBL_RMON_ALARM_INDEX                     1
#define TBL_RMON_ALARM_INTERVAL                  2
#define TBL_RMON_ALARM_VARIABLE                  3
#define TBL_RMON_ALARM_SAMPLETYPE                4
#define TBL_RMON_ALARM_VALUE                     5
#define TBL_RMON_ALARM_STARTUPALARM              6
#define TBL_RMON_ALARM_RISINGTHRESHOLD           7
#define TBL_RMON_ALARM_FALLINGTHRESHOLD          8
#define TBL_RMON_ALARM_RISINGEVENTINDEX          9
#define TBL_RMON_ALARM_FALLINGEVENTINDEX         10
#define TBL_RMON_ALARM_OWNER                     11
#define TBL_RMON_ALARM_STATUS                    12


/*
 * RMON MIB eventTable
 */
#define TBL_RMON_EVENT_INDEX                    1
#define TBL_RMON_EVENT_DESCRIPTION              2
#define TBL_RMON_EVENT_TYPE                     3
#define TBL_RMON_EVENT_COMMUNITY                4
#define TBL_RMON_EVENT_LASTTIMESENT             5
#define TBL_RMON_EVENT_OWNER                    6
#define TBL_RMON_EVENT_STATUS                   7





#define SNMPAGT_RETURN_INTEGER(V) \
do { \
    *var_len = sizeof(int32); \
    return (u_char*)&V; \
} while (0)

#define SNMPAGT_RETURN_TIMETICKS(V) \
do { \
    *var_len = sizeof(long); \
    return (u_char*)&V; \
} while (0)

#define SNMPAGT_RETURN_IPADDRESS(V) \
do { \
    *var_len = sizeof(struct sal_in4_addr); \
    return (u_char*)&V; \
} while (0)

#define SNMPAGT_RETURN_STRING(V,L) \
do { \
    *var_len = L; \
    return (u_char*)(V); \
} while (0)

#define SNMPAGT_RETURN_OID(V,L) \
do { \
    *var_len = (L)*sizeof(oid); \
    return (u_char*)(V); \
} while (0)

extern int
header_generic(struct variable *, oid *, size_t *, int, size_t *,
                   WriteMethod **);
int32
snmpagt_nodes_create(snmpagt_nodes_t *p_node);
extern snmpagt_table_t*
snmpagt_table_create(snmpagt_table_param_t *param);
netsnmp_table_row*
snmpagt_table_get_row_by_int32_index(snmpagt_table_t *p_table, int32 index);
netsnmp_table_row*
snmpagt_table_get_row_by_int32_double_index(snmpagt_table_t *p_table, int32 index1, int32 index2);

#endif /* !__SNMPAGT_DEFINE_H__ */


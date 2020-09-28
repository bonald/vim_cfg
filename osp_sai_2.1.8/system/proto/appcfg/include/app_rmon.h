
#ifndef __RMON_API_H__
#define __RMON_API_H__

#include "lib_snmp.h"

/*
  RMON GET, SET CODE
  
  This module declares the instrumentation code to the RMON
  
*/
#define RMON_API_GET_SUCCESS           0
#define RMON_API_SET_SUCCESS           0
#define RMON_API_SET_FAILURE          -1
#define RMON_API_GET_FAILURE          -1
#define RMON_API_VARIABLE_SET_FAILURE -2

#define RMON_NONE                  0
#define RMON_NATIVE                1
#define RMON_PROMISCUOUS           2
#define RMON_INDEX_MIN             1
#define RMON_INDEX_MAX             65535

#define RMON_BUCKET_NUM_MAX        10
#define RMON_INTERVAL_MIN          1
#define RMON_INTERVAL_PROM_MIN     10
#define RMON_INTERVAL_MAX          65535
#define RMON_EVENT_NUM_MAX         128
#define RMON_ALARM_NUM_MAX         32

#define RMON_MIN_THRESHOLD         (-(~(1 << (8*sizeof(int) -1))) - 1)
#define RMON_MAX_THRESHOLD         ~(1 << (8*sizeof(int) -1))

#define RMON_ALARM_ABS             1
#define RMON_ALARM_DELTA           2

#define MAX_OWNER_STR_LEN          254

#define ETHER_STATS_INDEX_POS      11

#define RMON_DEFAULT_NAME          "RMON_SNMP"

/* Rmon default values */

#define RMON_MODE_DEFAULT                      0
#define RMON_STATUS_VALID                      1
#define RMON_STATUS_CREATEREQUEST              2
#define RMON_STATUS_UNDERCREATION              3
#define RMON_STATUS_INVALID                    4
#define RMON_STATUS_NONEXISTENT                5

#define RMON_ERR_BASE                         -100
#define RMON_ERR_NOMEM                        (RMON_ERR_BASE + 1)

#define RMON_MODE_DEFAULT                      0

#define VALID_STATUS                           1
#define CREATE_REQ_STATUS                      2
#define UNDER_CREATION_STATUS                  3
#define INVALID_STATUS                         4

#define RMON_EVENT_NONE                        1 
#define RMON_EVENT_LOG                         2
#define RMON_EVENT_TRAP                        3
#define RMON_EVENT_LOG_TRAP                    4

/* Minimum and Maximum Allowable values for various paramters */
#define RMON_ETHER_STATS_                      1
#define RMON_HISTORY_CONTROL_                  1
//#define RMON_ALARM_VAR_WORD_LENGTH             64
#define RMON_EVENT_                            7
#define RMON_COMM_LENGTH                       255       /* change from 127->255 for CNOS system */
//#define RMON_DESCR_LENGTH                      127
//#define RMON_OWNER_NAME_SIZE                   15

/* History control table defaults */
//#define RMON_HISTORY_CTRL_BR_DEFAULT           5 
//#define RMON_HISTORY_CTRL_BG_DEFAULT           5
//#define RMON_HISTORY_CTRL_SAMPLE_INTERVAL_DEFAULT  1800  /* In secs */

#define RMON_ROW_AGING_TIME                    300

/* EtherStats Variable Strings */
#define ETHERSTATSINDEXSTR                     "etherStatsIndex"
#define ETHERSTATSDSRCSTR                      "etherStatsDataSource"
#define ETHERSTATSDROPEVENTSVSTR               "etherStatsDropEvents"
#define ETHERSTATSOCTETSSTR                    "etherStatsOctets"
#define ETHERSTATSPKTSSTR                      "etherStatsPkts"
#define ETHERSTATSBDCPKTSSTR                   "etherStatsBroadcastPkts"
#define ETHERSTATSMCPKTSSTR                    "etherStatsMulticastPkts"
#define ETHERSTATSCRCALIGNERRSTR               "etherStatsCRCAlignErrors"
#define ETHERSTATSUSZPKTSSTR                   "etherStatsUndersizePkts"
#define ETHERSTATSOVSZPKTSSTR                  "etherStatsOversizePkts"
#define ETHERSTATSFRGSSTR                      "etherStatsFragments"
#define ETHERSTATSJABBERSSTR                   "etherStatsJabbers"
#define ETHERSTATSCOLLISIONSSTR                "etherStatsCollisions"
#define ETHERSTATSPKT64OCTSTR                  "etherStatsPkts64Octets"
#define ETHERSTATSPKT65TO127OCTSTR             "etherStatsPkts65to127Octets"
#define ETHERSTATSPKT128TO255OCTSTR            "etherStatsPkts128to255Octets"
#define ETHERSTATSPKT256TO511OCTSTR            "etherStatsPkts256to511Octets"
#define ETHERSTATSPKT512TO1023OCTSTR           "etherStatsPkts512to1023Octets"
#define ETHERSTATSPKT1024TO15180OCTSTR         "etherStatsPkts1024to1518Octets"
#define ETHERSTATSOWNERSTR                     "etherStatsOwner"
#define ETHERSTATSSTATUSSTR                    "etherStatsStatus"

#define RMON_INTERFACE_INDEX_POSITION 11

/* Macros used to find out whether the user has given etherStatsEntry.x.x format or etherStatsPkts.x format */
#define RMON_ETHER_STATS_ENTRY                 1
#define RMON_ETHER_STATS_VARIABLE              2

#define ETHERSTATSENTRY                        "etherStatsEntry."
#define ETHERSTATSNUM                          "1.3.6.1.2.1.16.1.1.1"

#define RMON_ALARM_OWNER_CONF                 (1 << 0)


#define RMON_INFO_SET(R,V)                                                   \
   do {                                                                      \
      (R)->info = (V);                                                       \
   } while (0)

#define RMON_INFO_UNSET(R)                                                   \
   do {                                                                      \
     (R)->info = NULL;                                                       \
   } while (0)

/*
 * this is the root OID we use to register the objects handled in this
 * module.
 */
#define RMONMIBOID 1,3,6,1,2,1,16

/* 
 * ZebOS enterprise RMON-MIB equivalent root OID.  This variable is used to
 * register the RMON-MIB to SNMP agent under SMUX protocol.
 */
#define RMONDMIBOID 1,3,6,1,4,1,3317,1,2,25

/*
 * RMON MIB etherStatsTable 
 */
#define ETHERSTATSINDEX                         1
#define ETHERSTATSDATASOURCE                    2
#define ETHERSTATSDROPEVENTS                    3
#define ETHERSTATSOCTETS                        4
#define ETHERSTATSPKTS                          5
#define ETHERSTATSBROADCASTPKTS                 6
#define ETHERSTATSMULTICASTPKTS                 7
#define ETHERSTATSCRCALIGNERRORS                8
#define ETHERSTATSUNDERSIZEPKTS                 9
#define ETHERSTATSOVERSIZEPKTS                  10
#define ETHERSTATSFRAGMENTS                     11
#define ETHERSTATSJABBERS                       12
#define ETHERSTATSCOLLISIONS                    13
#define ETHERSTATSPKTS64OCTETS                  14
#define ETHERSTATSPKTS65TO127OCTETS             15
#define ETHERSTATSPKTS128TO255OCTETS            16
#define ETHERSTATSPKTS256TO511OCTETS            17
#define ETHERSTATSPKTS512TO1023OCTETS           18
#define ETHERSTATSPKTS1024TO1518OCTETS          19
#define ETHERSTATSOWNER                         20
#define ETHERSTATSSTATUS                        21


/*
 * RMON MIB historyControlTable
 */
#define HISTORYCONTROLINDEX                     1
#define HISTORYCONTROLDATASOURCE                2
#define HISTORYCONTROLBUCKETSREQUESTED          3
#define HISTORYCONTROLBUCKETSGRANTED            4
#define HISTORYCONTROLINTERVAL                  5
#define HISTORYCONTROLOWNER                     6
#define HISTORYCONTROLSTATUS                    7


/*
 * RMON MIB etherHistoryTable
 */
#define ETHERHISTORYINDEX                       1
#define ETHERHISTORYSAMPLEINDEX                 2
#define ETHERHISTORYINTERVALSTART               3
#define ETHERHISTORYDROPEVENTS                  4
#define ETHERHISTORYOCTETS                      5
#define ETHERHISTORYPKTS                        6
#define ETHERHISTORYBROADCASTPKTS               7
#define ETHERHISTORYMULTICASTPKTS               8
#define ETHERHISTORYCRCALIGNERRORS              9
#define ETHERHISTORYUNDERSIZEPKTS               10
#define ETHERHISTORYOVERSIZEPKTS                11
#define ETHERHISTORYFRAGMENTS                   12
#define ETHERHISTORYJABBERS                     13
#define ETHERHISTORYCOLLISIONS                  14
#define ETHERHISTORYUTILIZATION                 15
#define RISING_ALARM                            1
#define FALLING_ALARM                           2
#define RISING_OR_FALLING_ALARM                 3

/*
 * RMON MIB alarmTable
 */
#define ALARMINDEX                              1
#define ALARMINTERVAL                           2
#define ALARMVARIABLE                           3
#define ALARMSAMPLETYPE                         4
#define ALARMVALUE                              5
#define ALARMSTARTUPALARM                       6
#define ALARMRISINGTHRESHOLD                    7
#define ALARMFALLINGTHRESHOLD                   8
#define ALARMRISINGEVENTINDEX                   9
#define ALARMFALLINGEVENTINDEX                  10
#define ALARMOWNER                              11
#define ALARMSTATUS                             12


/*
 * RMON MIB eventTable
 */
#define EVENTINDEX                              1
#define EVENTDESCRIPTION                        2
#define EVENTTYPE                               3
#define EVENTCOMMUNITY                          4
#define EVENTLASTTIMESENT                       5
#define EVENTOWNER                              6
#define EVENTSTATUS                             7

#define RMON_NO_ALARM                           2
#define RMON_RISE_ALARM                         1
#define RMON_FALL_ALARM                         0

/* rmonExtensionTable */
#define RMONEXTENSIONIFINDEX                    1
#define RMONEXTENSIONCLRCOUNTER                 2

/* SNMP value hack. */
#define INTEGER         ASN_INTEGER
#define INTEGER32       ASN_INTEGER
#define ENTRYSTATUS     ASN_INTEGER
#define INTERFACEINDEX  ASN_INTEGER
#define TRUTHVALUE      ASN_INTEGER
#define COUNTER32       ASN_COUNTER
#define GAUGE32         ASN_UNSIGNED
#define IPADDRESS       ASN_IPADDRESS
#define OCTET_STRING    ASN_OCTET_STR
#define SNMPADMINSTRING ASN_OCTET_STR
#define OWNERSTRING     ASN_OCTET_STR
#define DISPLAYSTRING   ASN_OCTET_STR
#define OBJECT_ID       ASN_OBJECT_ID
#define BITS            ASN_BIT_STR
#define TIMETICKS       ASN_TIMETICKS

#define NOTACCESSIBLE   NOACCESS 
#define RCREATE         3

#define EMPTY_STRING                  ""
#define ETHER_STATS_OID_SIZE          10

#define RMON_ETHER_STATS_DATASOURCE      (1 << 0)
#define RMON_ETHER_STATS_OWNER           (1 << 1)
#define RMON_ETHERSTAT_DATASOURCE_MAXSIZE 11

#define RMON_HISTORY_CONTROL_DATASOURCE  (1 << 0)
#define RMON_HISTORY_CONTROL_BUCKETS_REQ (1 << 1)
#define RMON_HISTORY_CONTROL_INTERVAL    (1 << 2)
#define RMON_HISTORY_CONTROL_OWNER       (1 << 3)
#define RMON_HISTORY_CONTROL_MAXSIZE      11

#define RMON_EVENT_CONF_DESCRIPTION      (1 << 0)
#define RMON_EVENT_CONF_OWNER            (1 << 1)
#define RMON_EVENT_CONF_TYPE             (1 << 2)
#define RMON_EVENT_CONF_COMM             (1 << 3)

#define RMON_ALARM_INTERVAL              (1 << 0)
#define RMON_ALARM_VARIABLE              (1 << 1)
#define RMON_ALARM_SAMPLETYPE            (1 << 2)
#define RMON_ALARM_STARTUP_ALARM         (1 << 3)
#define RMON_ALARM_RISING_THRESHOLD      (1 << 4)
#define RMON_ALARM_FALLING_THRESHOLD     (1 << 5)
#define RMON_ALARM_RISING_EVENT_IDX      (1 << 6)
#define RMON_ALARM_FALLING_EVENT_IDX     (1 << 7)
#define RMON_ALARM_OWNER                 (1 << 8)
#define RMON_ALARM_VARIABLE_MAXSIZE       12


uint32 rmon_debug_is_on(uint32 flag);

#define RMON_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (rmon_debug_is_on(flag)) \
        log_sys(M_MOD_RMON, E_DEBUG, fmt, ##args); \
} while (0)


void
rmon_alarm_timer_handler(void *p_data);

void
rmon_event_under_creation_timer(void *p_data);

void
rmon_alarm_under_creation_timer(void *p_data);

void
rmon_ether_under_creation_timer(void *p_data);

void
rmon_history_under_creation_timer(void *p_data);

int32 
rmon_store_sample_stats(tbl_rmon_history_t *p_db_his);

void
rmon_coll_history_timer_handler(void *p_data);

void 
rmon_oid_to_string(uint32 *oidval, char *variable);

int32 
rmon_api_update_ether_stats(tbl_interface_t *p_db_if, tbl_port_stats_t *p_port_stats, uint32 field_id);

uint32
rmon_event_get_count();

uint32
rmon_alarm_get_count();

int32
rmon_snmp_set_event_status (tbl_rmon_event_t *p_db_event, uint32 status);

int32
rmon_event_set_active(tbl_rmon_event_t *p_db_event);

int32
rmon_set_alarm_variable (tbl_rmon_alarm_t *p_alarm, oid_t *name);

int32
rmon_set_alarm_variableword(tbl_rmon_alarm_t *p_alarm, char *Variable);

int32
rmon_get_ether_stats_var(char *Variable, oid_t *ether_stats_var);

int32
rmon_api_alarm_add(tbl_rmon_alarm_t *p_alarm, char *Variable, cfg_cmd_para_t *para, uint32 alarmStartupAlarm);

int32
rmon_alarm_set_active(tbl_rmon_alarm_t *p_db_alarm);

int32
rmon_alarm_set_inactive(tbl_rmon_alarm_t *p_db_alarm);

int32
rmon_api_stats_check_if(uint32 ifindex);

int32
rmon_snmp_set_ether_stats_status (tbl_rmon_ether_stats_t *p_db_stats, uint32 status);

int32
rmon_api_ether_stats_add(tbl_interface_t *p_db_if, tbl_rmon_ether_stats_t *p_stats);

int32
rmon_api_history_check_if(uint32 ifindex);

int32
rmon_history_set_active(tbl_rmon_history_t *p_db_history);

int32
rmon_history_set_inactive(tbl_rmon_history_t *p_db_history);

int32
rmon_api_history_add(tbl_interface_t *p_db_if, tbl_rmon_history_t *p_history);

int32
rmon_cmd_process_event(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_cmd_process_alarm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_cmd_process_stats(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_cmd_process_history(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_show_event(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_event_t *p_db_event);

int32
rmon_cmd_show_event(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_show_alarm(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_alarm_t *p_db_alarm);

int32
rmon_cmd_show_alarm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_show_stats(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_ether_stats_t *p_db_stats);

int32
rmon_cmd_show_stats(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rmon_show_history(cfg_cmd_para_t *para, FILE *fp, tbl_rmon_history_t *p_db_his);

int32
rmon_cmd_show_history(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_rmon_start();


#endif

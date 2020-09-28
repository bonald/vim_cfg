#ifndef __LIB_SNMP_H__
#define __LIB_SNMP_H__
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"

#ifndef ASN_INTEGER
#define ASN_INTEGER             0x02
#define ASN_OCTET_STR           0x04
#define ASN_OBJECT_ID           0x06
#define ASN_APPLICATION         0x40
#define ASN_IPADDRESS           (ASN_APPLICATION | 0)
#define ASN_COUNTER             (ASN_APPLICATION | 1)
#define ASN_UNSIGNED            (ASN_APPLICATION | 2)
#define ASN_TIMETICKS           (ASN_APPLICATION | 3)
#define ASN_COUNTER64           (ASN_APPLICATION | 6)
#endif

/* Macros. */
#define OID_COPY(PTR,OID,LEN) \
do {                                \
  sal_memcpy ((PTR), (OID), (LEN)*sizeof(oid_t));   \
  (PTR) += (LEN);                   \
} while (0)

#define OID_SET_IP_ADDR(PTR,ADDR)   \
do {                                                        \
    u_int8_t *_pnt_ = (u_int8_t *)(ADDR);                   \
    int _i_;                                                \
    for (i = 0; i < sizeof (struct sal_in4_addr); i++) {    \
        PTR[i] = *pnt++;                                    \
    }                                                       \
    (PTR) += sizeof (struct sal_in4_addr);                  \
} while (0)

#ifdef HAVE_IPV6
#define OID_SET_IP_ADDR6(PTR,ADDR)  \
do {                                                        \
    u_int8_t *_pnt_ = (u_int8_t *)(ADDR);                   \
    int _i_;                                                \
    for (i = 0; i < sizeof (struct sal_in6_addr); i++) {    \
        PTR[i] = *pnt++;                                    \
    }                                                       \
    (PTR) += sizeof (struct sal_in6_addr);                  \
} while (0)
#endif /* HAVE_IPV6 */

#define OID_SET_ARG1(PTR,ARG1)  \
do {                    \
    *(PTR)++ = (ARG1);  \
} while (0)

#define OID_SET_ARG2(PTR,ARG1,ARG2)     \
do {                    \
    *(PTR)++ = (ARG1);  \
    *(PTR)++ = (ARG2);  \
} while (0)

#define OID_SET_ARG3(PTR,ARG1,ARG2,ARG3)    \
do {                    \
    *(PTR)++ = (ARG1);  \
    *(PTR)++ = (ARG2);  \
    *(PTR)++ = (ARG3);  \
} while (0)

#define OID_SET_ARG4(PTR,ARG1,ARG2,ARG3,ARG4)   \
do {                    \
    *(PTR)++ = (ARG1);  \
    *(PTR)++ = (ARG2);  \
    *(PTR)++ = (ARG3);  \
    *(PTR)++ = (ARG4);  \
} while (0)

#define OID_SET_ARG5(PTR,ARG1,ARG2,ARG3,ARG4,ARG5)      \
do {                    \
    *(PTR)++ = (ARG1);  \
    *(PTR)++ = (ARG2);  \
    *(PTR)++ = (ARG3);  \
    *(PTR)++ = (ARG4);  \
    *(PTR)++ = (ARG5);  \
} while (0)

#define OID_SET_ARGN(PTR,ARGC,ARGV)     \
do {                                    \
    int _i;                             \
    for (_i = 0; _i < (ARGC); _i++)     \
        *(PTR)++ = ARGV[_i];            \
} while (0)

#define OID_SET_VAL(OBJ,LEN,TYPE,VLEN,VAL)      \
do {                            \
    (OBJ).namelen = (LEN);      \
    (OBJ).val_type = (TYPE);    \
    (OBJ).val_len = (VLEN);     \
    (OBJ).val = (VAL);          \
} while (0)

#define SNMP_TRAP_EN_CHECK(_FLAG_) \
do { \
    if (0 == snmp_trap_check_enable((_FLAG_))) \
    { \
        return PM_E_NONE; \
    } \
} while (0)

typedef unsigned long oid_t;

#ifndef MAX_OID_LEN
#define MAX_OID_LEN 128
#endif

/* defines for interface admin and oper status */
#define SNMP_INTERFACE_UP       1
#define SNMP_INTERFACE_DOWN     2 
#define SNMP_INTERFACE_TESTING  3

#define PUBLICTRAPMIB           1,3,6,1,6,3,1,1,5
#define MIB2IFGROUPOID          1,3,6,1,2,1,2
#define CENTECTRAPMIB           1,3,6,1,4,1,27975
#define ENTERPRISEBASEMIB       1,3,6,1,4,1
#define RMONMIBOID              1,3,6,1,2,1,16

/*VRRP-MIB TRAP Information */
#define VRRP_TRAP_NEWMASTER           1
#define VRRP_TRAP_AUTHFAILURE         2

typedef struct
{
    size_t namelen;
    oid_t name[MAX_OID_LEN];
    unsigned char val_type;
    size_t val_len;
    void *val;
} trap_val_t;

int32
snmp_trapv2(oid_t *trap_oid, size_t trap_oid_len,
    oid_t spec_trap_val, oid_t *name, size_t namelen,
    trap_val_t *trapobj, size_t trapobjlen);
unsigned long get_snmp_enterprise_oid();

int32
snmp_calc_enterprise_oid(oid_t *enterprise_oid, uint32 eoid_len, uint32 *oid_len);

int32
snmp_trap_if_running_change(tbl_interface_t* p_db_if, bool enable);

int32
snmp_trap_sys_reboot(uint8 reboot_type);

int32
snmp_trap_write_config(uint32 success);

int32
snmp_trap_reboot_send(uint32 save);

int32
snmp_trap_memory_usage_trap(uint32 isHigher, uint32 freeSize);

int32
snmp_trap_cpu_usage_trap(uint32 isHigher, uint32 usage);

int32
snmp_trap_telnet_session_trap(uint32 start, uint32 lineIndex, char *userName, char *ipAddr);

int32
snmp_trap_time_change(char *clock, uint32 month, uint32 day, uint32 year);

int32
snmp_temp_trap(uint32 normal_or_high, uint32 fram_index, uint32 curr_temp);

int32
snmp_fan_present_trap(uint32 present, uint32 fan_num);

int32
snmp_fan_trap(uint32 normal_or_fail, uint32 fan_num);

int32 
snmp_power_trap(uint32 state, uint32 power_num);

int32
snmp_transceiver_monitor_trap(char *message , uint32 panel_port_no);

int32 
snmp_transceiver_trap(uint32 insert, uint8 panel_slot, uint8 panel_port, uint8 panel_sub_port);

int32
snmp_vrrp_failure_trap(struct sal_in4_addr* pcktscraddr, uint32 auth_failure_type);

int32
snmp_vrrp_transition_to_master_trap(struct sal_in4_addr* master_ip_addr);

int32
snmp_rmon_alarm_trap(tbl_rmon_alarm_t *p_db_alarm, uint32 alarm_type);
#endif /* !__LIB_SNMP_H__ */

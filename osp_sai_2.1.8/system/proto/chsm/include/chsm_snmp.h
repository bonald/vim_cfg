#ifndef _CHSM_SNMP_TRAP_H
#define _CHSM_SNMP_TRAP_H

#define CHSM_FRAM_INDEX 1
#define CHSM_SLOT_INDEX 1

enum chsm_snmp_power_type_e
{
    CHSM_SNMP_POWER_ABSENT     =1,
    CHSM_SNMP_POWER_FAIL,
    CHSM_SNMP_POWER_ALERT,   
    CHSM_SNMP_POWER_NO_ALERT,  
    CHSM_SNMP_POWER_NOR,
    CHSM_SNMP_POWER_FAN_FAIL
};
typedef enum chsm_snmp_power_type_e chsm_snmp_power_type_t;

#endif

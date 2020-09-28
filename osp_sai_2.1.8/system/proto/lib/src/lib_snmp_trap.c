
#include "proto.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "chsm_snmp.h"
#include "lib_snmp.h"

#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"


static oid_t mib2if_oid[] = {MIB2IFGROUPOID}; /* 1,3,6,1,2,1,2 */

uint32
snmp_trap_check_enable(uint32 trap_flag)
{
    tbl_snmp_cfg_t *p_cfg = tbl_snmp_cfg_get_snmp_cfg();

    return GLB_FLAG_ISSET(p_cfg->trap_enable, trap_flag);
}

void
_snmp_trap_if_running_change_delay(tbl_interface_t *p_db_if)
{
    trap_val_t obj[3];
    oid_t link_down_trap_oid[]  = {PUBLICTRAPMIB, 3};
    oid_t link_up_trap_oid[]    = {PUBLICTRAPMIB, 4};
    size_t trap_oid_len = 0;
    size_t namelen = 0;
    int flag_admin = 0;
    int flag_link = 0;
    bool enable = FALSE;
    oid_t *ptr = NULL;

    enable = (tbl_interface_is_running(p_db_if)) ? TRUE : FALSE;
    
    trap_oid_len = sizeof(link_down_trap_oid) / sizeof(oid_t);
    namelen = sizeof(mib2if_oid) / sizeof(oid_t);

    ptr = obj[0].name;
    OID_COPY(ptr, mib2if_oid, namelen);
    OID_SET_ARG4(ptr, 2, 1, 1, p_db_if->ifindex);

    OID_SET_VAL(obj[0], ptr - obj[0].name, ASN_INTEGER,
               sizeof(uint32), &p_db_if->ifindex);    
    
    ptr = obj[1].name;
    OID_COPY(ptr, mib2if_oid, namelen);
    OID_SET_ARG4(ptr, 2, 1 ,7, p_db_if->ifindex);
    flag_admin = (p_db_if->flags & GLB_IFF_UP) ? SNMP_INTERFACE_UP : SNMP_INTERFACE_DOWN;
    OID_SET_VAL(obj[1], ptr - obj[1].name, ASN_INTEGER,
               sizeof(int), &flag_admin);

    ptr = obj[2].name;
    OID_COPY(ptr, mib2if_oid, namelen);
    OID_SET_ARG4(ptr, 2, 1, 8, p_db_if->ifindex);
    flag_link = (p_db_if->flags & GLB_IFF_RUNNING) ? SNMP_INTERFACE_UP : SNMP_INTERFACE_DOWN;
    OID_SET_VAL(obj[2], ptr - obj[2].name, ASN_INTEGER,
               sizeof(int), &flag_link);
    
    snmp_trapv2((enable ? link_up_trap_oid : link_down_trap_oid),
                    trap_oid_len,
                    enable,
                    mib2if_oid,
                    namelen,
                    obj, 
                    3);
}

void
_snmp_trap_if_running_change_delay_cb(void* arg)
{
    tbl_interface_key_t* if_key = (tbl_interface_key_t*)(arg);
    tbl_interface_t *p_db_if = NULL;
    
    p_db_if = tbl_interface_get_interface(if_key);
    if (p_db_if)
    {
        _snmp_trap_if_running_change_delay(p_db_if);
    }
    
    XFREE(MEM_TEMP, if_key);
}

int32
snmp_trap_if_running_change(tbl_interface_t* p_db_if, bool enable)
{
    tbl_brg_global_t* p_brg_global = NULL;
    p_brg_global = tbl_brg_global_get_brg_global();
    tbl_interface_key_t* if_key = NULL;

    int32 rc = PM_E_NONE;
    
    if (enable)
    {
        SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_LINKUP);
        
        if (p_brg_global->trap_delay_up)
        {
            if_key = XCALLOC(MEM_TEMP, sizeof(tbl_interface_key_t));
            if (NULL == if_key)
            {
                return PM_E_NO_MEMORY;
            }
            sal_strncpy(if_key->name, p_db_if->key.name, IFNAME_SIZE);
            CTC_TASK_ADD_TIME(_snmp_trap_if_running_change_delay_cb, if_key, p_brg_global->trap_delay_up);    
        }
        else
        {
            _snmp_trap_if_running_change_delay(p_db_if);
        }
    }
    else
    {
        SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_LINKDOWN);

        if (p_brg_global->trap_delay_down)
        {
            if_key = XCALLOC(MEM_TEMP, sizeof(tbl_interface_key_t));
            if (NULL == if_key)
            {
                return PM_E_NO_MEMORY;
            }
            sal_strncpy(if_key->name, p_db_if->key.name, IFNAME_SIZE);
            CTC_TASK_ADD_TIME(_snmp_trap_if_running_change_delay_cb, if_key, p_brg_global->trap_delay_down);
        }
        else
        {
            _snmp_trap_if_running_change_delay(p_db_if);
        }
    }

    return rc;
}

int32
snmp_trap_sys_reboot(uint8 reboot_type)
{
    oid_t coldstart_trap_oid[] = {PUBLICTRAPMIB, 1};
    oid_t warmstart_trap_oid[] = {PUBLICTRAPMIB, 2};
    oid_t sysUpTimeOid[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
    uint32 trapoidlen;
    uint32 warmstart = FALSE;
    
    if (GLB_REBOOT_COLD == reboot_type)
    {
        SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_COLDSTART);
        warmstart = FALSE;
    }
    else if (GLB_REBOOT_WARM == reboot_type)
    {
        SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_WARMSTART);
        warmstart = TRUE;
    }
    else
    {
        log_sys(M_MOD_LIB, E_WARNING, "Hardware cannot identify reboot type, not send reboot trap");
        return PM_E_NONE;
    }

    trapoidlen = sizeof(warmstart_trap_oid) / sizeof(oid_t);

    snmp_trapv2((warmstart ? warmstart_trap_oid : coldstart_trap_oid),
              trapoidlen, 
              0,
              sysUpTimeOid, 
              sizeof(sysUpTimeOid) / sizeof(oid_t),
              NULL, 
              0);

    return PM_E_NONE;
}

int32
snmp_trap_write_config(uint32 success)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t writeSuccessTrap_oid[MAX_OID_LEN];
    oid_t writeFailureTrap_oid[MAX_OID_LEN];
    oid_t configsave_oid[] = {CENTECTRAPMIB, 1, 3, 1};
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(writeSuccessTrap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(configsave_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    writeSuccessTrap_oid[trapoidlen++] = 1;
    writeSuccessTrap_oid[trapoidlen++] = 7;
    writeSuccessTrap_oid[trapoidlen++] = 1;

    sal_memcpy(writeFailureTrap_oid, writeSuccessTrap_oid, trapoidlen * sizeof(oid_t));
    writeFailureTrap_oid[trapoidlen-1] = 2;

    configsave_oid[namelen++] = 1;
    configsave_oid[namelen++] = 3;
    configsave_oid[namelen++] = 1;

    ptr = obj[0].name;
    OID_COPY (ptr, configsave_oid, namelen);
    OID_SET_ARG1 (ptr, 0);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER,
                 sizeof(int), &success);

    snmp_trapv2 ((success ? writeSuccessTrap_oid : writeFailureTrap_oid),
              trapoidlen, 0,
              configsave_oid, namelen,
              obj, 1);

    return PM_E_NONE;
}

int32
snmp_trap_reboot_send(uint32 save)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t rebootTrap_oid[MAX_OID_LEN];
    oid_t reload_oid[MAX_OID_LEN];
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(rebootTrap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(reload_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    rebootTrap_oid[trapoidlen++] = 1;
    rebootTrap_oid[trapoidlen++] = 7;
    rebootTrap_oid[trapoidlen++] = 3;

    reload_oid[namelen++] = 1;
    reload_oid[namelen++] = 3;
    reload_oid[namelen++] = 2;

    ptr = obj[0].name;
    OID_COPY (ptr, reload_oid, namelen);
    OID_SET_ARG1 (ptr, 0);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER,
               sizeof(uint32), &save);
    
    snmp_trapv2 (rebootTrap_oid, trapoidlen, 0,
                 reload_oid, namelen,
                 obj, 1);

    return PM_E_NONE;
}

int32
snmp_trap_memory_usage_trap(uint32 isHigher, uint32 freeSize)
{
    oid_t memUsageHigher_oid[MAX_OID_LEN];
    oid_t memUsageNormal_oid[MAX_OID_LEN];
    oid_t memFreeTotal_oid[MAX_OID_LEN];
    oid_t enterprise_oid[MAX_OID_LEN];
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(memUsageHigher_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(memFreeTotal_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    memUsageHigher_oid[trapoidlen++] = 1;
    memUsageHigher_oid[trapoidlen++] = 7;
    memUsageHigher_oid[trapoidlen++] = 4;

    sal_memcpy(memUsageNormal_oid, memUsageHigher_oid, trapoidlen * sizeof(oid_t));
    memUsageNormal_oid[trapoidlen-1] = 5;

    memFreeTotal_oid[namelen++] = 1;
    memFreeTotal_oid[namelen++] = 1;
    memFreeTotal_oid[namelen++] = 11;

    ptr = obj[0].name;
    OID_COPY (ptr, memFreeTotal_oid, namelen);
    OID_SET_ARG1(ptr, 0);
    OID_SET_VAL(obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof(uint32), &freeSize);

    snmp_trapv2 ((isHigher)? memUsageHigher_oid : memUsageNormal_oid,
                 trapoidlen, 0,
                 memFreeTotal_oid, sizeof(memFreeTotal_oid)/sizeof(oid_t),
                 obj, sizeof(obj)/sizeof(trap_val_t));

    //log_sys(M_MOD_LIB, E_ERROR, "snmp_trap_memory_usage_trap() %u, %u", isHigher, freeSize);
    
    return PM_E_NONE;
}

int32
snmp_trap_cpu_usage_trap(uint32 isHigher, uint32 usage)
{
    oid_t cpuUsageHigher_oid[MAX_OID_LEN];
    oid_t cpuUsageNormal_oid[MAX_OID_LEN];
    oid_t cpuIdle_oid[MAX_OID_LEN];
    oid_t enterprise_oid[MAX_OID_LEN];
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;
    uint32 idle = 0;

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(cpuUsageHigher_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(cpuIdle_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    cpuUsageHigher_oid[trapoidlen++] = 1;
    cpuUsageHigher_oid[trapoidlen++] = 7;
    cpuUsageHigher_oid[trapoidlen++] = 9;

    sal_memcpy(cpuUsageNormal_oid, cpuUsageHigher_oid, trapoidlen * sizeof(oid_t));
    cpuUsageNormal_oid[trapoidlen-1] = 10;

    cpuIdle_oid[namelen++] = 1;
    cpuIdle_oid[namelen++] = 2;
    cpuIdle_oid[namelen++] = 11;

    idle = 100 - usage;

    ptr = obj[0].name;
    OID_COPY (ptr, cpuIdle_oid, namelen);
    OID_SET_ARG1 (ptr, 0);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof(uint32), &idle);

    snmp_trapv2 ((isHigher) ? cpuUsageHigher_oid : cpuUsageNormal_oid,
              trapoidlen, 0,
              cpuIdle_oid, sizeof(cpuIdle_oid)/sizeof(oid_t),
              obj, sizeof(obj)/sizeof(trap_val_t));

    //log_sys(M_MOD_LIB, E_ERROR, "snmp_trap_cpu_usage_trap() %u, %u", isHigher, usage);
    
    return PM_E_NONE;
}

int32
snmp_trap_telnet_session_trap(uint32 start, uint32 lineIndex, char *userName, char *ipAddr)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t sessionStartTrap_oid[MAX_OID_LEN];
    oid_t sessionStopTrap_oid[MAX_OID_LEN];
    oid_t users_oid[MAX_OID_LEN];
    trap_val_t obj[3];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(sessionStartTrap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(users_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    sessionStartTrap_oid[trapoidlen++] = 1;
    sessionStartTrap_oid[trapoidlen++] = 7;
    sessionStartTrap_oid[trapoidlen++] = 6;

    sal_memcpy(sessionStopTrap_oid, sessionStartTrap_oid, trapoidlen * sizeof(oid_t));
    sessionStopTrap_oid[trapoidlen-1] = 7;

    users_oid[namelen++] = 30;
    users_oid[namelen++] = 6;
    users_oid[namelen++] = 1;

    ptr = obj[0].name;
    OID_COPY (ptr, users_oid, namelen);
    OID_SET_ARG2(ptr, 2, lineIndex);
    OID_SET_VAL(obj[0], ptr - obj[0].name, ASN_INTEGER,
           sizeof (int), &lineIndex);

    ptr = obj[1].name;
    OID_COPY (ptr, users_oid, namelen);
    OID_SET_ARG2(ptr, 3, lineIndex);
    if (userName == NULL || sal_strlen(userName) == 0)
        OID_SET_VAL(obj[1], ptr - obj[1].name, ASN_OCTET_STR, 0 , 0);
    else
        OID_SET_VAL(obj[1], ptr - obj[1].name, ASN_OCTET_STR, sal_strlen(userName), userName);

    ptr = obj[2].name;
    OID_COPY(ptr, users_oid, namelen);
    OID_SET_ARG2 (ptr, 5, lineIndex);

    OID_SET_VAL (obj[2], ptr - obj[2].name, ASN_OCTET_STR,
           sal_strlen(ipAddr), ipAddr);

    snmp_trapv2((start ? sessionStartTrap_oid : sessionStopTrap_oid),
          trapoidlen,
          0,
          users_oid,
          sizeof(users_oid) / sizeof(oid_t),
          obj, 
          sizeof(obj) / sizeof(trap_val_t));

    return PM_E_NONE;
}

int32
snmp_trap_time_change(char *clock, uint32 month, uint32 day, uint32 year)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t sessionTimeChange_oid[MAX_OID_LEN];
    oid_t time_oid[MAX_OID_LEN];
    trap_val_t obj[4];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr = NULL;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(sessionTimeChange_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(time_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    sessionTimeChange_oid[trapoidlen++] = 1;
    sessionTimeChange_oid[trapoidlen++] = 7;
    sessionTimeChange_oid[trapoidlen++] = 8;

    time_oid[namelen++] = 1;
    time_oid[namelen++] = 8;
    time_oid[namelen++] = 1;

    ptr = obj[0].name;
    OID_COPY (ptr, time_oid, namelen);
    OID_SET_ARG2 (ptr, 1, 0);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_OCTET_STR,
           sal_strlen (clock), clock);

    ptr = obj[1].name;
    OID_COPY (ptr, time_oid, namelen);
    OID_SET_ARG2 (ptr, 2, 0);
    OID_SET_VAL (obj[1], ptr - obj[1].name, ASN_INTEGER, sizeof(uint32), &month);

    ptr = obj[2].name;
    OID_COPY (ptr, time_oid, namelen);
    OID_SET_ARG2 (ptr, 3, 0);
    OID_SET_VAL (obj[2], ptr - obj[2].name, ASN_INTEGER, sizeof(uint32), &day);

    ptr = obj[3].name;
    OID_COPY (ptr, time_oid, namelen);
    OID_SET_ARG2 (ptr, 4, 0);
    OID_SET_VAL (obj[3], ptr - obj[3].name, ASN_INTEGER, sizeof(uint32), &year);

    snmp_trapv2(sessionTimeChange_oid,
          trapoidlen, 
          0,
          time_oid, 
          sizeof(time_oid) / sizeof(oid_t),
          obj, 
          sizeof(obj) / sizeof(trap_val_t));

    return PM_E_NONE;
}

/*temperature high or normal trap*/
int32
snmp_temp_trap(uint32 normal_or_high, uint32 fram_index, uint32 curr_temp)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t temp_high_trap_oid[MAX_OID_LEN];
    oid_t temp_nor_trap_oid[MAX_OID_LEN];
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    trap_val_t obj[3];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    uint32 slot_index = 1;  /*we only support 1 now*/
    oid_t *ptr;
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);
    
    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(temp_high_trap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    temp_high_trap_oid[trapoidlen++] = 39;
    temp_high_trap_oid[trapoidlen++] = 1;
    temp_high_trap_oid[trapoidlen++] = 13;

    sal_memcpy(temp_nor_trap_oid, temp_high_trap_oid, trapoidlen * sizeof(oid_t));
    temp_nor_trap_oid[trapoidlen-1] = 14;
        
    chsm_devm_oid_new[namelen++] = 37;

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG4 (ptr, 1, 5, 1, 1);

    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof (uint32), &fram_index);

    ptr = obj[1].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG4 (ptr, 1, 5, 1, 2);

    OID_SET_VAL (obj[1], ptr - obj[1].name, ASN_INTEGER, sizeof (uint32), &slot_index);

    ptr = obj[2].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG3 (ptr, 4, 1, 2);

    OID_SET_VAL (obj[2], ptr - obj[2].name, ASN_INTEGER, sizeof (uint32), &curr_temp);

    snmp_trapv2(normal_or_high ? temp_nor_trap_oid : temp_high_trap_oid,
                trapoidlen, normal_or_high,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj,sizeof(obj) / sizeof(trap_val_t));
    
    return PM_E_NONE;
}

/*fan present or absent trap*/
int32
snmp_fan_present_trap(uint32 present, uint32 fan_num)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t fan_present_oid[MAX_OID_LEN];
    oid_t fan_absent_oid[MAX_OID_LEN];
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);
    
    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(fan_present_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    fan_present_oid[trapoidlen++] = 39;
    fan_present_oid[trapoidlen++] = 1;
    fan_present_oid[trapoidlen++] = 26;

    sal_memcpy(fan_absent_oid, fan_present_oid, trapoidlen * sizeof(oid_t));
    fan_absent_oid[trapoidlen-1] = 25;
        
    chsm_devm_oid_new[namelen++] = 37;

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG3 (ptr, 2, 1, 1);

    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof (uint32), &fan_num);

    snmp_trapv2(present ? fan_present_oid : fan_absent_oid,
                trapoidlen, present,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj, sizeof(obj) / sizeof(trap_val_t));
    
    return PM_E_NONE;
}

/*fan fail or normal trap*/
int32
snmp_fan_trap(uint32 normal_or_fail, uint32 fan_num)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t fan_nor_trap_oid[MAX_OID_LEN];
    oid_t fan_fail_trap_oid[MAX_OID_LEN];
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    trap_val_t obj[1]; 
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(fan_nor_trap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    fan_nor_trap_oid[trapoidlen++] = 39;
    fan_nor_trap_oid[trapoidlen++] = 1;
    fan_nor_trap_oid[trapoidlen++] = 6;

    sal_memcpy(fan_fail_trap_oid, fan_nor_trap_oid, trapoidlen * sizeof(oid_t));
    fan_fail_trap_oid[trapoidlen-1] = 5;
        
    chsm_devm_oid_new[namelen++] = 37;

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG3 (ptr, 2, 1, 1);

    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof(uint32), &fan_num);

    snmp_trapv2(normal_or_fail ? fan_nor_trap_oid : fan_fail_trap_oid,
                trapoidlen, normal_or_fail,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj, sizeof(obj) / sizeof(trap_val_t));
    
    return PM_E_NONE;
}

int32 
snmp_power_trap(uint32 state, uint32 power_num)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t power_absent_trap_oid[MAX_OID_LEN];
    oid_t power_fail_trap_oid[MAX_OID_LEN];
    oid_t power_nor_trap_oid[MAX_OID_LEN];
    oid_t power_fan_fail_trap_oid[MAX_OID_LEN];
    oid_t power_fan_alert_trap_oid[MAX_OID_LEN];
    oid_t power_fan_no_alert_trap_oid[MAX_OID_LEN];    
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    oid_t *oid_p = NULL;
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr;
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(power_absent_trap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    power_absent_trap_oid[trapoidlen++] = 39;
    power_absent_trap_oid[trapoidlen++] = 1;
    power_absent_trap_oid[trapoidlen++] = 1;

    sal_memcpy(power_fail_trap_oid, power_absent_trap_oid, trapoidlen * sizeof(oid_t));
    power_fail_trap_oid[trapoidlen-1] = 2;

    sal_memcpy(power_nor_trap_oid, power_absent_trap_oid, trapoidlen * sizeof(oid_t));
    power_nor_trap_oid[trapoidlen-1] = 3;

    sal_memcpy(power_fan_fail_trap_oid, power_absent_trap_oid, trapoidlen * sizeof(oid_t));
    power_fan_fail_trap_oid[trapoidlen-1] = 4;
        
    sal_memcpy(power_fan_alert_trap_oid, power_absent_trap_oid, trapoidlen * sizeof(oid_t));
    power_fan_alert_trap_oid[trapoidlen-1] = 27;

    sal_memcpy(power_fan_no_alert_trap_oid, power_absent_trap_oid, trapoidlen * sizeof(oid_t));
    power_fan_no_alert_trap_oid[trapoidlen-1] = 28;

    chsm_devm_oid_new[namelen++] = 37;
    chsm_devm_oid_new[namelen++] = 1;
    
    if(CHSM_SNMP_POWER_ABSENT == state)
    {
        oid_p = power_absent_trap_oid;
    }
    else if(CHSM_SNMP_POWER_FAIL == state)
    {
        oid_p = power_fail_trap_oid;
    }
    else if(CHSM_SNMP_POWER_NOR == state)
    {
        oid_p = power_nor_trap_oid;
    }
    else if(CHSM_SNMP_POWER_ALERT == state)
    {
        oid_p = power_fan_alert_trap_oid;
    }
    else if(CHSM_SNMP_POWER_NO_ALERT == state)
    {
        oid_p = power_fan_no_alert_trap_oid;
    }
    else
    {
        oid_p = power_fan_fail_trap_oid;
    }

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG3 (ptr, 2, 1, 1);

    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_INTEGER, sizeof (uint32), &power_num);

    snmp_trapv2(oid_p, trapoidlen, state,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj, sizeof(obj) / sizeof(trap_val_t));
    
    return PM_E_NONE;
}

int32
snmp_transceiver_monitor_trap(char *message , uint32 panel_port_no)
{
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t transceiver_monitor_trap_oid[MAX_OID_LEN];
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr;
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(transceiver_monitor_trap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    transceiver_monitor_trap_oid[trapoidlen++] = 39;
    transceiver_monitor_trap_oid[trapoidlen++] = 1;
    transceiver_monitor_trap_oid[trapoidlen++] = 20;
        
    chsm_devm_oid_new[namelen++] = 37;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 10;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 12;

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG1 (ptr, panel_port_no);

    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_OCTET_STR, sal_strlen(message), message);

    snmp_trapv2(transceiver_monitor_trap_oid,
                trapoidlen, 1,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj, sizeof(obj) / sizeof(trap_val_t));
    
    return PM_E_NONE;
}

int32 
snmp_transceiver_trap(uint32 insert, uint8 panel_slot, uint8 panel_port, uint8 panel_sub_port)
{
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t sfp_insert_trap_oid[MAX_OID_LEN];
    oid_t sfp_remove_trap_oid[MAX_OID_LEN];
    oid_t chsm_devm_oid_new[MAX_OID_LEN];
    char message[255] = "\0";
    trap_val_t obj[1];
    uint32 trapoidlen = 0;
    uint32 namelen = 0;
    oid_t *ptr;

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_SYSTEM);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(sfp_insert_trap_oid, enterprise_oid, trapoidlen * sizeof(oid_t));
    sal_memcpy(chsm_devm_oid_new, enterprise_oid, trapoidlen * sizeof(oid_t));
    namelen = trapoidlen;

    sfp_insert_trap_oid[trapoidlen++] = 39;
    sfp_insert_trap_oid[trapoidlen++] = 1;
    sfp_insert_trap_oid[trapoidlen++] = 18;
        
    sal_memcpy(sfp_remove_trap_oid, sfp_insert_trap_oid, trapoidlen * sizeof(oid_t));
    sfp_remove_trap_oid[trapoidlen-1] = 19;
    
    chsm_devm_oid_new[namelen++] = 37;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 10;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 1;
    chsm_devm_oid_new[namelen++] = 11;
    
    if_build_port_shortname_by_slot_port(ifname, panel_slot, panel_port, panel_sub_port);
    if (insert)
    {
        sal_sprintf(message, "Transceiver %s is inserted!", ifname);
    }
    else
    {
        sal_sprintf(message, "Transceiver %s is removed!", ifname);
    }

    ptr = obj[0].name;
    OID_COPY (ptr, chsm_devm_oid_new, namelen);
    OID_SET_ARG1 (ptr, panel_port);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_OCTET_STR, sal_strlen(message), message);

    snmp_trapv2(insert? sfp_insert_trap_oid : sfp_remove_trap_oid,
                trapoidlen, insert,
                chsm_devm_oid_new, sizeof chsm_devm_oid_new / sizeof(oid_t),
                obj, sizeof(obj) / sizeof(trap_val_t));
    
    
    return PM_E_NONE;
}

/* vrrp failure trap*/
int32
snmp_vrrp_failure_trap(struct sal_in4_addr* pcktscraddr, uint32 auth_failure_type)
{
    oid_t      vrrp_oid[] = {1,3,6,1,2,1,68};
    oid_t      vrrp_trap_oid[] = {1,3,6,1,2,1,68,0,2};
    uint32     namelen;
    oid_t      *ptr = NULL;
    trap_val_t obj[2];
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_VRRP);

    namelen = sizeof(vrrp_oid) / sizeof (oid_t); 

    /*Get vrrpTrapPacketSrc */
    ptr = obj[0].name;
    OID_COPY (ptr, vrrp_oid, namelen);
    OID_SET_ARG2 (ptr, 1, 5);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_IPADDRESS,
             sizeof(struct sal_in4_addr), pcktscraddr);
  
    /*Get vrrpTrapAuthErrorType */
    ptr = obj[1].name;
    OID_COPY (ptr, vrrp_oid, namelen);
    OID_SET_ARG2 (ptr, 1, 6);
    OID_SET_VAL (obj[1], ptr - obj[1].name, ASN_INTEGER,
             sizeof (uint32), &auth_failure_type);

    snmp_trapv2 (vrrp_trap_oid, sizeof(vrrp_trap_oid) / sizeof(oid_t),
             VRRP_TRAP_AUTHFAILURE,
             vrrp_oid, sizeof(vrrp_oid) / sizeof(oid_t),
             obj, sizeof(obj) / sizeof(trap_val_t));

    return PM_E_NONE;
}

int32
snmp_vrrp_transition_to_master_trap(struct sal_in4_addr* master_ip_addr)
{
    oid_t      vrrp_oid[] = {1,3,6,1,2,1,68};
    oid_t      vrrp_trap_oid[] = {1,3,6,1,2,1,68,0,1};
    uint32     namelen;
    oid_t      *ptr = NULL;
    trap_val_t obj[1];
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_VRRP);

    namelen = sizeof(vrrp_oid) / sizeof (oid_t); 

    /*Get vrrp master ip address */
    ptr = obj[0].name;
    OID_COPY (ptr, vrrp_oid, namelen);
    OID_SET_ARG4 (ptr, 1, 3, 1, 7);
    OID_SET_VAL (obj[0], ptr - obj[0].name, ASN_IPADDRESS,
             sizeof(struct sal_in4_addr), master_ip_addr);
  
    snmp_trapv2 (vrrp_trap_oid, sizeof(vrrp_trap_oid) / sizeof(oid_t),
             VRRP_TRAP_NEWMASTER,
             vrrp_oid, sizeof(vrrp_oid) / sizeof(oid_t),
             obj, sizeof(obj) / sizeof(trap_val_t));

    return PM_E_NONE;
}


/*for ptp trap*/
enum ptp_snmp_state_e
{
    PTP_SNMP_STATE_NORMAL,
    PTP_SNMP_STATE_INITIALIZING,
    PTP_SNMP_STATE_FAULTY,
    PTP_SNMP_STATE_DISABLED,
    PTP_SNMP_STATE_LISTENING,
    PTP_SNMP_STATE_PRE_MASTER,
    PTP_SNMP_STATE_MASTER,
    PTP_SNMP_STATE_PASSIVE,
    PTP_SNMP_STATE_UNCALIBRATED,
    PTP_SNMP_STATE_SLAVE,
    PTP_SNMP_STATE_UNKNOWN
};
typedef enum ptp_snmp_state_e ptp_snmp_state_t;

int32 _ptp_snmp_map_ptp_state(int32 port_state)
{
    switch (port_state)
    {
    case PTP_STATE_INITIALIZING:
        return PTP_SNMP_STATE_INITIALIZING;
    case PTP_STATE_FAULTY:
        return PTP_SNMP_STATE_FAULTY;
    case PTP_STATE_DISABLED:
        return PTP_SNMP_STATE_DISABLED;
    case PTP_STATE_LISTENING:
        return PTP_SNMP_STATE_LISTENING;
    case PTP_STATE_PRE_MASTER:
        return PTP_SNMP_STATE_PRE_MASTER;
    case PTP_STATE_MASTER:
        return PTP_SNMP_STATE_MASTER;
    case PTP_STATE_PASSIVE:
        return PTP_SNMP_STATE_PASSIVE;
    case PTP_STATE_UNCALIBRATED:
        return PTP_SNMP_STATE_UNCALIBRATED;
    case PTP_STATE_SLAVE:
        return PTP_SNMP_STATE_SLAVE;

    default:
        return PTP_SNMP_STATE_UNKNOWN;
    }
}

int32
ptp_snmp_master_change_trap(tbl_ptp_foreign_t* p_best)
{
    oid_t ptp_trap_oid[MAX_OID_LEN];
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t ptp_event_oid[MAX_OID_LEN];
    trap_val_t obj;
    uint32 trapoidlen;
    int32 namelen;
    oid_t *ptr;
    u_int32_t if_idx = 0;
    u_int32_t node_idx = 0;
    char port_identity[32];
    char ifname_ext[IFNAME_SIZE];

    tbl_interface_t *p_db_if = NULL;

    if (!p_best->p_port_belong_to)
    {
        return PM_E_FAIL;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_best->p_port_belong_to->ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if_idx = p_best->p_port_belong_to->ifindex;
    sal_snprintf(port_identity, sizeof(port_identity), "%s", ptp_desc_clockid(p_best->key.foreign_master_port_identity.clock_id));

    //if (!g_p_master->masterChangeTrap)
    //{
    //    return;
    //}
    
    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_PTP_MASTER_CHANGE);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy(ptp_trap_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    sal_memcpy(ptp_event_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    namelen = trapoidlen;

    ptp_trap_oid[trapoidlen++] = 85;
    ptp_trap_oid[trapoidlen++] = 100;
    ptp_trap_oid[trapoidlen++] = 1;

    ptp_event_oid[namelen++] = 85;
    ptp_event_oid[namelen++] = 3;
    ptp_event_oid[namelen++] = 1;

    ptr = obj.name;
    OID_COPY (ptr, ptp_event_oid, namelen);
    OID_SET_ARG3 (ptr, 4, if_idx, node_idx);
    OID_SET_VAL (obj, ptr - obj.name, ASN_OCTET_STR, sal_strlen(port_identity), port_identity);

    snmp_trapv2 (ptp_trap_oid, trapoidlen, 0,
                ptp_event_oid, sizeof ptp_event_oid / sizeof (oid_t),
                &obj, sizeof obj / sizeof (trap_val_t));

    return PM_E_NONE;
}

int32
ptp_snmp_state_change_trap(tbl_ptp_port_t* p_port)
{
    oid_t ptp_trap_oid[MAX_OID_LEN];
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t ptp_event_oid[MAX_OID_LEN];
    trap_val_t obj;
    uint32 trapoidlen;
    int32 namelen;
    oid_t *ptr;
    u_int32_t if_idx = 0;
    int32 state;
    char ifname_ext[IFNAME_SIZE];

    tbl_interface_t *p_db_if = NULL;

    if (!p_port)
    {
        return PM_E_FAIL;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(p_port->ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    if_idx = p_port->ifindex;
    state = _ptp_snmp_map_ptp_state(p_port->recommended_state);

    //if (!g_p_master->stateChangeTrap)
    //{
    //    return;
    //}

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_PTP_STATE_CHANGE);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy (ptp_trap_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    sal_memcpy (ptp_event_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    namelen = trapoidlen;

    ptp_trap_oid[trapoidlen++] = 85;
    ptp_trap_oid[trapoidlen++] = 100;
    ptp_trap_oid[trapoidlen++] = 2;

    ptp_event_oid[namelen++] = 85;
    ptp_event_oid[namelen++] = 2;
    ptp_event_oid[namelen++] = 1;

    ptr = obj.name;
    OID_COPY (ptr, ptp_event_oid, namelen);
    OID_SET_ARG2 (ptr, 15, if_idx);
    OID_SET_VAL (obj, ptr - obj.name, ASN_INTEGER, sizeof (int32), &state);

    snmp_trapv2 (ptp_trap_oid, trapoidlen, 0,
                ptp_event_oid, sizeof ptp_event_oid / sizeof (oid_t),
                &obj, sizeof obj / sizeof (trap_val_t));

    return PM_E_NONE;
}

int32
ptp_snmp_accuracy_change_trap(uint32 accuracy)
{
    oid_t ptp_trap_oid[MAX_OID_LEN];
    oid_t enterprise_oid[MAX_OID_LEN];
    oid_t ptp_event_oid[MAX_OID_LEN];
    trap_val_t obj;
    uint32 trapoidlen;
    int32 namelen;
    oid_t *ptr;

    //if (!g_p_master->accuracyChange)
    //{
    //    return;
    //}

    SNMP_TRAP_EN_CHECK(GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE);

    snmp_calc_enterprise_oid(enterprise_oid, MAX_OID_LEN, &trapoidlen);
    sal_memcpy (ptp_trap_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    sal_memcpy (ptp_event_oid, enterprise_oid, trapoidlen*sizeof (oid_t));
    namelen = trapoidlen;

    ptp_trap_oid[trapoidlen++] = 85;
    ptp_trap_oid[trapoidlen++] = 100;
    ptp_trap_oid[trapoidlen++] = 3;

    ptp_event_oid[namelen++] = 85;
    ptp_event_oid[namelen++] = 1;
    ptp_event_oid[namelen++] = 13;

    ptr = obj.name;
    OID_COPY (ptr, ptp_event_oid, namelen);
    OID_SET_ARG1 (ptr, 0);
    OID_SET_VAL (obj, ptr - obj.name, ASN_INTEGER, sizeof (int32), &accuracy);

    snmp_trapv2 (ptp_trap_oid, trapoidlen, 0,
                ptp_event_oid, sizeof ptp_event_oid / sizeof (oid_t),
                &obj, sizeof obj / sizeof (trap_val_t));

    return PM_E_NONE;
}

int32
snmp_rmon_alarm_trap(tbl_rmon_alarm_t *p_db_alarm, uint32 alarm_type)
{
    oid_t  rmon_alarm_oid[] = {RMONMIBOID};
    size_t trap_oid_len = 9;
    oid_t  rmon_alarm_trap_oid[9] = {RMONMIBOID, 0, 1};
    char   alarm_variable[RMON_ALARM_VAR_WORD_LENGTH];
    size_t namelen = 0;
    int32  alarm_value = 0;

    oid_t *ptr = NULL;
    trap_val_t obj[5];

    namelen = sizeof(rmon_alarm_oid) / sizeof(oid_t);

    ptr = obj[0].name;
    OID_COPY(ptr, rmon_alarm_oid, namelen);
    OID_SET_ARG4(ptr, 3, 1, 1, 1);

    OID_SET_VAL(obj[0], ptr - obj[0].name, ASN_INTEGER,
               sizeof(uint32), &p_db_alarm->key.index);    

    ptr = obj[1].name;
    OID_COPY(ptr, rmon_alarm_oid, namelen);
    OID_SET_ARG4(ptr, 3, 1, 1, 3);
    uint32 i = 0;
    char szNumber[32];
    for (i = 0; i < RMON_ALARM_VARIABLE_MAXSIZE; ++i)
    {
        sal_snprintf(szNumber, 32, "%d", p_db_alarm->variable[i]); 
        sal_strcat(alarm_variable, szNumber);
        if (i != RMON_ALARM_VARIABLE_MAXSIZE - 1)
        {
            sal_strcat(alarm_variable, ".");
        }
    }
    OID_SET_VAL(obj[1], ptr - obj[1].name, ASN_OBJECT_ID,
               sal_strlen(alarm_variable), alarm_variable);

    ptr = obj[2].name;
    OID_COPY(ptr, rmon_alarm_oid, namelen);
    OID_SET_ARG4(ptr, 3, 1, 1, 4);
    OID_SET_VAL(obj[2], ptr - obj[2].name, ASN_INTEGER,
               sizeof(uint32), &p_db_alarm->sample_type);

    ptr = obj[3].name;
    OID_COPY(ptr, rmon_alarm_oid, namelen);
    OID_SET_ARG4(ptr, 3, 1, 1, 5);
    alarm_value = p_db_alarm->value;
    OID_SET_VAL(obj[3], ptr - obj[3].name, ASN_INTEGER,
               sizeof(int32), &alarm_value);

    if (alarm_type == RMON_RISE_ALARM)
    {
        ptr = obj[4].name;
        OID_COPY(ptr, rmon_alarm_oid, namelen);
        OID_SET_ARG4(ptr, 3, 1, 1, 7);
        OID_SET_VAL(obj[4], ptr - obj[4].name, ASN_INTEGER,
                   sizeof(uint32), &p_db_alarm->rising_threshold);
    }
    else
    {
        ptr = obj[4].name;
        OID_COPY(ptr, rmon_alarm_oid, namelen);
        OID_SET_ARG4(ptr, 3, 1, 1, 8);
        OID_SET_VAL(obj[4], ptr - obj[4].name, ASN_INTEGER,
                   sizeof(uint32), &p_db_alarm->falling_threshold);
        rmon_alarm_trap_oid[8] = 2;
    }

    snmp_trapv2(rmon_alarm_trap_oid,
                    trap_oid_len,
                    alarm_type,
                    rmon_alarm_oid,
                    namelen,
                    obj, 
                    5);

    return PM_E_NONE;
}



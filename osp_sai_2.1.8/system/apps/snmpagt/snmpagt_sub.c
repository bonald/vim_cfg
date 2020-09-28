
#include "proto.h"
#include "cdbclient.h"
#include "cdbclient_subscribe.h"
#include "snmpagt_define.h"
#include "snmpagt.h"
#include "gen/tbl_snmp_trap_msg_define.h"
#include "gen/tbl_snmp_trap_msg.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_app_debug.h"

extern int32
ifx_table_add_row(tbl_interface_t *p_if);
extern int32
ifx_table_del_row(int32 ifindex);
extern int32
if_table_add_row(tbl_interface_t *p_if);
extern int32
if_table_del_row(int32 ifindex);

extern int32
interface_table_add_row(tbl_interface_t *p_if);
extern int32
interface_table_del_row(int32 ifindex);
extern int32
ifx_table_update_row(tbl_interface_t *p_if, uint32 field_id);
extern int32
if_table_update_row(tbl_interface_t *p_if, uint32 field_id);
extern int32
interface_table_update_row(tbl_interface_t *p_if, uint32 field_id);

extern int32
ifx_table_update_row_stats(tbl_port_stats_t *p_if_stats, uint32 field_id);
extern int32
if_table_update_row_stats(tbl_port_stats_t *p_if_stats, uint32 field_id);

extern int32
version_add_row(tbl_version_t * p_version);
extern int32
version_set_row(tbl_version_t * p_version, uint32 field_id);
extern int32
card_add_row(tbl_card_t * p_card);
extern int32
card_set_row(tbl_card_t * p_card, uint32 field_id);

extern int32
priv_device_sensor_add_row(tbl_sensor_t *p_sensor);
extern int32
priv_device_sensor_set_row(tbl_sensor_t *p_sensor, uint32 field_id);
extern int32
priv_device_sensor_del_row(tbl_sensor_key_t *p_sensor_key);
extern int32
priv_device_power_add_row(tbl_psu_t *p_psu);
extern int32
priv_device_power_set_row(tbl_psu_t *p_psu, uint32 field_id);
extern int32
priv_device_power_del_row(tbl_psu_key_t *p_psu_key);
extern int32
priv_device_fan_add_row(tbl_fan_t *p_fan);
extern int32
priv_device_fan_set_row(tbl_fan_t *p_fan, uint32 field_id);
extern int32
priv_device_fan_del_row(tbl_fan_key_t *p_fan_key);
extern int32
priv_device_fiber_add_row(tbl_fiber_t *p_fiber);
extern int32
priv_device_fiber_set_row(tbl_fiber_t *p_fiber, uint32 field_id);
extern int32
priv_device_fiber_del_row(tbl_fiber_key_t *p_fiber_key);
extern int32
rmon_ether_stats_add_row(tbl_rmon_ether_stats_t * p_ether_stats);
extern int32
rmon_ether_stats_set_row(tbl_rmon_ether_stats_t *p_ether_stats, uint32 field_id);
extern int32
rmon_ether_stats_del_row(tbl_rmon_ether_stats_key_t *p_ether_stats);
extern int32
rmon_history_add_row(tbl_rmon_history_t *p_history);
extern int32
rmon_history_set_row(tbl_rmon_history_t *p_history, uint32 field_id);
extern int32
rmon_history_del_row(tbl_rmon_history_key_t *p_history);
extern int32
rmon_ether_history_add_row(tbl_rmon_stats_history_t *p_ether_history);
extern int32
rmon_ether_history_del_row(tbl_rmon_stats_history_key_t *p_ether_history);
extern int32
rmon_alarm_add_row(tbl_rmon_alarm_t *p_alarm);
extern int32
rmon_alarm_set_row(tbl_rmon_alarm_t *p_alarm, uint32 field_id);
extern int32
rmon_alarm_del_row(tbl_rmon_alarm_key_t *p_alarm);
extern int32
rmon_event_add_row(tbl_rmon_event_t *p_event);
extern int32
rmon_event_set_row(tbl_rmon_event_t *p_event, uint32 field_id);
extern int32
rmon_event_del_row(tbl_rmon_event_key_t *p_event);


static oid UPTIME_OID[]     = { PUBLICMIB_OID, 1, 3, 0 };
static oid SNMPTRAP_OID[]   = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };

int32
snmpagt_trap_get_var(netsnmp_variable_list *vars, int32 i, snmpagt_parse_info_t *info)
{
    oid item_oid[MAX_OID_LEN];
    size_t oid_len = 0;
    oid value_oid[MAX_OID_LEN];
    size_t value_oid_len = 0;
    char *item_oidstr = NULL;
    char *item_type = NULL;
    char *item_value = NULL;
    int intv = 0;
    uint32 ulongv = 0;
    u_char type = 0;
    int32 rc = 0;
    
    item_oidstr = info->argv[4 + 3*i];
    item_type = info->argv[4 + 3*i + 1];
    item_value = info->argv[4 + 3*i + 2];

    switch (item_type[0])
    {
    case 'i':
        type = ASN_INTEGER;
        intv = cdb_int_str2val(item_value, &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        break;
    case 'u':
        type = ASN_UNSIGNED;
        ulongv = cdb_uint_str2val(item_value, &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        break;
    case 't':
        type = ASN_TIMETICKS;
        ulongv = cdb_uint_str2val(item_value, &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        break;
    case 'a':
        type = ASN_IPADDRESS;
        ulongv = cdb_uint_str2val(item_value, &rc);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        break;
    case 's':
        type = ASN_OCTET_STR;
        break;
    case 'o':
        type = ASN_OBJECT_ID;
        sal_memset(value_oid, 0, sizeof(value_oid));
        value_oid_len = MAX_OID_LEN;
        rc = snmpagt_str2oid(item_value, value_oid, &value_oid_len);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        break;
    default:
        return PM_E_FAIL;
    }

    sal_memset(item_oid, 0, sizeof(item_oid));
    oid_len = MAX_OID_LEN;
    rc = snmpagt_str2oid(item_oidstr, item_oid, &oid_len);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }

    if (ASN_INTEGER == type)
    {
        snmp_varlist_add_variable(&vars, item_oid, oid_len, type, (u_char*)&intv, sizeof(intv));
    }
    else if ((ASN_UNSIGNED == type) || (ASN_TIMETICKS == type) || (ASN_IPADDRESS == type))
    {
        snmp_varlist_add_variable(&vars, item_oid, oid_len, type, (u_char*)&ulongv, sizeof(ulongv));
    }
    else if (ASN_OCTET_STR == type)
    {
        snmp_varlist_add_variable(&vars, item_oid, oid_len, type, (u_char*)item_value, sal_strlen(item_value));
    }
    else if (ASN_OBJECT_ID == type)
    {
        snmp_varlist_add_variable(&vars, item_oid, oid_len, type, (u_char*)value_oid, value_oid_len*sizeof(oid));
    }
    
    return rc;
}

int32
snmpagt_trap(snmpagt_parse_info_t *info)
{
    oid trap_oid[MAX_OID_LEN];
    size_t oid_len = 0;
    netsnmp_variable_list *vars = NULL;
    uint32 uptime = 0;
    int32 rc = 0;
    int32 i = 0;
    int32 var_count = 0;

    if (info->argc < 4)
    {
        return PM_E_FAIL;
    }
    if (((info->argc - 4) % 3) != 0)
    {
        return PM_E_FAIL;
    }

    var_count = (info->argc - 4)/3;
    uptime = cdb_uint_str2val(info->argv[3], &rc);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(trap_oid, 0, sizeof(trap_oid));
    oid_len = MAX_OID_LEN;
    rc = snmpagt_str2oid(info->argv[1], trap_oid, &oid_len);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    
    // uptime
    snmp_varlist_add_variable(&vars, UPTIME_OID, OID_LENGTH(UPTIME_OID), 
                              ASN_TIMETICKS, (u_char*)&uptime, sizeof(uptime));

    // trap oid
    snmp_varlist_add_variable(&vars, SNMPTRAP_OID, OID_LENGTH(SNMPTRAP_OID), 
                              ASN_OBJECT_ID, (u_char*)trap_oid, oid_len*sizeof(oid));

    for (i = 0; i < var_count; i++)
    {
        rc = snmpagt_trap_get_var(vars, i, info);
        if (rc < 0)
        {
            goto ERROR;
        }
    }
    send_v2trap(vars);

ERROR:
    if (vars)
    {
        snmp_free_varbind(vars);
        vars = NULL;
    }
   
    return PM_E_NONE;
}

int32
snmpagt_parse_para(char *para_str, snmpagt_parse_info_t *p_info)
{
    uint32          cmdstrlen = 0;
    char           *pos = NULL;

    sal_memset(p_info, 0, sizeof(snmpagt_parse_info_t));
    if (NULL == para_str)
    {
        return PM_E_NONE;
    }

    cmdstrlen = sal_strlen(para_str);
    if (0 == cmdstrlen)
    {
        return PM_E_NONE;
    }
    if (para_str[cmdstrlen-1] == CMD_DIVISION_CHAR)
    {
        para_str[cmdstrlen-1] = 0;
    }

    pos = cfg_strchr(para_str, CMD_DIVISION_CHAR);
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_strchr(pos + 1, CMD_DIVISION_CHAR);
    }
    
    return PM_E_NONE;
}

extern unsigned int netsnmp_ctc_trap_count;

int32
snmpagt_cdb_snmp_trap_msg(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper)
{
    snmpagt_parse_info_t parse_info;
    tbl_snmp_trap_msg_t *p_msg = NULL;
    int32 rc = 0;
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_msg = (tbl_snmp_trap_msg_t*)p_tbl;
        if (snmp_get_do_logging() && netsnmp_ds_get_ctc_debug())
        {
            snmp_log(LOG_DEBUG, "[Trap] %s", p_msg->msg);
        }
        rc = snmpagt_parse_para(p_msg->msg, &parse_info);
        if (rc < 0)
        {
            return 0;
        }
        netsnmp_ctc_trap_count++;
        snmpagt_trap(&parse_info);
        break;
        
    default:
        break;
    }

    return 0;
}

int32
snmpagt_cdb_tbl_interface(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_interface_t *p_if = NULL;
    tbl_interface_key_t *p_if_key = NULL;
    int32 ifindex = 0;
    int32 hw_type = 0;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_interface_t*)p_tbl;
        if ((GLB_IF_TYPE_PORT_IF == p_if->hw_type) || (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)
          ||(GLB_IF_TYPE_VLAN_IF == p_if->hw_type) || (GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type))
        {
            ifx_table_add_row(p_if);
            if_table_add_row(p_if);
            interface_table_add_row(p_if);
            snmpagt_set_ifg_stats_en(p_if);
        }
        break;

    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        if ((GLB_IF_TYPE_PORT_IF == p_if->hw_type) || (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)
          ||(GLB_IF_TYPE_VLAN_IF == p_if->hw_type) || (GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type))
        {
            ifx_table_update_row(p_if, field_id);
            if_table_update_row(p_if, field_id);
            interface_table_update_row(p_if, field_id);
        }

        if (TBL_INTERFACE_FLD_IFG_STATS_EN == field_id)
        {
            snmpagt_set_ifg_stats_en(p_if);
        }
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        ifindex = tbl_interface_get_ifindex_by_name(p_if_key->name);
        hw_type = tbl_interface_get_type_by_ifindex(ifindex);
        if ((GLB_IF_TYPE_PORT_IF == hw_type) || (GLB_IF_TYPE_LINKAGG_IF == hw_type)
          ||(GLB_IF_TYPE_VLAN_IF == hw_type) || (GLB_IF_TYPE_LOOPBACK_IF == hw_type))
        {
            ifx_table_del_row(ifindex);
            if_table_del_row(ifindex);
            interface_table_del_row(ifindex);
        }
        break;

    default:
        break;
    }

    return 0;
}

int32
snmpagt_cdb_snmp_cfg(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_snmp_cfg_t *p_cfg = NULL;
    p_cfg = (tbl_snmp_cfg_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_ADD:
        mib2_system_update_syscontact(p_cfg->contact.info);
        mib2_system_update_syslocation(p_cfg->location.info);
        break;

    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_SNMP_CFG_FLD_CONTACT:
            return mib2_system_update_syscontact(p_cfg->contact.info);
            break;

        case TBL_SNMP_CFG_FLD_LOCATION:
            return mib2_system_update_syslocation(p_cfg->location.info);
            break;

        case TBL_SNMP_CFG_FLD_AGENT_STATS:
            return netsnmp_ds_set_ctc_stats(p_cfg->agent_stats);
            break;

        default:
            break;
        }

    default:
            break;
    }

    return 0;
}

int32
snmpagt_cdb_app_debug(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_app_debug_t *p_debug = (tbl_app_debug_t*)p_tbl;
    uint32 enable = FALSE;

    enable = (p_debug->snmp) ? TRUE : FALSE;
    switch (oper)
    {
    case CDB_OPER_ADD:
        netsnmp_ds_set_ctc_debug(enable);
        break;

    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_APP_DEBUG_FLD_SNMP_AGENT:
            return netsnmp_ds_set_ctc_debug(enable);
            break;

        default:
            break;
        }

    default:
            break;
    }

    return 0;
}

int32
snmpagt_cdb_sys_global(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_sys_global_t *p_glb = NULL;

    p_glb = (tbl_sys_global_t*)p_tbl;

    if (CDB_OPER_ADD == oper)
    {
        mib2_system_update_sysname(p_glb->hostname);
    }
    else if (CDB_OPER_SET == oper)
    {
        switch (field_id)
        {
        case TBL_SYS_GLOBAL_FLD_HOSTNAME:
            return mib2_system_update_sysname(p_glb->hostname);
            break;

        default:
            break;
        }
    }

    return 0;
}

int32
snmpagt_cdb_mem_summary(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_mem_summary_t *p_mem = NULL;

    p_mem = (tbl_mem_summary_t*)p_tbl;
    switch (field_id)
    {
    /* free is last changed */
    case TBL_MEM_SUMMARY_FLD_FREE:
        return priv_system_update_mem(p_mem->total, p_mem->used, p_mem->free);
        break;

    default:
        break;
    }

    return 0;
}

int32
snmpagt_cdb_manage_if(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_manage_if_t *p_mgmtif = NULL;
    char addr_str[CMD_BUF_32];
    
    p_mgmtif = (tbl_manage_if_t*)p_tbl;

    if (CDB_OPER_ADD == oper)
    {
        cdb_prefix_ipv4_val2str(addr_str, CMD_BUF_32, &p_mgmtif->addr);
        priv_mgmt_update_mgmtip(addr_str);
        priv_mgmt_update_mgmtgateway(p_mgmtif->gateway);
    }
    else if (CDB_OPER_SET == oper)
    {
        switch (field_id)
        {
        case TBL_MANAGE_IF_FLD_ADDR:
            sal_memset(addr_str, 0, sizeof(addr_str));
            cdb_prefix_ipv4_val2str(addr_str, CMD_BUF_32, &p_mgmtif->addr);
            return priv_mgmt_update_mgmtip(addr_str);
            break;

        case TBL_MANAGE_IF_FLD_GATEWAY:
            return priv_mgmt_update_mgmtgateway(p_mgmtif->gateway);
            break;

        default:
            break;
        }
    }

    return 0;
}

int32
test_add_row(int index, int absent)
{
    tbl_psu_t psu;

    sal_memset(&psu, 0, sizeof(psu));
    psu.key.id = index;
    psu.absent = absent;

    return priv_device_power_add_row(&psu);
}

int32
test_update_row(int index, int absent)
{
    tbl_psu_t psu;

    sal_memset(&psu, 0, sizeof(psu));
    psu.key.id = index;
    psu.absent = absent;

    return priv_device_power_set_row(&psu, TBL_PSU_FLD_ABSENT);
}

int32
test_del_row(int index)
{
    tbl_psu_key_t psu;

    sal_memset(&psu, 0, sizeof(psu));
    psu.id = index;

    return priv_device_power_del_row(&psu);
}

int32
snmpagt_cdb_sensor(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_sensor_t *p_sensor = NULL;
    tbl_sensor_key_t *p_sensor_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_sensor = (tbl_sensor_t*)p_tbl;
        return priv_device_sensor_add_row(p_sensor);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_sensor = (tbl_sensor_t*)p_tbl;
        return priv_device_sensor_set_row(p_sensor, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_sensor_key = (tbl_sensor_key_t*)p_tbl;
        return priv_device_sensor_del_row(p_sensor_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_fiber(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_fiber_t *p_fiber = NULL;
    tbl_fiber_key_t *p_fiber_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_fiber = (tbl_fiber_t*)p_tbl;
        return priv_device_fiber_add_row(p_fiber);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_fiber = (tbl_fiber_t*)p_tbl;
        return priv_device_fiber_set_row(p_fiber, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_fiber_key = (tbl_fiber_key_t*)p_tbl;
        return priv_device_fiber_del_row(p_fiber_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_psu(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_psu_t *p_psu = NULL;
    tbl_psu_key_t *p_psu_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_psu = (tbl_psu_t*)p_tbl;
        return priv_device_power_add_row(p_psu);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_psu = (tbl_psu_t*)p_tbl;
        return priv_device_power_set_row(p_psu, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_psu_key = (tbl_psu_key_t*)p_tbl;
        return priv_device_power_del_row(p_psu_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_fan(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_fan_t *p_fan = NULL;
    tbl_fan_key_t *p_fan_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_fan = (tbl_fan_t*)p_tbl;
        return priv_device_fan_add_row(p_fan);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_fan = (tbl_fan_t*)p_tbl;
        return priv_device_fan_set_row(p_fan, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_fan_key = (tbl_fan_key_t*)p_tbl;
        return priv_device_fan_del_row(p_fan_key);
    }
    
    return 0;
}


int32
snmpagt_cdb_tbl_interface_stats(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_port_stats_t *p_if_stats = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_SET:
        p_if_stats = (tbl_port_stats_t*)p_tbl;
        ifx_table_update_row_stats(p_if_stats, field_id);
        if_table_update_row_stats(p_if_stats, field_id);
        break;

    default:
        break;
    }

    return 0;
}

int32
snmpagt_cdb_rmon_ether_stats(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_rmon_ether_stats_t *p_stats = NULL;
    tbl_rmon_ether_stats_key_t *p_stats_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_stats = (tbl_rmon_ether_stats_t*)p_tbl;
        return rmon_ether_stats_add_row(p_stats);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_stats = (tbl_rmon_ether_stats_t*)p_tbl;
        return rmon_ether_stats_set_row(p_stats, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_stats_key = (tbl_rmon_ether_stats_key_t*)p_tbl;
        return rmon_ether_stats_del_row(p_stats_key);
    }

    return 0;
}

int32
snmpagt_cdb_rmon_history(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_rmon_history_t *p_history = NULL;
    tbl_rmon_history_key_t *p_history_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_history = (tbl_rmon_history_t*)p_tbl;
        return rmon_history_add_row(p_history);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_history = (tbl_rmon_history_t*)p_tbl;
        return rmon_history_set_row(p_history, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_history_key = (tbl_rmon_history_key_t*)p_tbl;
        return rmon_history_del_row(p_history_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_rmon_stats_history(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_rmon_stats_history_t *p_history = NULL;
    tbl_rmon_stats_history_key_t *p_history_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_history = (tbl_rmon_stats_history_t*)p_tbl;
        return rmon_ether_history_add_row(p_history);
    }
    else if (CDB_OPER_SET == oper)
    {
        return 0;
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_history_key = (tbl_rmon_stats_history_key_t*)p_tbl;
        return rmon_ether_history_del_row(p_history_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_rmon_alarm(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_rmon_alarm_t *p_alarm = NULL;
    tbl_rmon_alarm_key_t *p_alarm_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_alarm = (tbl_rmon_alarm_t*)p_tbl;
        return rmon_alarm_add_row(p_alarm);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_alarm = (tbl_rmon_alarm_t*)p_tbl;
        return rmon_alarm_set_row(p_alarm, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_alarm_key = (tbl_rmon_alarm_key_t*)p_tbl;
        return rmon_alarm_del_row(p_alarm_key);
    }
    
    return 0;
}

int32
snmpagt_cdb_rmon_event(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_rmon_event_t *p_event = NULL;
    tbl_rmon_event_key_t *p_event_key = NULL;
    
    if (CDB_OPER_ADD == oper)
    {
        p_event = (tbl_rmon_event_t*)p_tbl;
        return rmon_event_add_row(p_event);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_event = (tbl_rmon_event_t*)p_tbl;
        return rmon_event_set_row(p_event, field_id);
    }
    else if (CDB_OPER_DEL == oper)
    {
        p_event_key = (tbl_rmon_event_key_t*)p_tbl;
        return rmon_event_del_row(p_event_key);
    }

    return 0;
}

int32
snmpagt_cdb_version(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_version_t * p_version = NULL;

    if (CDB_OPER_ADD == oper)
    {
        p_version = (tbl_version_t *)p_tbl;
        return version_add_row(p_version);
    }
    else if (CDB_OPER_SET == oper)
    {
        p_version = (tbl_version_t *)p_tbl;
        return version_set_row(p_version, field_id);
    }
    return 0;
}

int32
snmpagt_cdb_card(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint8 *p_tbl, 
        uint8 *p_ds, cdb_oper_t oper, uint32 field_id)
{
    tbl_card_t * p_card = NULL;
    if (CDB_OPER_ADD == oper)
    {
        p_card = (tbl_card_t *)p_tbl;
        return card_add_row(p_card);
    }
    else if (CDB_OPER_SET == oper)
    {
        return card_set_row(p_card, field_id);
    }
    return 0;
}

int32
snmpagt_cdb_dispatch(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
{
    uint8 *p_tbl = NULL;
    uint8 *p_ds = NULL;
    int32 rc = PM_E_NONE;

    p_tbl = p_msg->data;
    if (p_ds_node)
    {
        p_ds = p_msg->data + p_tbl_node->tbl_info->key_size;
    }

#if 0
    if (p_ds_node)
    {
        log_sys(M_MOD_SNMPAGT, E_DEBUG, "snmpagt_sub_dispatch tbl %d ds %d oper %d", p_tbl_node->id, p_ds_node->id, p_msg->u_hdr.data.oper);
    }
    else
    {
        log_sys(M_MOD_SNMPAGT, E_DEBUG, "snmpagt_sub_dispatch tbl %d oper %d", p_tbl_node->id, p_msg->u_hdr.data.oper);
    }
#endif

    switch (p_tbl_node->id)
    {
    case TBL_SNMP_TRAP_MSG:
        snmpagt_cdb_snmp_trap_msg(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper);
        break;
        
    case TBL_INTERFACE:
        if (NULL == p_ds_node)
        {
            snmpagt_cdb_tbl_interface(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        }
        break;
        
    case TBL_PORT_STATS:
        if (NULL == p_ds_node)
        {
            snmpagt_cdb_tbl_interface_stats(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        }
        break;

    case TBL_SNMP_CFG:
        snmpagt_cdb_snmp_cfg(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_APP_DEBUG:
        snmpagt_cdb_app_debug(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_SYS_GLOBAL:
        snmpagt_cdb_sys_global(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_MEM_SUMMARY:
        snmpagt_cdb_mem_summary(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_MANAGE_IF:
        snmpagt_cdb_manage_if(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;
    
    case TBL_SENSOR:
        snmpagt_cdb_sensor(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_FIBER:
        snmpagt_cdb_fiber(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_PSU:
        snmpagt_cdb_psu(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;
        
    case TBL_FAN:
        snmpagt_cdb_fan(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_RMON_ETHER_STATS:
        snmpagt_cdb_rmon_ether_stats(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_RMON_HISTORY:
        snmpagt_cdb_rmon_history(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_RMON_STATS_HISTORY:
        snmpagt_cdb_rmon_stats_history(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_RMON_ALARM:
        snmpagt_cdb_rmon_alarm(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_RMON_EVENT:
        snmpagt_cdb_rmon_event(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    case TBL_VERSION:
        snmpagt_cdb_version(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;
    case TBL_CARD:
        snmpagt_cdb_card(p_tbl_node, p_ds_node, p_tbl, p_ds, p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields[0]);
        break;

    default:
        break;
    }

    return rc;
}

int32
snmpagt_sub_resync()
{
    return ipc_ctl_to_cds_sub_tbl_resync();
}

int32
snmpagt_sub_init(uint32 sync)
{
    int32 rc = PM_E_NONE;
    char tables[CMD_BUF_256];
    cdb_subscribe_format_t format = CDB_SUB_FORMAT_RAWSNMP;

    sal_strncpy(tables, "snmp_trap_msg,interface,snmp_cfg,sys_global,manage_if,fiber,fan,sensor,mem_summary,psu,port_stats,rmon_ether_stats,rmon_history,rmon_stats_history,rmon_alarm,rmon_event,app_debug,version,card", CMD_BUF_256);
    cdbclt_subscribe_register_user_cb(snmpagt_cdb_dispatch);

    /* 2. sync connect to CCS */
    rc = ipc_connect_ccs_subscribe(5);
    if (rc == 0)
    {
        rc = cdbclt_subscribe_init_tbl(tables, format, sync);
    }

    return PM_E_NONE;
}


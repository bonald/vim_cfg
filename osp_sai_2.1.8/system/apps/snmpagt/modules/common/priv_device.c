
#include "snmpagt_define.h"
#include "snmpagt.h"

snmpagt_row_t FIBER_ROW[] = 
{
    {TBL_FIBER_RX_PWR2,                 ASN_OCTET_STR,  0},
    {TBL_FIBER_TX_PWR2,                 ASN_OCTET_STR,  0},
    {TBL_FIBER_TX_HIGH_ALARM,           ASN_OCTET_STR,  0},
    {TBL_FIBER_RX_HIGH_ALARM,           ASN_OCTET_STR,  0},
    {TBL_FIBER_TX_HIGH_WARN,            ASN_OCTET_STR,  0},
    {TBL_FIBER_RX_HIGH_WARN,            ASN_OCTET_STR,  0},
    {TBL_FIBER_TX_LOW_ALARM,            ASN_OCTET_STR,  0},
    {TBL_FIBER_RX_LOW_ALARM,            ASN_OCTET_STR,  0},
    {TBL_FIBER_TX_LOW_WARN,             ASN_OCTET_STR,  0},
    {TBL_FIBER_RX_LOW_WARN,             ASN_OCTET_STR,  0},
};

snmpagt_row_t POWER_ROW[] = 
{
    {TBL_POWER_STATUS,                  ASN_INTEGER,    0},
    {TBL_POWER_WORK_STATUS,             ASN_INTEGER,    0},
    {TBL_POWER_TYPE,                    ASN_INTEGER,    0},
    {TBL_POWER_ALERT_STATUS,            ASN_INTEGER,    0},
};

snmpagt_row_t FAN_ROW[] = 
{
    {TBL_FAN_MODULE,                    ASN_INTEGER,    0},
    {TBL_FAN_STATUS,                    ASN_INTEGER,    0},
    {TBL_FAN_SPEED,                     ASN_INTEGER,    0},
};

snmpagt_row_t TEMPR_ROW[] = 
{
    {TBL_TEMPR_TEMPERATURE,             ASN_INTEGER,    0},
    {TBL_TEMPR_LOW,                     ASN_INTEGER,    0},
    {TBL_TEMPR_HIGH,                    ASN_INTEGER,    0},
    {TBL_TEMPR_CRITICAL,                ASN_INTEGER,    0},
    {TBL_TEMPR_LOCATION,                ASN_INTEGER,    0},
};

static snmpagt_table_t *g_p_fiber_tbl = NULL;
static snmpagt_table_t *g_p_power_tbl = NULL;
static snmpagt_table_t *g_p_fan_tbl = NULL;
static snmpagt_table_t *g_p_tempr_tbl = NULL;

static oid FIBER_OID[]  = { CTCENTERPRISE_OID, 1, 11 };
static oid POWER_OID[]  = { CTCENTERPRISE_OID, 37, 1, 2 };
static oid FAN_OID[]    = { CTCENTERPRISE_OID, 37, 2 };
static oid TEMPR_OID[]  = { CTCENTERPRISE_OID, 37, 4 };

int32
_priv_device_fan_get_module(tbl_fan_t *p_fan)
{
    return (p_fan->tray + 1);
}

int32
_priv_device_fan_get_status(tbl_fan_t *p_fan)
{
    int32 status = 0;
    
    if (0 != sal_strcmp(p_fan->status, "OK"))
    {
        status = 2;
    }
    else
    {
        status = 1;
    }
    return status;
}

int32
_priv_device_fan_get_speed(tbl_fan_t *p_fan)
{
    int32 speed = 0;
    
    if (0 != sal_strcmp(p_fan->status, "OK"))
    {
        speed = 0;
    }
    else
    {
        speed = p_fan->percent;
    }
    return speed;
}

int32
_priv_device_power_get_absent(tbl_psu_t *p_psu)
{
    int32 absent = 0;
    
    if (p_psu->absent)
    {
        absent = TBL_POWER_ABSENT_VAL;
    }
    else
    {
        absent = TBL_POWER_PRESENT_VAL;
    }

    return absent;
}
    
int32
_priv_device_power_get_type(tbl_psu_t *p_psu)
{
    int32 type = 4;
    
    if (0 == sal_strcmp(p_psu->mode, "AC"))
    {
        type = 1;
    }
    else if (0 == sal_strcmp(p_psu->mode, "DC"))
    {
        type = 2;
    }
    else if (0 == sal_strcmp(p_psu->mode, "-"))
    {
        type = 3;
    }
    return type;
}

int32
_priv_device_power_get_alert_status(tbl_psu_t *p_psu)
{
    int32 alert_status = 0;
    
    if ((0 == p_psu->absent) && (0 == sal_strcmp(p_psu->alert_status, "ALERT")))
    {
        alert_status = 2;
    }
    else if (0 == sal_strcmp(p_psu->alert_status, "NO"))
    {
        alert_status = 1;
    }
    else
    {
        alert_status = 3;
    }
    return alert_status;
}

int32
_priv_device_power_get_run_status(tbl_psu_t *p_psu)
{
    int32 run_status = 0;

    if (p_psu->absent)
    {
        run_status = 3;
    }
    else if (0 == sal_strcmp(p_psu->run_status, "FAIL"))
    {
        run_status = 2;
    }
    else if (0 == sal_strcmp(p_psu->run_status, "OK"))
    {
        run_status = 1;
    }
    else
    {
        run_status = 3;
    }
    return run_status;
}

int32
_priv_device_fiber_set_tx_pwr2(tbl_fiber_t *p_fiber, netsnmp_table_row *row)
{
    char txpwr_str[CMD_BUF_256];

    if (0 == p_fiber->channel_idx)
    {
        sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr2[0]*100));
    }
    else if (1 == p_fiber->channel_idx)
    {
        sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr2[1]*100));
    }
    else if (2 == p_fiber->channel_idx)
    {
        sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr2[2]*100));
    }
    else if (3 == p_fiber->channel_idx)
    {
        sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr2[3]*100));
    }
    else
    {
        sal_snprintf(txpwr_str, CMD_BUF_256, "%d,%d,%d,%d", 
            (int32)(p_fiber->tx_pwr2[0]*100),
            (int32)(p_fiber->tx_pwr2[1]*100),
            (int32)(p_fiber->tx_pwr2[2]*100),
            (int32)(p_fiber->tx_pwr2[3]*100));
    }
    netsnmp_set_row_column(row, TBL_FIBER_TX_PWR2,          ASN_OCTET_STR, (void*)&txpwr_str, sal_strlen(txpwr_str));

    return 0;
}

int32
_priv_device_fiber_set_rx_pwr2(tbl_fiber_t *p_fiber, netsnmp_table_row *row)
{
    char rxpwr_str[CMD_BUF_256];

    if (0 == p_fiber->channel_idx)
    {
        sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr2[0]*100));
    }
    else if (1 == p_fiber->channel_idx)
    {
        sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr2[1]*100));
    }
    else if (2 == p_fiber->channel_idx)
    {
        sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr2[2]*100));
    }
    else if (3 == p_fiber->channel_idx)
    {
        sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr2[3]*100));
    }
    else
    {
        sal_snprintf(rxpwr_str, CMD_BUF_256, "%d,%d,%d,%d", 
            (int32)(p_fiber->rx_pwr2[0]*100),
            (int32)(p_fiber->rx_pwr2[1]*100),
            (int32)(p_fiber->rx_pwr2[2]*100),
            (int32)(p_fiber->rx_pwr2[3]*100));
    }    
    netsnmp_set_row_column(row, TBL_FIBER_RX_PWR2,          ASN_OCTET_STR, (void*)&rxpwr_str, sal_strlen(rxpwr_str));
    
    return 0;
}

int32
_priv_device_fiber_set_tx_pwr(tbl_fiber_t *p_fiber, netsnmp_table_row *row)
{
    char txpwr_str[CMD_BUF_256];

    sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr[0]*100));
    netsnmp_set_row_column(row, TBL_FIBER_TX_HIGH_ALARM,    ASN_OCTET_STR, (void*)&txpwr_str, sal_strlen(txpwr_str));
    sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr[1]*100));
    netsnmp_set_row_column(row, TBL_FIBER_TX_LOW_ALARM,     ASN_OCTET_STR, (void*)&txpwr_str, sal_strlen(txpwr_str));
    sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr[2]*100));
    netsnmp_set_row_column(row, TBL_FIBER_TX_HIGH_WARN,     ASN_OCTET_STR, (void*)&txpwr_str, sal_strlen(txpwr_str));
    sal_snprintf(txpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->tx_pwr[3]*100));
    netsnmp_set_row_column(row, TBL_FIBER_TX_LOW_WARN,      ASN_OCTET_STR, (void*)&txpwr_str, sal_strlen(txpwr_str));

    return 0;
}

int32
_priv_device_fiber_set_rx_pwr(tbl_fiber_t *p_fiber, netsnmp_table_row *row)
{
    char rxpwr_str[CMD_BUF_256];

    sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr[0]*100));
    netsnmp_set_row_column(row, TBL_FIBER_RX_HIGH_ALARM,    ASN_OCTET_STR, (void*)&rxpwr_str, sal_strlen(rxpwr_str));
    sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr[1]*100));
    netsnmp_set_row_column(row, TBL_FIBER_RX_LOW_ALARM,     ASN_OCTET_STR, (void*)&rxpwr_str, sal_strlen(rxpwr_str));
    sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr[2]*100));
    netsnmp_set_row_column(row, TBL_FIBER_RX_HIGH_WARN,     ASN_OCTET_STR, (void*)&rxpwr_str, sal_strlen(rxpwr_str));
    sal_snprintf(rxpwr_str, CMD_BUF_256, "%d", (int32)(p_fiber->rx_pwr[3]*100));
    netsnmp_set_row_column(row, TBL_FIBER_RX_LOW_WARN,      ASN_OCTET_STR, (void*)&rxpwr_str, sal_strlen(rxpwr_str));

    return 0;
}

int32
priv_device_fiber_add_row(tbl_fiber_t *p_fiber)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fiber->key.port_id + 1;
    int32 rc = 0;
    
    if (NULL == g_p_fiber_tbl)
    {
        return -1;
    }

    if (!p_fiber->ddm_support)
    {
        return 0;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    _priv_device_fiber_set_tx_pwr2(p_fiber, row);
    _priv_device_fiber_set_rx_pwr2(p_fiber, row);
    _priv_device_fiber_set_tx_pwr(p_fiber, row);
    _priv_device_fiber_set_rx_pwr(p_fiber, row);
    
    rc = netsnmp_table_data_add_row(g_p_fiber_tbl->p_dataset->table, row);
    return rc;
}

int32
priv_device_fiber_set_row(tbl_fiber_t *p_fiber, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fiber->key.port_id + 1;
    int32 rc = 0;
    
    if (NULL == g_p_fiber_tbl)
    {
        return -1;
    }

    if (!p_fiber->ddm_support)
    {
        return 0;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_fiber_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_FIBER_FLD_TX_PWR2:
        _priv_device_fiber_set_tx_pwr2(p_fiber, row);
        break;

    case TBL_FIBER_FLD_RX_PWR2:
        _priv_device_fiber_set_rx_pwr2(p_fiber, row);
        break;

    case TBL_FIBER_FLD_TX_PWR:
        _priv_device_fiber_set_tx_pwr(p_fiber, row);
        break;

    case TBL_FIBER_FLD_RX_PWR:
        _priv_device_fiber_set_rx_pwr(p_fiber, row);
        break;

    case TBL_FIBER_FLD_MAX:
        _priv_device_fiber_set_tx_pwr2(p_fiber, row);
        _priv_device_fiber_set_rx_pwr2(p_fiber, row);
        _priv_device_fiber_set_tx_pwr(p_fiber, row);
        _priv_device_fiber_set_rx_pwr(p_fiber, row);
        break;

    default:
        break;
    }

    rc = netsnmp_table_data_add_row(g_p_fiber_tbl->p_dataset->table, row);
    return rc;
}

int32
priv_device_fiber_del_row(tbl_fiber_key_t *p_fiber_key)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fiber_key->port_id + 1;
    
    if (NULL == g_p_fiber_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_fiber_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_fiber_tbl->p_dataset, row);
    return 0;
}

int32
priv_device_power_add_row(tbl_psu_t *p_psu)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_psu->key.id + 1;
    int32 absent = 0;
    int32 type = 0;
    int32 alert_status = 0;
    int32 run_status = 0;
    int32 rc = 0;
    
    if (NULL == g_p_power_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    absent = _priv_device_power_get_absent(p_psu);
    type = _priv_device_power_get_type(p_psu);
    alert_status = _priv_device_power_get_alert_status(p_psu);
    run_status = _priv_device_power_get_run_status(p_psu);
    
    netsnmp_set_row_column(row, TBL_POWER_STATUS,       ASN_INTEGER, (void*)&absent, sizeof(absent));
    netsnmp_set_row_column(row, TBL_POWER_TYPE,         ASN_INTEGER, (void*)&type, sizeof(type));
    netsnmp_set_row_column(row, TBL_POWER_ALERT_STATUS, ASN_INTEGER, (void*)&alert_status, sizeof(alert_status));
    netsnmp_set_row_column(row, TBL_POWER_WORK_STATUS,  ASN_INTEGER, (void*)&run_status, sizeof(run_status));

    rc = netsnmp_table_data_add_row(g_p_power_tbl->p_dataset->table, row);
    return rc;
}

int32
priv_device_power_set_row(tbl_psu_t *p_psu, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_psu->key.id + 1;
    int32 value = 0;
    
    if (NULL == g_p_power_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_power_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    switch (field_id)
    {
    case TBL_PSU_FLD_ABSENT:
        value = _priv_device_power_get_absent(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        /* absent maybe affect alert_status */
        value = _priv_device_power_get_alert_status(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_ALERT_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        value = _priv_device_power_get_run_status(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_WORK_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    case TBL_PSU_FLD_MODE:
        value = _priv_device_power_get_type(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_TYPE, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    case TBL_PSU_FLD_ALERT_STATUS:
        value = _priv_device_power_get_alert_status(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_ALERT_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    case TBL_PSU_FLD_RUN_STATUS:
        value = _priv_device_power_get_run_status(p_psu);
        netsnmp_set_row_column(row, TBL_POWER_WORK_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    default:
        break;
    }

    return 0;
}

int32
priv_device_power_del_row(tbl_psu_key_t *p_psu_key)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_psu_key->id + 1;
    
    if (NULL == g_p_power_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_power_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_power_tbl->p_dataset, row);
    return 0;
}

int32
priv_device_fan_add_row(tbl_fan_t *p_fan)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fan->key.id + 1;
    int32 module = 0;
    int32 status = 0;
    int32 speed = 0;
    int32 rc = 0;
    
    if (NULL == g_p_fan_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    module = _priv_device_fan_get_module(p_fan);
    status = _priv_device_fan_get_status(p_fan);
    speed = _priv_device_fan_get_speed(p_fan);
    
    netsnmp_set_row_column(row, TBL_FAN_MODULE,         ASN_INTEGER, (void*)&module, sizeof(module));
    netsnmp_set_row_column(row, TBL_FAN_STATUS,         ASN_INTEGER, (void*)&status, sizeof(status));
    netsnmp_set_row_column(row, TBL_FAN_SPEED,          ASN_INTEGER, (void*)&speed, sizeof(speed));

    rc = netsnmp_table_data_add_row(g_p_fan_tbl->p_dataset->table, row);
    return rc;
}

int32
priv_device_fan_set_row(tbl_fan_t *p_fan, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fan->key.id + 1;
    int32 value = 0;
    
    if (NULL == g_p_fan_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_fan_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    switch (field_id)
    {
    case TBL_FAN_FLD_TRAY:
        value = _priv_device_fan_get_module(p_fan);
        netsnmp_set_row_column(row, TBL_FAN_MODULE, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    case TBL_FAN_FLD_STATUS:
        value = _priv_device_fan_get_status(p_fan);
        netsnmp_set_row_column(row, TBL_FAN_STATUS, ASN_INTEGER, (void*)&value, sizeof(value));
        /* status maybe affect speed */
        value = _priv_device_fan_get_speed(p_fan);
        netsnmp_set_row_column(row, TBL_FAN_SPEED, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    case TBL_FAN_FLD_PERCENT:
        value = _priv_device_fan_get_speed(p_fan);
        netsnmp_set_row_column(row, TBL_FAN_SPEED, ASN_INTEGER, (void*)&value, sizeof(value));
        break;

    default:
        break;
    }

    return 0;
}

int32
priv_device_fan_del_row(tbl_fan_key_t *p_fan_key)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_fan_key->id + 1;
    
    if (NULL == g_p_fan_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_fan_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_fan_tbl->p_dataset, row);
    return 0;
}

int32
priv_device_sensor_add_row(tbl_sensor_t *p_sensor)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_sensor->key.id + 1;
    int32 pos = 0;
    int32 rc = 0;
    
    if (NULL == g_p_tempr_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));
    
    /* columns */
    if (0 == sal_strcmp(p_sensor->position, "AROUND_CPU"))
    {
        pos = TBL_TMP_POSISION_CPU;
    }
    else if (0 == sal_strcmp(p_sensor->position, "BEFORE_CHIP"))
    {
        pos = TBL_TMP_POSISION_BEFOR_CHIP;
    }
    else if (0 == sal_strcmp(p_sensor->position, "BEHIND_CHIP"))
    {
        pos = TBL_TMP_POSISION_BEHIND_CHIP;
    }
    else if (0 == sal_strcmp(p_sensor->position, "AROUND_FAN"))
    {
        pos = TBL_TMP_POSISION_FAN;
    }
    else if (0 == sal_strcmp(p_sensor->position, "AROUND_PSU"))
    {
        pos = TBL_TMP_POSISION_CHIP0;
    }
    else if (0 == sal_strcmp(p_sensor->position, "SWITCH_CHIP0"))
    {
        pos = TBL_TMP_POSISION_CHIP0;
    }
    else if (0 == sal_strcmp(p_sensor->position, "SWITCH_CHIP1"))
    {
        pos = TBL_TMP_POSISION_CHIP1;
    }
    else if (0 == sal_strcmp(p_sensor->position, "AROUND_CHIP"))
    {
        pos = TBL_TMP_POSISION_AROUND_CHIP;
    }

    netsnmp_set_row_column(row, TBL_TEMPR_TEMPERATURE,  ASN_INTEGER, (void*)&p_sensor->temperature, sizeof(p_sensor->temperature));
    netsnmp_set_row_column(row, TBL_TEMPR_LOW,          ASN_INTEGER, (void*)&p_sensor->tmpr_low, sizeof(p_sensor->tmpr_low));
    netsnmp_set_row_column(row, TBL_TEMPR_HIGH,         ASN_INTEGER, (void*)&p_sensor->tmpr_high, sizeof(p_sensor->tmpr_high));
    netsnmp_set_row_column(row, TBL_TEMPR_CRITICAL,     ASN_INTEGER, (void*)&p_sensor->tmpr_critical, sizeof(p_sensor->tmpr_critical));
    netsnmp_set_row_column(row, TBL_TEMPR_LOCATION,     ASN_INTEGER, (void*)&pos, sizeof(pos));
    
    rc = netsnmp_table_data_add_row(g_p_tempr_tbl->p_dataset->table, row);
    return rc;
}

int32
priv_device_sensor_set_row(tbl_sensor_t *p_sensor, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_sensor->key.id + 1;
    
    if (NULL == g_p_tempr_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_tempr_tbl, index);
    if (NULL == row)
    {
        return -1;
    }

    switch (field_id)
    {
    case TBL_SENSOR_FLD_TEMPERATURE:
        netsnmp_set_row_column(row, TBL_TEMPR_TEMPERATURE,  ASN_INTEGER, (void*)&p_sensor->temperature, sizeof(p_sensor->temperature));
        break;

    case TBL_SENSOR_FLD_TMPR_LOW:
        netsnmp_set_row_column(row, TBL_TEMPR_LOW,          ASN_INTEGER, (void*)&p_sensor->tmpr_low, sizeof(p_sensor->tmpr_low));
        break;

    case TBL_SENSOR_FLD_TMPR_HIGH:
        netsnmp_set_row_column(row, TBL_TEMPR_HIGH,         ASN_INTEGER, (void*)&p_sensor->tmpr_high, sizeof(p_sensor->tmpr_high));
        break;

    case TBL_SENSOR_FLD_TMPR_CRITICAL:
        netsnmp_set_row_column(row, TBL_TEMPR_CRITICAL,     ASN_INTEGER, (void*)&p_sensor->tmpr_critical, sizeof(p_sensor->tmpr_critical));
        break;

    default:
        break;
    }

    return 0;
}

int32
priv_device_sensor_del_row(tbl_sensor_key_t *p_sensor_key)
{
    netsnmp_table_row *row = NULL;
    int32 index = p_sensor_key->id + 1;
    
    if (NULL == g_p_tempr_tbl)
    {
        return -1;
    }

    row = snmpagt_table_get_row_by_int32_index(g_p_tempr_tbl, index);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_tempr_tbl->p_dataset, row);
    return 0;
}

int
priv_device_fiber_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int
priv_device_power_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int
priv_device_fan_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int
priv_device_tempr_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int32
priv_device_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];

    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(FIBER_OID, sizeof(FIBER_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = FIBER_ROW;
    param.row_count = sizeof(FIBER_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = priv_device_fiber_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_fiber_tbl = table;


    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(POWER_OID, sizeof(POWER_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = POWER_ROW;
    param.row_count = sizeof(POWER_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = priv_device_power_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_power_tbl = table;

    
    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(FAN_OID, sizeof(FAN_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = FAN_ROW;
    param.row_count = sizeof(FAN_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = priv_device_fan_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_fan_tbl = table;

    
    sal_memset(&oid_str, 0, sizeof(oid_str));
    sal_memset(&param, 0, sizeof(param));    
    snmpagt_oid2str(TEMPR_OID, sizeof(TEMPR_OID)/sizeof(oid), oid_str, CMD_BUF_256);
    param.oid_str = oid_str;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = TEMPR_ROW;
    param.row_count = sizeof(TEMPR_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = priv_device_tempr_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }
    g_p_tempr_tbl = table;

    return 0;
}


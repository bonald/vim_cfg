
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

#include "lacp_define.h"

const char *lacp_selected_strs[SELECTED_STATE_MAX] =
{
    "UNSELECTED",
    "SELECTED",
    "STANDBY"
};

const char *lacp_rx_event_strs[RX_EVENT_MAX] =
{
    "begin",
    "port_moved",
    "portenabled_lacpenabled",
    "portenabled_lacpdisabled",
    "currwhile_timer_expired",
    "pdu_rx",
    "port_disabled"
};

const char *lacp_rx_state_strs[RX_STATE_MAX] =
{
    "INITIALIZE",
    "PORT_DISABLED",
    "LACP_DISABLED",
    "EXPIRED",
    "DEFAULTED",
    "CURRENT"
};

const char *lacp_pertx_event_strs[PERTX_EVENT_MAX] =
{
    "disable",
    "enable",
    "short_timeout",
    "long_timeout",
    "periodic_time_expired"
};

const char *lacp_pertx_state_strs[PERTX_STATE_MAX] =
{
    "NO_PERIODIC",
    "FAST_PERIODIC",
    "SLOW_PERIODIC",
    "PERIODIC_TX"
};

const char *lacp_mux_event_strs[MUX_EVENT_MAX] =
{
    "begin",
    "selected",
    "standby",
    "unselected",
    "selected+ready",
    "selected+partner_insync",
    "partner_outofsync"
};

const char *lacp_mux_state_strs[MUX_STATE_MAX] =
{
    "DETACHED",
    "WAITING",
    "ATTACHED",
    "COLLECTING_DISTRIBUTING"
};

int32
ds_lacp_add_lacp_sync(tbl_interface_t *p_if, ds_lacp_t *p_lacp, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lacp_t *p_db_lacp = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_lacp = p_db_if->lacp;
    if (p_db_lacp)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lacp = XCALLOC(MEM_DS_LACP, sizeof(ds_lacp_t));
    if (NULL == p_db_lacp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lacp, p_lacp, sizeof(ds_lacp_t));

    /* 4. add to db */
    p_db_if->lacp = p_db_lacp;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_LACP, p_db_if, p_db_lacp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_LACP, p_db_if, p_db_lacp);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lacp_del_lacp_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lacp_t *p_db_lacp = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_lacp = p_db_if->lacp;
    if (NULL == p_db_lacp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_LACP, p_db_if, p_db_lacp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_LACP, p_db_if, p_db_lacp);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->lacp = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_LACP, p_db_lacp);

    return PM_E_NONE;
}

int32
ds_lacp_set_lacp_field_sync(tbl_interface_t *p_if, ds_lacp_t *p_lacp, ds_lacp_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lacp_t *p_db_lacp = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_lacp = ds_lacp_get_lacp(p_if);
    if (NULL == p_db_lacp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_LACP_FLD_ACTOR_PORT_PRIORITY:
        p_db_lacp->actor_port_priority = p_lacp->actor_port_priority;
        break;

    case DS_LACP_FLD_ACTOR_OPER_PORT_KEY:
        p_db_lacp->actor_oper_port_key = p_lacp->actor_oper_port_key;
        break;

    case DS_LACP_FLD_ACTOR_PORT_NUMBER:
        p_db_lacp->actor_port_number = p_lacp->actor_port_number;
        break;

    case DS_LACP_FLD_SHORT_TIMEOUT:
        p_db_lacp->short_timeout = p_lacp->short_timeout;
        break;

    case DS_LACP_FLD_ACTOR_OPER_PORT_STATE:
        p_db_lacp->actor_oper_port_state = p_lacp->actor_oper_port_state;
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY:
        p_db_lacp->partner_admin_system_priority = p_lacp->partner_admin_system_priority;
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_SYSTEM:
        sal_memcpy(p_db_lacp->partner_admin_system, p_lacp->partner_admin_system, sizeof(p_lacp->partner_admin_system));
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_KEY:
        p_db_lacp->partner_admin_key = p_lacp->partner_admin_key;
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY:
        p_db_lacp->partner_admin_port_priority = p_lacp->partner_admin_port_priority;
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER:
        p_db_lacp->partner_admin_port_number = p_lacp->partner_admin_port_number;
        break;

    case DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE:
        p_db_lacp->partner_admin_port_state = p_lacp->partner_admin_port_state;
        break;

    case DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY:
        p_db_lacp->partner_oper_system_priority = p_lacp->partner_oper_system_priority;
        break;

    case DS_LACP_FLD_PARTNER_OPER_SYSTEM:
        sal_memcpy(p_db_lacp->partner_oper_system, p_lacp->partner_oper_system, sizeof(p_lacp->partner_oper_system));
        break;

    case DS_LACP_FLD_PARTNER_OPER_KEY:
        p_db_lacp->partner_oper_key = p_lacp->partner_oper_key;
        break;

    case DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY:
        p_db_lacp->partner_oper_port_priority = p_lacp->partner_oper_port_priority;
        break;

    case DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER:
        p_db_lacp->partner_oper_port_number = p_lacp->partner_oper_port_number;
        break;

    case DS_LACP_FLD_PARTNER_OPER_PORT_STATE:
        p_db_lacp->partner_oper_port_state = p_lacp->partner_oper_port_state;
        break;

    case DS_LACP_FLD_TX_LACP_COUNT:
        p_db_lacp->tx_lacp_count = p_lacp->tx_lacp_count;
        break;

    case DS_LACP_FLD_TX_ERROR_COUNT:
        p_db_lacp->tx_error_count = p_lacp->tx_error_count;
        break;

    case DS_LACP_FLD_RX_LACP_COUNT:
        p_db_lacp->rx_lacp_count = p_lacp->rx_lacp_count;
        break;

    case DS_LACP_FLD_RX_ERROR_COUNT:
        p_db_lacp->rx_error_count = p_lacp->rx_error_count;
        break;

    case DS_LACP_FLD_MLAG_ID:
        p_db_lacp->mlag_id = p_lacp->mlag_id;
        break;

    case DS_LACP_FLD_NTT:
        p_db_lacp->ntt = p_lacp->ntt;
        break;

    case DS_LACP_FLD_LACP_ENABLED:
        p_db_lacp->lacp_enabled = p_lacp->lacp_enabled;
        break;

    case DS_LACP_FLD_READY_N:
        p_db_lacp->ready_n = p_lacp->ready_n;
        break;

    case DS_LACP_FLD_PORT_MOVED:
        p_db_lacp->port_moved = p_lacp->port_moved;
        break;

    case DS_LACP_FLD_PERIODIC_TX_STATE:
        p_db_lacp->periodic_tx_state = p_lacp->periodic_tx_state;
        break;

    case DS_LACP_FLD_RECEIVE_STATE:
        p_db_lacp->receive_state = p_lacp->receive_state;
        break;

    case DS_LACP_FLD_MUX_STATE:
        p_db_lacp->mux_state = p_lacp->mux_state;
        break;

    case DS_LACP_FLD_SELECTED:
        p_db_lacp->selected = p_lacp->selected;
        break;

    case DS_LACP_FLD_PDU:
        p_db_lacp->pdu = p_lacp->pdu;
        break;

    case DS_LACP_FLD_CURRENT_WHILE_TIMER:
        p_db_lacp->current_while_timer = p_lacp->current_while_timer;
        break;

    case DS_LACP_FLD_PERIODIC_TIMER:
        p_db_lacp->periodic_timer = p_lacp->periodic_timer;
        break;

    case DS_LACP_FLD_WAIT_WHILE_TIMER:
        p_db_lacp->wait_while_timer = p_lacp->wait_while_timer;
        break;

    case DS_LACP_FLD_MAX:
        sal_memcpy(p_db_lacp, p_lacp, sizeof(ds_lacp_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_LACP, field_id, p_db_if, p_db_lacp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_LACP, field_id, p_db_if, p_db_lacp);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lacp_add_lacp(tbl_interface_t *p_if, ds_lacp_t *p_lacp)
{
    return ds_lacp_add_lacp_sync(p_if, p_lacp, TRUE);
}

int32
ds_lacp_del_lacp(tbl_interface_t *p_if)
{
    return ds_lacp_del_lacp_sync(p_if, TRUE);
}

int32
ds_lacp_set_lacp_field(tbl_interface_t *p_if, ds_lacp_t *p_lacp, ds_lacp_field_id_t field_id)
{
    return ds_lacp_set_lacp_field_sync(p_if, p_lacp, field_id, TRUE);
}

ds_lacp_t*
ds_lacp_get_lacp(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->lacp;
}

int32
ds_lacp_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_lacp_t *p_lacp)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_lacp_key_value_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_lacp_key_name_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_LACP);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_lacp_key_value_dump(p_if, p_lacp, ds_str));
    return str;
}

char*
ds_lacp_field_name_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_LACP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_LACP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_lacp_field_value_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->actor_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->actor_oper_port_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->actor_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_SHORT_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->short_timeout);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->actor_oper_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_admin_system_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lacp->partner_admin_system);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_admin_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_admin_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_admin_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_admin_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_oper_system_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lacp->partner_oper_system);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_oper_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_oper_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_oper_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->partner_oper_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_LACP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->tx_lacp_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_ERROR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->tx_error_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_LACP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->rx_lacp_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_ERROR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->rx_error_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_MLAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->mlag_id);
    }
    if (FLD_MATCH(DS_LACP_FLD_NTT, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->ntt);
    }
    if (FLD_MATCH(DS_LACP_FLD_LACP_ENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->lacp_enabled);
    }
    if (FLD_MATCH(DS_LACP_FLD_READY_N, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->ready_n);
    }
    if (FLD_MATCH(DS_LACP_FLD_PORT_MOVED, field_id))
    {
        sal_sprintf(str, "%u", p_lacp->port_moved);
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TX_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(lacp_pertx_state_strs, LACP_PERTX_STATE_MAX, p_lacp->periodic_tx_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_RECEIVE_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(lacp_rx_state_strs, LACP_RX_STATE_MAX, p_lacp->receive_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_MUX_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(lacp_mux_state_strs, LACP_MUX_STATE_MAX, p_lacp->mux_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_SELECTED, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(lacp_selected_strs, LACP_SELECTED_MAX, p_lacp->selected));
    }
    if (FLD_MATCH(DS_LACP_FLD_PDU, field_id))
    {
    }
    if (FLD_MATCH(DS_LACP_FLD_CURRENT_WHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_lacp->current_while_timer);
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_lacp->periodic_timer);
    }
    if (FLD_MATCH(DS_LACP_FLD_WAIT_WHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_lacp->wait_while_timer);
    }
    return str;
}

char**
ds_lacp_table_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_lacp_key_name_dump(p_if, p_lacp, buf));
    for(i=1; i<DS_LACP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_lacp_field_name_dump(p_if, p_lacp, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_lacp_field_value_dump(p_if, p_lacp, i, buf));
    }
    return str;
}

int32
ds_lacp_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_lacp_t *p_lacp)
{
    int32 ret=0;
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_PRIORITY, field_id))
    {
        p_lacp->actor_port_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_KEY, field_id))
    {
        p_lacp->actor_oper_port_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_NUMBER, field_id))
    {
        p_lacp->actor_port_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_SHORT_TIMEOUT, field_id))
    {
        p_lacp->short_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_STATE, field_id))
    {
        p_lacp->actor_oper_port_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY, field_id))
    {
        p_lacp->partner_admin_system_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM, field_id))
    {
        cdb_mac_addr_str2val(p_lacp->partner_admin_system, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_KEY, field_id))
    {
        p_lacp->partner_admin_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY, field_id))
    {
        p_lacp->partner_admin_port_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER, field_id))
    {
        p_lacp->partner_admin_port_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE, field_id))
    {
        p_lacp->partner_admin_port_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY, field_id))
    {
        p_lacp->partner_oper_system_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM, field_id))
    {
        cdb_mac_addr_str2val(p_lacp->partner_oper_system, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_KEY, field_id))
    {
        p_lacp->partner_oper_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY, field_id))
    {
        p_lacp->partner_oper_port_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER, field_id))
    {
        p_lacp->partner_oper_port_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_STATE, field_id))
    {
        p_lacp->partner_oper_port_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_LACP_COUNT, field_id))
    {
        p_lacp->tx_lacp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_ERROR_COUNT, field_id))
    {
        p_lacp->tx_error_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_LACP_COUNT, field_id))
    {
        p_lacp->rx_lacp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_ERROR_COUNT, field_id))
    {
        p_lacp->rx_error_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_MLAG_ID, field_id))
    {
        p_lacp->mlag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_NTT, field_id))
    {
        p_lacp->ntt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_LACP_ENABLED, field_id))
    {
        p_lacp->lacp_enabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_READY_N, field_id))
    {
        p_lacp->ready_n = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PORT_MOVED, field_id))
    {
        p_lacp->port_moved = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TX_STATE, field_id))
    {
        cdb_enum_str2val(lacp_pertx_state_strs, LACP_PERTX_STATE_MAX, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_RECEIVE_STATE, field_id))
    {
        cdb_enum_str2val(lacp_rx_state_strs, LACP_RX_STATE_MAX, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_MUX_STATE, field_id))
    {
        cdb_enum_str2val(lacp_mux_state_strs, LACP_MUX_STATE_MAX, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_SELECTED, field_id))
    {
        cdb_enum_str2val(lacp_selected_strs, LACP_SELECTED_MAX, str);
    }
    if (FLD_MATCH(DS_LACP_FLD_PDU, field_id))
    {
    }
    if (FLD_MATCH(DS_LACP_FLD_CURRENT_WHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_lacp->current_while_timer);
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TIMER, field_id))
    {
        ctc_task_str2val(str, p_lacp->periodic_timer);
    }
    if (FLD_MATCH(DS_LACP_FLD_WAIT_WHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_lacp->wait_while_timer);
    }
    return ret;
}

int32
ds_lacp_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_lacp_t *p_lacp)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_LACP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_lacp_key_str2val(tbl_key_value, ds_key_value, p_if, p_lacp));

    for(i=1; i<DS_LACP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_lacp_field_value_parser( array[j++], i, p_if, p_lacp));
    }

    return PM_E_NONE;
}

int32
ds_lacp_dump_one(ds_lacp_t *p_lacp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_LACP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_ACTOR_PORT_PRIORITY].name,
            p_lacp->actor_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_ACTOR_OPER_PORT_KEY].name,
            p_lacp->actor_oper_port_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_PORT_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_ACTOR_PORT_NUMBER].name,
            p_lacp->actor_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_SHORT_TIMEOUT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_SHORT_TIMEOUT].name,
            p_lacp->short_timeout);
    }
    if (FLD_MATCH(DS_LACP_FLD_ACTOR_OPER_PORT_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_ACTOR_OPER_PORT_STATE].name,
            p_lacp->actor_oper_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY].name,
            p_lacp->partner_admin_system_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_SYSTEM, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_SYSTEM].name,
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lacp->partner_admin_system));
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_KEY].name,
            p_lacp->partner_admin_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY].name,
            p_lacp->partner_admin_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER].name,
            p_lacp->partner_admin_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE].name,
            p_lacp->partner_admin_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY].name,
            p_lacp->partner_oper_system_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_SYSTEM, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_SYSTEM].name,
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lacp->partner_oper_system));
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_KEY].name,
            p_lacp->partner_oper_key);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY].name,
            p_lacp->partner_oper_port_priority);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER].name,
            p_lacp->partner_oper_port_number);
    }
    if (FLD_MATCH(DS_LACP_FLD_PARTNER_OPER_PORT_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PARTNER_OPER_PORT_STATE].name,
            p_lacp->partner_oper_port_state);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_LACP_COUNT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_TX_LACP_COUNT].name,
            p_lacp->tx_lacp_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_TX_ERROR_COUNT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_TX_ERROR_COUNT].name,
            p_lacp->tx_error_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_LACP_COUNT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_RX_LACP_COUNT].name,
            p_lacp->rx_lacp_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_RX_ERROR_COUNT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_RX_ERROR_COUNT].name,
            p_lacp->rx_error_count);
    }
    if (FLD_MATCH(DS_LACP_FLD_MLAG_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_MLAG_ID].name,
            p_lacp->mlag_id);
    }
    if (FLD_MATCH(DS_LACP_FLD_NTT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_NTT].name,
            p_lacp->ntt);
    }
    if (FLD_MATCH(DS_LACP_FLD_LACP_ENABLED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_LACP_ENABLED].name,
            p_lacp->lacp_enabled);
    }
    if (FLD_MATCH(DS_LACP_FLD_READY_N, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_READY_N].name,
            p_lacp->ready_n);
    }
    if (FLD_MATCH(DS_LACP_FLD_PORT_MOVED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LACP_FLD_PORT_MOVED].name,
            p_lacp->port_moved);
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TX_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_PERIODIC_TX_STATE].name,
            cdb_enum_val2str(lacp_pertx_state_strs, LACP_PERTX_STATE_MAX, p_lacp->periodic_tx_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_RECEIVE_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_RECEIVE_STATE].name,
            cdb_enum_val2str(lacp_rx_state_strs, LACP_RX_STATE_MAX, p_lacp->receive_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_MUX_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_MUX_STATE].name,
            cdb_enum_val2str(lacp_mux_state_strs, LACP_MUX_STATE_MAX, p_lacp->mux_state));
    }
    if (FLD_MATCH(DS_LACP_FLD_SELECTED, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_SELECTED].name,
            cdb_enum_val2str(lacp_selected_strs, LACP_SELECTED_MAX, p_lacp->selected));
    }
    if (FLD_MATCH(DS_LACP_FLD_PDU, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[DS_LACP_FLD_PDU].name);
    }
    if (FLD_MATCH(DS_LACP_FLD_CURRENT_WHILE_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_CURRENT_WHILE_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_lacp->current_while_timer));
    }
    if (FLD_MATCH(DS_LACP_FLD_PERIODIC_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_PERIODIC_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_lacp->periodic_timer));
    }
    if (FLD_MATCH(DS_LACP_FLD_WAIT_WHILE_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LACP_FLD_WAIT_WHILE_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_lacp->wait_while_timer));
    }

    return PM_E_NONE;
}

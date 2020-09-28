
#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "cdb_data_cmp.h"

tbl_sys_global_t *_g_p_tbl_sys_global = NULL;

int32
tbl_sys_global_set_sys_global_field_sync(tbl_sys_global_t *p_sys_glb, tbl_sys_global_field_id_t field_id, uint32 sync)
{
    tbl_sys_global_t *p_db_sys_glb = _g_p_tbl_sys_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SYS_GLOBAL_FLD_HOSTNAME:
        sal_memcpy(p_db_sys_glb->hostname, p_sys_glb->hostname, sizeof(p_sys_glb->hostname));
        break;

    case TBL_SYS_GLOBAL_FLD_ROUTE_MAC:
        sal_memcpy(p_db_sys_glb->route_mac, p_sys_glb->route_mac, sizeof(p_sys_glb->route_mac));
        break;

    case TBL_SYS_GLOBAL_FLD_MAC_NUM:
        p_db_sys_glb->mac_num = p_sys_glb->mac_num;
        break;

    case TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT:
        p_db_sys_glb->curr_vlanif_count = p_sys_glb->curr_vlanif_count;
        break;

    case TBL_SYS_GLOBAL_FLD_INIT_DONE:
        p_db_sys_glb->init_done = p_sys_glb->init_done;
        break;

    case TBL_SYS_GLOBAL_FLD_STARTUP_DONE:
        p_db_sys_glb->startup_done = p_sys_glb->startup_done;
        break;

    case TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE:
        p_db_sys_glb->max_frame_size = p_sys_glb->max_frame_size;
        break;

    case TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE:
        p_db_sys_glb->jumbo_frame_size = p_sys_glb->jumbo_frame_size;
        break;

    case TBL_SYS_GLOBAL_FLD_REBOOT_TYPE:
        p_db_sys_glb->reboot_type = p_sys_glb->reboot_type;
        break;

    case TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE:
        p_db_sys_glb->ipg_shaping_enable = p_sys_glb->ipg_shaping_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE:
        p_db_sys_glb->ipg_policing_enable = p_sys_glb->ipg_policing_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE:
        p_db_sys_glb->ipg_storm_control_enable = p_sys_glb->ipg_storm_control_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL:
        p_db_sys_glb->aaa_new_model = p_sys_glb->aaa_new_model;
        break;

    case TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION:
        p_db_sys_glb->service_password_encryption = p_sys_glb->service_password_encryption;
        break;

    case TBL_SYS_GLOBAL_FLD_MAX_VTY:
        p_db_sys_glb->max_vty = p_sys_glb->max_vty;
        break;

    case TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1:
        p_db_sys_glb->aaa_privilege1 = p_sys_glb->aaa_privilege1;
        break;

    case TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2:
        p_db_sys_glb->aaa_privilege2 = p_sys_glb->aaa_privilege2;
        break;

    case TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3:
        p_db_sys_glb->aaa_privilege3 = p_sys_glb->aaa_privilege3;
        break;

    case TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE:
        p_db_sys_glb->dhcp_service_enable = p_sys_glb->dhcp_service_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE:
        p_db_sys_glb->dhcp_relay_enable = p_sys_glb->dhcp_relay_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE:
        p_db_sys_glb->dhcp_snooping_enable = p_sys_glb->dhcp_snooping_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE:
        p_db_sys_glb->http_service_enable = p_sys_glb->http_service_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME:
        sal_memcpy(p_db_sys_glb->http_image_name, p_sys_glb->http_image_name, sizeof(p_sys_glb->http_image_name));
        break;

    case TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL:
        p_db_sys_glb->errdisable_interval = p_sys_glb->errdisable_interval;
        break;

    case TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH:
        p_db_sys_glb->trunction_length = p_sys_glb->trunction_length;
        break;

    case TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA:
        sal_memcpy(p_db_sys_glb->tap_ts_macda, p_sys_glb->tap_ts_macda, sizeof(p_sys_glb->tap_ts_macda));
        break;

    case TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA:
        sal_memcpy(p_db_sys_glb->tap_ts_macsa, p_sys_glb->tap_ts_macsa, sizeof(p_sys_glb->tap_ts_macsa));
        break;

    case TBL_SYS_GLOBAL_FLD_ETHER_TYPE:
        p_db_sys_glb->ether_type = p_sys_glb->ether_type;
        break;

    case TBL_SYS_GLOBAL_FLD_PTF_PORT:
        p_db_sys_glb->ptf_port = p_sys_glb->ptf_port;
        break;

    case TBL_SYS_GLOBAL_FLD_TELNET_DISABLE:
        p_db_sys_glb->telnet_disable = p_sys_glb->telnet_disable;
        break;

    case TBL_SYS_GLOBAL_FLD_TELNET_PORT:
        p_db_sys_glb->telnet_port = p_sys_glb->telnet_port;
        break;

    case TBL_SYS_GLOBAL_FLD_TELNET_ACL:
        sal_memcpy(p_db_sys_glb->telnet_acl, p_sys_glb->telnet_acl, sizeof(p_sys_glb->telnet_acl));
        break;

    case TBL_SYS_GLOBAL_FLD_SSH_ACL:
        sal_memcpy(p_db_sys_glb->ssh_acl, p_sys_glb->ssh_acl, sizeof(p_sys_glb->ssh_acl));
        break;

    case TBL_SYS_GLOBAL_FLD_HTTP_DISABLE:
        p_db_sys_glb->http_disable = p_sys_glb->http_disable;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTP_PORT:
        p_db_sys_glb->http_port = p_sys_glb->http_port;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT:
        p_db_sys_glb->http_timeout = p_sys_glb->http_timeout;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE:
        p_db_sys_glb->https_disable = p_sys_glb->https_disable;
        break;

    case TBL_SYS_GLOBAL_FLD_HTTPS_PORT:
        p_db_sys_glb->https_port = p_sys_glb->https_port;
        break;

    case TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE:
        p_db_sys_glb->rpcapi_disable = p_sys_glb->rpcapi_disable;
        break;

    case TBL_SYS_GLOBAL_FLD_RPCAPI_PORT:
        p_db_sys_glb->rpcapi_port = p_sys_glb->rpcapi_port;
        break;

    case TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN:
        p_db_sys_glb->rpcapi_ssl_en = p_sys_glb->rpcapi_ssl_en;
        break;

    case TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT:
        p_db_sys_glb->rpcapi_ssl_port = p_sys_glb->rpcapi_ssl_port;
        break;

    case TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE:
        p_db_sys_glb->rpcapi_auth_mode = p_sys_glb->rpcapi_auth_mode;
        break;

    case TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE:
        p_db_sys_glb->oper_rpcapi_auth_mode = p_sys_glb->oper_rpcapi_auth_mode;
        break;

    case TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE:
        p_db_sys_glb->cut_through_enable = p_sys_glb->cut_through_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT:
        p_db_sys_glb->protected_vlan_cnt = p_sys_glb->protected_vlan_cnt;
        break;

    case TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT:
        p_db_sys_glb->vlan_stats_cnt = p_sys_glb->vlan_stats_cnt;
        break;

    case TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM:
        p_db_sys_glb->mib_port_stats_read_num = p_sys_glb->mib_port_stats_read_num;
        break;

    case TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM:
        p_db_sys_glb->mib_flow_stats_read_num = p_sys_glb->mib_flow_stats_read_num;
        break;

    case TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1:
        p_db_sys_glb->memory_threshold1 = p_sys_glb->memory_threshold1;
        break;

    case TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2:
        p_db_sys_glb->memory_threshold2 = p_sys_glb->memory_threshold2;
        break;

    case TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3:
        p_db_sys_glb->memory_threshold3 = p_sys_glb->memory_threshold3;
        break;

    case TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE:
        p_db_sys_glb->system_memchk_state = p_sys_glb->system_memchk_state;
        break;

    case TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING:
        p_db_sys_glb->mgmt_if_running = p_sys_glb->mgmt_if_running;
        break;

    case TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER:
        p_db_sys_glb->mgmt_if_timer = p_sys_glb->mgmt_if_timer;
        break;

    case TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE:
        p_db_sys_glb->warmboot_type = p_sys_glb->warmboot_type;
        break;

    case TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME:
        p_db_sys_glb->warmboot_time = p_sys_glb->warmboot_time;
        break;

    case TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT:
        p_db_sys_glb->warmboot_count = p_sys_glb->warmboot_count;
        break;

    case TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE:
        p_db_sys_glb->system_time_update = p_sys_glb->system_time_update;
        break;

    case TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX:
        p_db_sys_glb->sflow_counter_ifindex = p_sys_glb->sflow_counter_ifindex;
        break;

    case TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE:
        p_db_sys_glb->dhcp_server_enable = p_sys_glb->dhcp_server_enable;
        break;

    case TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM:
        p_db_sys_glb->add_gre_ace_entry_num = p_sys_glb->add_gre_ace_entry_num;
        break;

    case TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM:
        p_db_sys_glb->add_vxlan_entry_num = p_sys_glb->add_vxlan_entry_num;
        break;

    case TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT:
        p_db_sys_glb->system_product = p_sys_glb->system_product;
        break;

    case TBL_SYS_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_sys_glb, p_sys_glb, sizeof(tbl_sys_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SYS_GLOBAL, field_id, p_db_sys_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SYS_GLOBAL, field_id, p_db_sys_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sys_global_set_sys_global_field(tbl_sys_global_t *p_sys_glb, tbl_sys_global_field_id_t field_id)
{
    return tbl_sys_global_set_sys_global_field_sync(p_sys_glb, field_id, TRUE);
}

tbl_sys_global_t*
tbl_sys_global_get_sys_global()
{
    return _g_p_tbl_sys_global;
}

int32
tbl_sys_global_key_str2val(char *str, tbl_sys_global_t *p_sys_glb)
{
    return PM_E_NONE;
}

char*
tbl_sys_global_key_name_dump(tbl_sys_global_t *p_sys_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_sys_global_key_value_dump(tbl_sys_global_t *p_sys_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_sys_global_field_name_dump(tbl_sys_global_t *p_sys_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SYS_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sys_global_field_value_dump(tbl_sys_global_t *p_sys_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HOSTNAME, field_id))
    {
        sal_sprintf(str, "%s", p_sys_glb->hostname);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ROUTE_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_sys_glb->route_mac);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAC_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->mac_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT, field_id))
    {
        sal_sprintf(str, "%d", p_sys_glb->curr_vlanif_count);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_INIT_DONE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->init_done);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_STARTUP_DONE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->startup_done);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->max_frame_size);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->jumbo_frame_size);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_REBOOT_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_reboot_type_strs, GLB_REBOOT_TYPE_MAX, p_sys_glb->reboot_type));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->ipg_shaping_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->ipg_policing_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->ipg_storm_control_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->aaa_new_model);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->service_password_encryption);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_VTY, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->max_vty);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->aaa_privilege1);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->aaa_privilege2);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->aaa_privilege3);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->dhcp_service_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->dhcp_relay_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->dhcp_snooping_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->http_service_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_sys_glb->http_image_name);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->errdisable_interval);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->trunction_length);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_sys_glb->tap_ts_macda);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_sys_glb->tap_ts_macsa);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ETHER_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->ether_type);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PTF_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->ptf_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->telnet_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->telnet_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_sys_glb->telnet_acl);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SSH_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_sys_glb->ssh_acl);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->http_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->http_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->http_timeout);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->https_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->https_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->rpcapi_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->rpcapi_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->rpcapi_ssl_en);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->rpcapi_ssl_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->rpcapi_auth_mode);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->oper_rpcapi_auth_mode);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->cut_through_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->protected_vlan_cnt);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->vlan_stats_cnt);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->mib_port_stats_read_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->mib_flow_stats_read_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->memory_threshold1);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->memory_threshold2);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->memory_threshold3);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->system_memchk_state);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->mgmt_if_running);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_sys_glb->mgmt_if_timer);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, p_sys_glb->warmboot_type));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->warmboot_time);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->warmboot_count);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->system_time_update);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->sflow_counter_ifindex);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->dhcp_server_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->add_gre_ace_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->add_vxlan_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_glb->system_product);
    }
    return str;
}

char**
tbl_sys_global_table_dump(tbl_sys_global_t *p_sys_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sys_global_key_name_dump(p_sys_glb, buf));
    for(i=1; i<TBL_SYS_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sys_global_field_name_dump(p_sys_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sys_global_field_value_dump(p_sys_glb, i, buf));
    }
    return str;
}

int32
tbl_sys_global_field_value_parser(char *str, int32 field_id, tbl_sys_global_t *p_sys_glb)
{
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HOSTNAME, field_id))
    {
        sal_strcpy(p_sys_glb->hostname, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ROUTE_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_sys_glb->route_mac, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAC_NUM, field_id))
    {
        int32 ret;
        p_sys_glb->mac_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_INIT_DONE, field_id))
    {
        int32 ret;
        p_sys_glb->init_done = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_STARTUP_DONE, field_id))
    {
        int32 ret;
        p_sys_glb->startup_done = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE, field_id))
    {
        int32 ret;
        p_sys_glb->max_frame_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE, field_id))
    {
        int32 ret;
        p_sys_glb->jumbo_frame_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_REBOOT_TYPE, field_id))
    {
        cdb_enum_str2val(glb_reboot_type_strs, GLB_REBOOT_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->ipg_shaping_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->ipg_policing_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->ipg_storm_control_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL, field_id))
    {
        int32 ret;
        p_sys_glb->aaa_new_model = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION, field_id))
    {
        int32 ret;
        p_sys_glb->service_password_encryption = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_VTY, field_id))
    {
        int32 ret;
        p_sys_glb->max_vty = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1, field_id))
    {
        int32 ret;
        p_sys_glb->aaa_privilege1 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2, field_id))
    {
        int32 ret;
        p_sys_glb->aaa_privilege2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3, field_id))
    {
        int32 ret;
        p_sys_glb->aaa_privilege3 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->dhcp_service_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->dhcp_relay_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->dhcp_snooping_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->http_service_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME, field_id))
    {
        sal_strcpy(p_sys_glb->http_image_name, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL, field_id))
    {
        int32 ret;
        p_sys_glb->errdisable_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH, field_id))
    {
        int32 ret;
        p_sys_glb->trunction_length = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA, field_id))
    {
        cdb_mac_addr_str2val(p_sys_glb->tap_ts_macda, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA, field_id))
    {
        cdb_mac_addr_str2val(p_sys_glb->tap_ts_macsa, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ETHER_TYPE, field_id))
    {
        int32 ret;
        p_sys_glb->ether_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PTF_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->ptf_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_DISABLE, field_id))
    {
        int32 ret;
        p_sys_glb->telnet_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->telnet_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_ACL, field_id))
    {
        sal_strcpy(p_sys_glb->telnet_acl, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SSH_ACL, field_id))
    {
        sal_strcpy(p_sys_glb->ssh_acl, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_DISABLE, field_id))
    {
        int32 ret;
        p_sys_glb->http_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->http_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT, field_id))
    {
        int32 ret;
        p_sys_glb->http_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE, field_id))
    {
        int32 ret;
        p_sys_glb->https_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->https_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE, field_id))
    {
        int32 ret;
        p_sys_glb->rpcapi_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->rpcapi_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN, field_id))
    {
        int32 ret;
        p_sys_glb->rpcapi_ssl_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT, field_id))
    {
        int32 ret;
        p_sys_glb->rpcapi_ssl_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE, field_id))
    {
        int32 ret;
        p_sys_glb->rpcapi_auth_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE, field_id))
    {
        int32 ret;
        p_sys_glb->oper_rpcapi_auth_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->cut_through_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT, field_id))
    {
        int32 ret;
        p_sys_glb->protected_vlan_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT, field_id))
    {
        int32 ret;
        p_sys_glb->vlan_stats_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM, field_id))
    {
        int32 ret;
        p_sys_glb->mib_port_stats_read_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM, field_id))
    {
        int32 ret;
        p_sys_glb->mib_flow_stats_read_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1, field_id))
    {
        int32 ret;
        p_sys_glb->memory_threshold1 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2, field_id))
    {
        int32 ret;
        p_sys_glb->memory_threshold2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3, field_id))
    {
        int32 ret;
        p_sys_glb->memory_threshold3 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE, field_id))
    {
        int32 ret;
        p_sys_glb->system_memchk_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING, field_id))
    {
        int32 ret;
        p_sys_glb->mgmt_if_running = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER, field_id))
    {
        ctc_task_str2val(str, p_sys_glb->mgmt_if_timer);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE, field_id))
    {
        cdb_enum_str2val(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME, field_id))
    {
        int32 ret;
        p_sys_glb->warmboot_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT, field_id))
    {
        int32 ret;
        p_sys_glb->warmboot_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE, field_id))
    {
        int32 ret;
        p_sys_glb->system_time_update = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX, field_id))
    {
        int32 ret;
        p_sys_glb->sflow_counter_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE, field_id))
    {
        int32 ret;
        p_sys_glb->dhcp_server_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_sys_glb->add_gre_ace_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_sys_glb->add_vxlan_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT, field_id))
    {
        int32 ret;
        p_sys_glb->system_product = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_sys_global_table_parser(char** array, char* key_value,tbl_sys_global_t *p_sys_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sys_global_key_str2val(key_value, p_sys_glb));

    for(i=1; i<TBL_SYS_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sys_global_field_value_parser( array[j++], i, p_sys_glb));
    }

    return PM_E_NONE;
}

int32
tbl_sys_global_dump_one(tbl_sys_global_t *p_sys_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HOSTNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HOSTNAME].name,
            p_sys_glb->hostname);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ROUTE_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_ROUTE_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_sys_glb->route_mac));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAC_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MAC_NUM].name,
            p_sys_glb->mac_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT].name,
            p_sys_glb->curr_vlanif_count);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_INIT_DONE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_INIT_DONE].name,
            p_sys_glb->init_done);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_STARTUP_DONE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_STARTUP_DONE].name,
            p_sys_glb->startup_done);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE].name,
            p_sys_glb->max_frame_size);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE].name,
            p_sys_glb->jumbo_frame_size);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_REBOOT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_REBOOT_TYPE].name, 
            cdb_enum_val2str(glb_reboot_type_strs, GLB_REBOOT_TYPE_MAX, p_sys_glb->reboot_type));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE].name,
            p_sys_glb->ipg_shaping_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE].name,
            p_sys_glb->ipg_policing_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE].name,
            p_sys_glb->ipg_storm_control_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL].name,
            p_sys_glb->aaa_new_model);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION].name,
            p_sys_glb->service_password_encryption);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MAX_VTY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MAX_VTY].name,
            p_sys_glb->max_vty);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1].name,
            p_sys_glb->aaa_privilege1);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2].name,
            p_sys_glb->aaa_privilege2);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3].name,
            p_sys_glb->aaa_privilege3);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE].name,
            p_sys_glb->dhcp_service_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE].name,
            p_sys_glb->dhcp_relay_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE].name,
            p_sys_glb->dhcp_snooping_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE].name,
            p_sys_glb->http_service_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME].name,
            p_sys_glb->http_image_name);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL].name,
            p_sys_glb->errdisable_interval);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH].name,
            p_sys_glb->trunction_length);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_sys_glb->tap_ts_macda));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_sys_glb->tap_ts_macsa));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ETHER_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_ETHER_TYPE].name,
            p_sys_glb->ether_type);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PTF_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_PTF_PORT].name,
            p_sys_glb->ptf_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TELNET_DISABLE].name,
            p_sys_glb->telnet_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TELNET_PORT].name,
            p_sys_glb->telnet_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_TELNET_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_TELNET_ACL].name,
            p_sys_glb->telnet_acl);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SSH_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SSH_ACL].name,
            p_sys_glb->ssh_acl);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTP_DISABLE].name,
            p_sys_glb->http_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTP_PORT].name,
            p_sys_glb->http_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT].name,
            p_sys_glb->http_timeout);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE].name,
            p_sys_glb->https_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_HTTPS_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_HTTPS_PORT].name,
            p_sys_glb->https_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE].name,
            p_sys_glb->rpcapi_disable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_RPCAPI_PORT].name,
            p_sys_glb->rpcapi_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN].name,
            p_sys_glb->rpcapi_ssl_en);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT].name,
            p_sys_glb->rpcapi_ssl_port);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE].name,
            p_sys_glb->rpcapi_auth_mode);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE].name,
            p_sys_glb->oper_rpcapi_auth_mode);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE].name,
            p_sys_glb->cut_through_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT].name,
            p_sys_glb->protected_vlan_cnt);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT].name,
            p_sys_glb->vlan_stats_cnt);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM].name,
            p_sys_glb->mib_port_stats_read_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM].name,
            p_sys_glb->mib_flow_stats_read_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1].name,
            p_sys_glb->memory_threshold1);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2].name,
            p_sys_glb->memory_threshold2);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3].name,
            p_sys_glb->memory_threshold3);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE].name,
            p_sys_glb->system_memchk_state);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING].name,
            p_sys_glb->mgmt_if_running);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_sys_glb->mgmt_if_timer));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE].name, 
            cdb_enum_val2str(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, p_sys_glb->warmboot_type));
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME].name,
            p_sys_glb->warmboot_time);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT].name,
            p_sys_glb->warmboot_count);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE].name,
            p_sys_glb->system_time_update);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX].name,
            p_sys_glb->sflow_counter_ifindex);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE].name,
            p_sys_glb->dhcp_server_enable);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM].name,
            p_sys_glb->add_gre_ace_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM].name,
            p_sys_glb->add_vxlan_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT].name,
            p_sys_glb->system_product);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sys_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sys_global_t *p_db_sys_glb = _g_p_tbl_sys_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_sys_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_sys_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sys_global_t*
tbl_sys_global_init_sys_global()
{
    _g_p_tbl_sys_global = XCALLOC(MEM_TBL_SYS_GLOBAL, sizeof(tbl_sys_global_t));
    return _g_p_tbl_sys_global;
}



#include "proto.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include "cdb_data_cmp.h"

tbl_openflow_t *_g_p_tbl_openflow = NULL;

int32
tbl_openflow_set_openflow_field_sync(tbl_openflow_t *p_openflow, tbl_openflow_field_id_t field_id, uint32 sync)
{
    tbl_openflow_t *p_db_openflow = _g_p_tbl_openflow;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_FLD_GROUP_HASH_KEY:
        p_db_openflow->group_hash_key = p_openflow->group_hash_key;
        break;

    case TBL_OPENFLOW_FLD_BOND_HASH_KEY:
        p_db_openflow->bond_hash_key = p_openflow->bond_hash_key;
        break;

    case TBL_OPENFLOW_FLD_BOND_HASH_USE:
        p_db_openflow->bond_hash_use = p_openflow->bond_hash_use;
        break;

    case TBL_OPENFLOW_FLD_LACP_LOCAL_PROC:
        p_db_openflow->lacp_local_proc = p_openflow->lacp_local_proc;
        break;

    case TBL_OPENFLOW_FLD_LOCAL_VTEP_IP:
        sal_memcpy(&p_db_openflow->local_vtep_ip, &p_openflow->local_vtep_ip, sizeof(p_openflow->local_vtep_ip));
        break;

    case TBL_OPENFLOW_FLD_DECAP_MODE:
        sal_memcpy(p_db_openflow->decap_mode, p_openflow->decap_mode, sizeof(p_openflow->decap_mode));
        break;

    case TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT:
        p_db_openflow->flow_drop_pkt_to_ingress_port = p_openflow->flow_drop_pkt_to_ingress_port;
        break;

    case TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT:
        p_db_openflow->group_drop_pkt_to_ingress_port = p_openflow->group_drop_pkt_to_ingress_port;
        break;

    case TBL_OPENFLOW_FLD_UDF_PARSER:
        p_db_openflow->udf_parser = p_openflow->udf_parser;
        break;

    case TBL_OPENFLOW_FLD_METER_IPG:
        p_db_openflow->meter_ipg = p_openflow->meter_ipg;
        break;

    case TBL_OPENFLOW_FLD_INBAND_EN:
        p_db_openflow->inband_en = p_openflow->inband_en;
        break;

    case TBL_OPENFLOW_FLD_PTP_E2E_EN:
        p_db_openflow->ptp_e2e_en = p_openflow->ptp_e2e_en;
        break;

    case TBL_OPENFLOW_FLD_INBAND_STAG:
        p_db_openflow->inband_stag = p_openflow->inband_stag;
        break;

    case TBL_OPENFLOW_FLD_INBAND_ADDR:
        sal_memcpy(&p_db_openflow->inband_addr, &p_openflow->inband_addr, sizeof(p_openflow->inband_addr));
        break;

    case TBL_OPENFLOW_FLD_INBAND_GW:
        sal_memcpy(&p_db_openflow->inband_gw, &p_openflow->inband_gw, sizeof(p_openflow->inband_gw));
        break;

    case TBL_OPENFLOW_FLD_MPLS_PARSER:
        p_db_openflow->mpls_parser = p_openflow->mpls_parser;
        break;

    case TBL_OPENFLOW_FLD_STPID:
        p_db_openflow->stpid = p_openflow->stpid;
        break;

    case TBL_OPENFLOW_FLD_CTPID:
        p_db_openflow->ctpid = p_openflow->ctpid;
        break;

    case TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP:
        sal_memcpy(p_db_openflow->inband_downlink_port_bmp, p_openflow->inband_downlink_port_bmp, sizeof(p_openflow->inband_downlink_port_bmp));
        break;

    case TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME:
        sal_memcpy(p_db_openflow->inband_uplink_port_name, p_openflow->inband_uplink_port_name, sizeof(p_openflow->inband_uplink_port_name));
        break;

    case TBL_OPENFLOW_FLD_FLOW_HASH_KEY:
        p_db_openflow->flow_hash_key = p_openflow->flow_hash_key;
        break;

    case TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN:
        p_db_openflow->vxlan_hash_merge_en = p_openflow->vxlan_hash_merge_en;
        break;

    case TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN:
        p_db_openflow->nvgre_hash_merge_en = p_openflow->nvgre_hash_merge_en;
        break;

    case TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE:
        p_db_openflow->efd_tcp_only_enable = p_openflow->efd_tcp_only_enable;
        break;

    case TBL_OPENFLOW_FLD_EFD_GRANULARITY:
        p_db_openflow->efd_granularity = p_openflow->efd_granularity;
        break;

    case TBL_OPENFLOW_FLD_EFD_DETECT_SPEED:
        p_db_openflow->efd_detect_speed = p_openflow->efd_detect_speed;
        break;

    case TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL:
        p_db_openflow->efd_detect_time_interval = p_openflow->efd_detect_time_interval;
        break;

    case TBL_OPENFLOW_FLD_EFD_AGING_TIMER:
        p_db_openflow->efd_aging_timer = p_openflow->efd_aging_timer;
        break;

    case TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS:
        p_db_openflow->efd_flow_traffic_class = p_openflow->efd_flow_traffic_class;
        break;

    case TBL_OPENFLOW_FLD_EFD_FLOW_COLOR:
        p_db_openflow->efd_flow_color = p_openflow->efd_flow_color;
        break;

    case TBL_OPENFLOW_FLD_EFD_IPG_ENABLE:
        p_db_openflow->efd_ipg_enable = p_openflow->efd_ipg_enable;
        break;

    case TBL_OPENFLOW_FLD_DPID:
        sal_memcpy(p_db_openflow->dpid, p_openflow->dpid, sizeof(p_openflow->dpid));
        break;

    case TBL_OPENFLOW_FLD_DPID_IS_SET:
        p_db_openflow->dpid_is_set = p_openflow->dpid_is_set;
        break;

    case TBL_OPENFLOW_FLD_PRIVATE_KEY:
        sal_memcpy(p_db_openflow->private_key, p_openflow->private_key, sizeof(p_openflow->private_key));
        break;

    case TBL_OPENFLOW_FLD_CERTIFICATE:
        sal_memcpy(p_db_openflow->certificate, p_openflow->certificate, sizeof(p_openflow->certificate));
        break;

    case TBL_OPENFLOW_FLD_CA_CERT:
        sal_memcpy(p_db_openflow->ca_cert, p_openflow->ca_cert, sizeof(p_openflow->ca_cert));
        break;

    case TBL_OPENFLOW_FLD_VXLAN_SRCPORT:
        p_db_openflow->vxlan_srcport = p_openflow->vxlan_srcport;
        break;

    case TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET:
        p_db_openflow->vxlan_srcport_is_set = p_openflow->vxlan_srcport_is_set;
        break;

    case TBL_OPENFLOW_FLD_VXLAN_DESTPORT:
        p_db_openflow->vxlan_destport = p_openflow->vxlan_destport;
        break;

    case TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET:
        p_db_openflow->vxlan_destport_is_set = p_openflow->vxlan_destport_is_set;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL:
        p_db_openflow->tp_oam_oam_interval = p_openflow->tp_oam_oam_interval;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET:
        p_db_openflow->tp_oam_oam_interval_is_set = p_openflow->tp_oam_oam_interval_is_set;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE:
        p_db_openflow->tp_oam_oam_cycle_is_disable = p_openflow->tp_oam_oam_cycle_is_disable;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL:
        p_db_openflow->tp_oam_aps_interval = p_openflow->tp_oam_aps_interval;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET:
        p_db_openflow->tp_oam_aps_interval_is_set = p_openflow->tp_oam_aps_interval_is_set;
        break;

    case TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE:
        p_db_openflow->tp_oam_aps_cycle_is_disable = p_openflow->tp_oam_aps_cycle_is_disable;
        break;

    case TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE:
        p_db_openflow->pw_not_standard_mode = p_openflow->pw_not_standard_mode;
        break;

    case TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM:
        p_db_openflow->check_port_bind_oam = p_openflow->check_port_bind_oam;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT:
        p_db_openflow->oam_packet_in_cycle_cnt = p_openflow->oam_packet_in_cycle_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT:
        p_db_openflow->oam_packet_in_event_ccm_cnt = p_openflow->oam_packet_in_event_ccm_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT:
        p_db_openflow->oam_packet_in_event_lm_cnt = p_openflow->oam_packet_in_event_lm_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT:
        p_db_openflow->oam_packet_in_event_dm_cnt = p_openflow->oam_packet_in_event_dm_cnt;
        break;

    case TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT:
        p_db_openflow->aps_packet_in_cycle_cnt = p_openflow->aps_packet_in_cycle_cnt;
        break;

    case TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT:
        p_db_openflow->aps_packet_in_event_path_change_cnt = p_openflow->aps_packet_in_event_path_change_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT:
        p_db_openflow->oam_packet_in_cycle_packet_cnt = p_openflow->oam_packet_in_cycle_packet_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT:
        p_db_openflow->oam_packet_in_event_packet_ccm_cnt = p_openflow->oam_packet_in_event_packet_ccm_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT:
        p_db_openflow->oam_packet_in_event_packet_lm_cnt = p_openflow->oam_packet_in_event_packet_lm_cnt;
        break;

    case TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT:
        p_db_openflow->oam_packet_in_event_packet_dm_cnt = p_openflow->oam_packet_in_event_packet_dm_cnt;
        break;

    case TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT:
        p_db_openflow->aps_packet_in_cycle_packet_cnt = p_openflow->aps_packet_in_cycle_packet_cnt;
        break;

    case TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT:
        p_db_openflow->aps_packet_in_event_path_change_packet_cnt = p_openflow->aps_packet_in_event_path_change_packet_cnt;
        break;

    case TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE:
        p_db_openflow->openflow_mac_learning_enable = p_openflow->openflow_mac_learning_enable;
        break;

    case TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE:
        p_db_openflow->controller_affect_flow_disable = p_openflow->controller_affect_flow_disable;
        break;

    case TBL_OPENFLOW_FLD_ASYNC_MSGID_EN:
        p_db_openflow->async_msgid_en = p_openflow->async_msgid_en;
        break;

    case TBL_OPENFLOW_FLD_FDB_FLOW_EN:
        p_db_openflow->fdb_flow_en = p_openflow->fdb_flow_en;
        break;

    case TBL_OPENFLOW_FLD_FDB_FLOW_VALID:
        p_db_openflow->fdb_flow_valid = p_openflow->fdb_flow_valid;
        break;

    case TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN:
        p_db_openflow->ffgroup_switchback_en = p_openflow->ffgroup_switchback_en;
        break;

    case TBL_OPENFLOW_FLD_MAX:
        sal_memcpy(p_db_openflow, p_openflow, sizeof(tbl_openflow_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW, field_id, p_db_openflow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW, field_id, p_db_openflow);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_set_openflow_field(tbl_openflow_t *p_openflow, tbl_openflow_field_id_t field_id)
{
    return tbl_openflow_set_openflow_field_sync(p_openflow, field_id, TRUE);
}

tbl_openflow_t*
tbl_openflow_get_openflow()
{
    return _g_p_tbl_openflow;
}

int32
tbl_openflow_key_str2val(char *str, tbl_openflow_t *p_openflow)
{
    return PM_E_NONE;
}

char*
tbl_openflow_key_name_dump(tbl_openflow_t *p_openflow, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_openflow_key_value_dump(tbl_openflow_t *p_openflow, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_openflow_field_name_dump(tbl_openflow_t *p_openflow, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_field_value_dump(tbl_openflow_t *p_openflow, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_HASH_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->group_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->bond_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_USE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->bond_hash_use);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LACP_LOCAL_PROC, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->lacp_local_proc);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LOCAL_VTEP_IP, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_openflow->local_vtep_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DECAP_MODE, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->decap_mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->flow_drop_pkt_to_ingress_port);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->group_drop_pkt_to_ingress_port);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_UDF_PARSER, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->udf_parser);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_METER_IPG, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->meter_ipg);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->inband_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PTP_E2E_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->ptp_e2e_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_STAG, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->inband_stag);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_ADDR, field_id))
    {
        cdb_prefix_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_openflow->inband_addr);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_GW, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_openflow->inband_gw);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_MPLS_PARSER, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->mpls_parser);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_STPID, field_id))
    {
        sal_sprintf(str, "%d", p_openflow->stpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CTPID, field_id))
    {
        sal_sprintf(str, "%d", p_openflow->ctpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_openflow->inband_downlink_port_bmp, sizeof(p_openflow->inband_downlink_port_bmp)/4);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->inband_uplink_port_name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_HASH_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->flow_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->vxlan_hash_merge_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->nvgre_hash_merge_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_tcp_only_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_GRANULARITY, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_granularity);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_SPEED, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_detect_speed);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_detect_time_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_AGING_TIMER, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_aging_timer);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_flow_traffic_class);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_COLOR, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_flow_color);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_IPG_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->efd_ipg_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->dpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->dpid_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PRIVATE_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->private_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CERTIFICATE, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->certificate);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CA_CERT, field_id))
    {
        sal_sprintf(str, "%s", p_openflow->ca_cert);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->vxlan_srcport_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->vxlan_destport);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->vxlan_destport_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_oam_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_oam_interval_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_oam_cycle_is_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_aps_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_aps_interval_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->tp_oam_aps_cycle_is_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->pw_not_standard_mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->check_port_bind_oam);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_cycle_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_ccm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_lm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_dm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->aps_packet_in_cycle_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->aps_packet_in_event_path_change_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_cycle_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_packet_ccm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_packet_lm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->oam_packet_in_event_packet_dm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->aps_packet_in_cycle_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->aps_packet_in_event_path_change_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->openflow_mac_learning_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->controller_affect_flow_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_ASYNC_MSGID_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->async_msgid_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->fdb_flow_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_VALID, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->fdb_flow_valid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->ffgroup_switchback_en);
    }
    return str;
}

char**
tbl_openflow_table_dump(tbl_openflow_t *p_openflow, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_key_name_dump(p_openflow, buf));
    for(i=1; i<TBL_OPENFLOW_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_field_name_dump(p_openflow, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_field_value_dump(p_openflow, i, buf));
    }
    return str;
}

int32
tbl_openflow_field_value_parser(char *str, int32 field_id, tbl_openflow_t *p_openflow)
{
    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_HASH_KEY, field_id))
    {
        int32 ret;
        p_openflow->group_hash_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_KEY, field_id))
    {
        int32 ret;
        p_openflow->bond_hash_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_USE, field_id))
    {
        int32 ret;
        p_openflow->bond_hash_use = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LACP_LOCAL_PROC, field_id))
    {
        int32 ret;
        p_openflow->lacp_local_proc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LOCAL_VTEP_IP, field_id))
    {
        cdb_addr_ipv4_str2val(&p_openflow->local_vtep_ip, str, 0);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DECAP_MODE, field_id))
    {
        sal_strcpy(p_openflow->decap_mode, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT, field_id))
    {
        int32 ret;
        p_openflow->flow_drop_pkt_to_ingress_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT, field_id))
    {
        int32 ret;
        p_openflow->group_drop_pkt_to_ingress_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_UDF_PARSER, field_id))
    {
        int32 ret;
        p_openflow->udf_parser = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_METER_IPG, field_id))
    {
        int32 ret;
        p_openflow->meter_ipg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_EN, field_id))
    {
        int32 ret;
        p_openflow->inband_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PTP_E2E_EN, field_id))
    {
        int32 ret;
        p_openflow->ptp_e2e_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_STAG, field_id))
    {
        int32 ret;
        p_openflow->inband_stag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_ADDR, field_id))
    {
        cdb_prefix_ipv4_str2val_wo_masklen(str, &p_openflow->inband_addr);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_GW, field_id))
    {
        cdb_addr_ipv4_str2val(&p_openflow->inband_gw, str, 0);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_MPLS_PARSER, field_id))
    {
        int32 ret;
        p_openflow->mpls_parser = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_STPID, field_id))
    {
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CTPID, field_id))
    {
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_openflow->inband_downlink_port_bmp, sizeof(p_openflow->inband_downlink_port_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME, field_id))
    {
        sal_strcpy(p_openflow->inband_uplink_port_name, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_HASH_KEY, field_id))
    {
        int32 ret;
        p_openflow->flow_hash_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN, field_id))
    {
        int32 ret;
        p_openflow->vxlan_hash_merge_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN, field_id))
    {
        int32 ret;
        p_openflow->nvgre_hash_merge_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE, field_id))
    {
        int32 ret;
        p_openflow->efd_tcp_only_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_GRANULARITY, field_id))
    {
        int32 ret;
        p_openflow->efd_granularity = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_SPEED, field_id))
    {
        int32 ret;
        p_openflow->efd_detect_speed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL, field_id))
    {
        int32 ret;
        p_openflow->efd_detect_time_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_AGING_TIMER, field_id))
    {
        int32 ret;
        p_openflow->efd_aging_timer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS, field_id))
    {
        int32 ret;
        p_openflow->efd_flow_traffic_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_COLOR, field_id))
    {
        int32 ret;
        p_openflow->efd_flow_color = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_IPG_ENABLE, field_id))
    {
        int32 ret;
        p_openflow->efd_ipg_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID, field_id))
    {
        sal_strcpy(p_openflow->dpid, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID_IS_SET, field_id))
    {
        int32 ret;
        p_openflow->dpid_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PRIVATE_KEY, field_id))
    {
        sal_strcpy(p_openflow->private_key, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CERTIFICATE, field_id))
    {
        sal_strcpy(p_openflow->certificate, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CA_CERT, field_id))
    {
        sal_strcpy(p_openflow->ca_cert, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT, field_id))
    {
        int32 ret;
        p_openflow->vxlan_srcport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET, field_id))
    {
        int32 ret;
        p_openflow->vxlan_srcport_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT, field_id))
    {
        int32 ret;
        p_openflow->vxlan_destport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET, field_id))
    {
        int32 ret;
        p_openflow->vxlan_destport_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_oam_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_oam_interval_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_oam_cycle_is_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_aps_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_aps_interval_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE, field_id))
    {
        int32 ret;
        p_openflow->tp_oam_aps_cycle_is_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE, field_id))
    {
        int32 ret;
        p_openflow->pw_not_standard_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM, field_id))
    {
        int32 ret;
        p_openflow->check_port_bind_oam = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_cycle_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_ccm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_lm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_dm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT, field_id))
    {
        int32 ret;
        p_openflow->aps_packet_in_cycle_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT, field_id))
    {
        int32 ret;
        p_openflow->aps_packet_in_event_path_change_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_cycle_packet_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_packet_ccm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_packet_lm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT, field_id))
    {
        int32 ret;
        p_openflow->oam_packet_in_event_packet_dm_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT, field_id))
    {
        int32 ret;
        p_openflow->aps_packet_in_cycle_packet_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT, field_id))
    {
        int32 ret;
        p_openflow->aps_packet_in_event_path_change_packet_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE, field_id))
    {
        int32 ret;
        p_openflow->openflow_mac_learning_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE, field_id))
    {
        int32 ret;
        p_openflow->controller_affect_flow_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_ASYNC_MSGID_EN, field_id))
    {
        int32 ret;
        p_openflow->async_msgid_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_EN, field_id))
    {
        int32 ret;
        p_openflow->fdb_flow_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_VALID, field_id))
    {
        int32 ret;
        p_openflow->fdb_flow_valid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN, field_id))
    {
        int32 ret;
        p_openflow->ffgroup_switchback_en = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_table_parser(char** array, char* key_value,tbl_openflow_t *p_openflow)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_key_str2val(key_value, p_openflow));

    for(i=1; i<TBL_OPENFLOW_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_field_value_parser( array[j++], i, p_openflow));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_dump_one(tbl_openflow_t *p_openflow, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_HASH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_GROUP_HASH_KEY].name,
            p_openflow->group_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_BOND_HASH_KEY].name,
            p_openflow->bond_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_BOND_HASH_USE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_BOND_HASH_USE].name,
            p_openflow->bond_hash_use);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LACP_LOCAL_PROC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_LACP_LOCAL_PROC].name,
            p_openflow->lacp_local_proc);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_LOCAL_VTEP_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_LOCAL_VTEP_IP].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_openflow->local_vtep_ip));
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DECAP_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_DECAP_MODE].name,
            p_openflow->decap_mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_FLOW_DROP_PKT_TO_INGRESS_PORT].name,
            p_openflow->flow_drop_pkt_to_ingress_port);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_GROUP_DROP_PKT_TO_INGRESS_PORT].name,
            p_openflow->group_drop_pkt_to_ingress_port);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_UDF_PARSER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_UDF_PARSER].name,
            p_openflow->udf_parser);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_METER_IPG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_METER_IPG].name,
            p_openflow->meter_ipg);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_EN].name,
            p_openflow->inband_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PTP_E2E_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_PTP_E2E_EN].name,
            p_openflow->ptp_e2e_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_STAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_STAG].name,
            p_openflow->inband_stag);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_ADDR].name, 
            cdb_prefix_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_openflow->inband_addr));
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_GW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_GW].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_openflow->inband_gw));
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_MPLS_PARSER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_MPLS_PARSER].name,
            p_openflow->mpls_parser);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_STPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_OPENFLOW_FLD_STPID].name,
            p_openflow->stpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CTPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_OPENFLOW_FLD_CTPID].name,
            p_openflow->ctpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_DOWNLINK_PORT_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_openflow->inband_downlink_port_bmp, sizeof(p_openflow->inband_downlink_port_bmp)/4));
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_INBAND_UPLINK_PORT_NAME].name,
            p_openflow->inband_uplink_port_name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FLOW_HASH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_FLOW_HASH_KEY].name,
            p_openflow->flow_hash_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_VXLAN_HASH_MERGE_EN].name,
            p_openflow->vxlan_hash_merge_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_NVGRE_HASH_MERGE_EN].name,
            p_openflow->nvgre_hash_merge_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_TCP_ONLY_ENABLE].name,
            p_openflow->efd_tcp_only_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_GRANULARITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_GRANULARITY].name,
            p_openflow->efd_granularity);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_DETECT_SPEED].name,
            p_openflow->efd_detect_speed);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_DETECT_TIME_INTERVAL].name,
            p_openflow->efd_detect_time_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_AGING_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_AGING_TIMER].name,
            p_openflow->efd_aging_timer);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_FLOW_TRAFFIC_CLASS].name,
            p_openflow->efd_flow_traffic_class);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_FLOW_COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_FLOW_COLOR].name,
            p_openflow->efd_flow_color);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_EFD_IPG_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_EFD_IPG_ENABLE].name,
            p_openflow->efd_ipg_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_DPID].name,
            p_openflow->dpid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_DPID_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_DPID_IS_SET].name,
            p_openflow->dpid_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PRIVATE_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_PRIVATE_KEY].name,
            p_openflow->private_key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CERTIFICATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_CERTIFICATE].name,
            p_openflow->certificate);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CA_CERT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_FLD_CA_CERT].name,
            p_openflow->ca_cert);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_VXLAN_SRCPORT].name,
            p_openflow->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_VXLAN_SRCPORT_IS_SET].name,
            p_openflow->vxlan_srcport_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_VXLAN_DESTPORT].name,
            p_openflow->vxlan_destport);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET].name,
            p_openflow->vxlan_destport_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL].name,
            p_openflow->tp_oam_oam_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_OAM_INTERVAL_IS_SET].name,
            p_openflow->tp_oam_oam_interval_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_OAM_CYCLE_IS_DISABLE].name,
            p_openflow->tp_oam_oam_cycle_is_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL].name,
            p_openflow->tp_oam_aps_interval);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_APS_INTERVAL_IS_SET].name,
            p_openflow->tp_oam_aps_interval_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_TP_OAM_APS_CYCLE_IS_DISABLE].name,
            p_openflow->tp_oam_aps_cycle_is_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_PW_NOT_STANDARD_MODE].name,
            p_openflow->pw_not_standard_mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_CHECK_PORT_BIND_OAM].name,
            p_openflow->check_port_bind_oam);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_CNT].name,
            p_openflow->oam_packet_in_cycle_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_CCM_CNT].name,
            p_openflow->oam_packet_in_event_ccm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_LM_CNT].name,
            p_openflow->oam_packet_in_event_lm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_DM_CNT].name,
            p_openflow->oam_packet_in_event_dm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_CNT].name,
            p_openflow->aps_packet_in_cycle_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_CNT].name,
            p_openflow->aps_packet_in_event_path_change_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_CYCLE_PACKET_CNT].name,
            p_openflow->oam_packet_in_cycle_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_CCM_CNT].name,
            p_openflow->oam_packet_in_event_packet_ccm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_LM_CNT].name,
            p_openflow->oam_packet_in_event_packet_lm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OAM_PACKET_IN_EVENT_PACKET_DM_CNT].name,
            p_openflow->oam_packet_in_event_packet_dm_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_APS_PACKET_IN_CYCLE_PACKET_CNT].name,
            p_openflow->aps_packet_in_cycle_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_APS_PACKET_IN_EVENT_PATH_CHANGE_PACKET_CNT].name,
            p_openflow->aps_packet_in_event_path_change_packet_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_OPENFLOW_MAC_LEARNING_ENABLE].name,
            p_openflow->openflow_mac_learning_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_CONTROLLER_AFFECT_FLOW_DISABLE].name,
            p_openflow->controller_affect_flow_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_ASYNC_MSGID_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_ASYNC_MSGID_EN].name,
            p_openflow->async_msgid_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_FDB_FLOW_EN].name,
            p_openflow->fdb_flow_en);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FDB_FLOW_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_FDB_FLOW_VALID].name,
            p_openflow->fdb_flow_valid);
    }
    if (FLD_MATCH(TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN].name,
            p_openflow->ffgroup_switchback_en);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_t *p_db_openflow = _g_p_tbl_openflow;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_openflow)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_openflow, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_openflow_t*
tbl_openflow_init_openflow()
{
    _g_p_tbl_openflow = XCALLOC(MEM_TBL_OPENFLOW, sizeof(tbl_openflow_t));
    return _g_p_tbl_openflow;
}



#include "proto.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "cdb_data_cmp.h"

tbl_mstp_global_t *_g_p_tbl_mstp_global = NULL;

#include "mstp_define.h"

int32
tbl_mstp_global_set_mstp_global_field_sync(tbl_mstp_global_t *p_mstp_glb, tbl_mstp_global_field_id_t field_id, uint32 sync)
{
    tbl_mstp_global_t *p_db_mstp_glb = _g_p_tbl_mstp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MSTP_GLOBAL_FLD_TYPE:
        p_db_mstp_glb->type = p_mstp_glb->type;
        break;

    case TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG:
        p_db_mstp_glb->no_mst_config = p_mstp_glb->no_mst_config;
        break;

    case TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST:
        p_db_mstp_glb->external_root_path_cost = p_mstp_glb->external_root_path_cost;
        break;

    case TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST:
        p_db_mstp_glb->internal_root_path_cost = p_mstp_glb->internal_root_path_cost;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_PORTID:
        p_db_mstp_glb->root_portId = p_mstp_glb->root_portId;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID:
        sal_memcpy(&p_db_mstp_glb->root_rootId, &p_mstp_glb->root_rootId, sizeof(p_mstp_glb->root_rootId));
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST:
        p_db_mstp_glb->root_rootPathCost = p_mstp_glb->root_rootPathCost;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID:
        sal_memcpy(&p_db_mstp_glb->root_regionRootId, &p_mstp_glb->root_regionRootId, sizeof(p_mstp_glb->root_regionRootId));
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST:
        p_db_mstp_glb->root_intRootPathCost = p_mstp_glb->root_intRootPathCost;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_mstp_glb->root_designatedBridgeId, &p_mstp_glb->root_designatedBridgeId, sizeof(p_mstp_glb->root_designatedBridgeId));
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID:
        p_db_mstp_glb->root_designatedPortId = p_mstp_glb->root_designatedPortId;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID:
        p_db_mstp_glb->root_bridgePortId = p_mstp_glb->root_bridgePortId;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE:
        p_db_mstp_glb->root_messageAge = p_mstp_glb->root_messageAge;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY:
        p_db_mstp_glb->root_fwdDelay = p_mstp_glb->root_fwdDelay;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME:
        p_db_mstp_glb->root_helloTime = p_mstp_glb->root_helloTime;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE:
        p_db_mstp_glb->root_maxAge = p_mstp_glb->root_maxAge;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY:
        p_db_mstp_glb->bridge_fwdDelay = p_mstp_glb->bridge_fwdDelay;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME:
        p_db_mstp_glb->bridge_helloTime = p_mstp_glb->bridge_helloTime;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE:
        p_db_mstp_glb->bridge_maxAge = p_mstp_glb->bridge_maxAge;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS:
        p_db_mstp_glb->bridge_maxHops = p_mstp_glb->bridge_maxHops;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY:
        p_db_mstp_glb->bridge_priority = p_mstp_glb->bridge_priority;
        break;

    case TBL_MSTP_GLOBAL_FLD_BRIDGE_ID:
        sal_memcpy(&p_db_mstp_glb->bridge_id, &p_mstp_glb->bridge_id, sizeof(p_mstp_glb->bridge_id));
        break;

    case TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME:
        sal_memcpy(&p_db_mstp_glb->last_tc_time, &p_mstp_glb->last_tc_time, sizeof(p_mstp_glb->last_tc_time));
        break;

    case TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES:
        p_db_mstp_glb->num_topo_changes = p_mstp_glb->num_topo_changes;
        break;

    case TBL_MSTP_GLOBAL_FLD_MLAG_ROLE:
        p_db_mstp_glb->mlag_role = p_mstp_glb->mlag_role;
        break;

    case TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE:
        p_db_mstp_glb->mlag_enable = p_mstp_glb->mlag_enable;
        break;

    case TBL_MSTP_GLOBAL_FLD_ENABLE:
        p_db_mstp_glb->enable = p_mstp_glb->enable;
        break;

    case TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED:
        p_db_mstp_glb->topology_change_detected = p_mstp_glb->topology_change_detected;
        break;

    case TBL_MSTP_GLOBAL_FLD_BPDUGUARD:
        p_db_mstp_glb->bpduguard = p_mstp_glb->bpduguard;
        break;

    case TBL_MSTP_GLOBAL_FLD_BPDUFILTER:
        p_db_mstp_glb->bpdufilter = p_mstp_glb->bpdufilter;
        break;

    case TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO:
        p_db_mstp_glb->admin_cisco = p_mstp_glb->admin_cisco;
        break;

    case TBL_MSTP_GLOBAL_FLD_OPER_CISCO:
        p_db_mstp_glb->oper_cisco = p_mstp_glb->oper_cisco;
        break;

    case TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT:
        p_db_mstp_glb->txHoldCount = p_mstp_glb->txHoldCount;
        break;

    case TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD:
        p_db_mstp_glb->pathcost_standard = p_mstp_glb->pathcost_standard;
        break;

    case TBL_MSTP_GLOBAL_FLD_TC_PROTECTION:
        p_db_mstp_glb->tc_protection = p_mstp_glb->tc_protection;
        break;

    case TBL_MSTP_GLOBAL_FLD_HOPCOUNT:
        p_db_mstp_glb->hopCount = p_mstp_glb->hopCount;
        break;

    case TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD:
        p_db_mstp_glb->recv_tcn_threshold = p_mstp_glb->recv_tcn_threshold;
        break;

    case TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER:
        p_db_mstp_glb->recv_tcn_number = p_mstp_glb->recv_tcn_number;
        break;

    case TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX:
        p_db_mstp_glb->root_port_ifindex = p_mstp_glb->root_port_ifindex;
        break;

    case TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX:
        p_db_mstp_glb->alternate_port_ifndex = p_mstp_glb->alternate_port_ifndex;
        break;

    case TBL_MSTP_GLOBAL_FLD_PRS_STATE:
        p_db_mstp_glb->prs_state = p_mstp_glb->prs_state;
        break;

    case TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID:
        p_db_mstp_glb->mstp_format_id = p_mstp_glb->mstp_format_id;
        break;

    case TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL:
        p_db_mstp_glb->mstp_revision_level = p_mstp_glb->mstp_revision_level;
        break;

    case TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME:
        sal_memcpy(p_db_mstp_glb->mstp_region_name, p_mstp_glb->mstp_region_name, sizeof(p_mstp_glb->mstp_region_name));
        break;

    case TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST:
        sal_memcpy(&p_db_mstp_glb->mstp_digest, &p_mstp_glb->mstp_digest, sizeof(p_mstp_glb->mstp_digest));
        break;

    case TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER:
        p_db_mstp_glb->tc_protection_timer = p_mstp_glb->tc_protection_timer;
        break;

    case TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN:
        p_db_mstp_glb->of_exclude_all_vlan = p_mstp_glb->of_exclude_all_vlan;
        break;

    case TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP:
        sal_memcpy(p_db_mstp_glb->of_include_vlan_bmp, p_mstp_glb->of_include_vlan_bmp, sizeof(p_mstp_glb->of_include_vlan_bmp));
        break;

    case TBL_MSTP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_mstp_glb, p_mstp_glb, sizeof(tbl_mstp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MSTP_GLOBAL, field_id, p_db_mstp_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MSTP_GLOBAL, field_id, p_db_mstp_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mstp_global_set_mstp_global_field(tbl_mstp_global_t *p_mstp_glb, tbl_mstp_global_field_id_t field_id)
{
    return tbl_mstp_global_set_mstp_global_field_sync(p_mstp_glb, field_id, TRUE);
}

tbl_mstp_global_t*
tbl_mstp_global_get_mstp_global()
{
    return _g_p_tbl_mstp_global;
}

int32
tbl_mstp_global_key_str2val(char *str, tbl_mstp_global_t *p_mstp_glb)
{
    return PM_E_NONE;
}

char*
tbl_mstp_global_key_name_dump(tbl_mstp_global_t *p_mstp_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_mstp_global_key_value_dump(tbl_mstp_global_t *p_mstp_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_mstp_global_field_name_dump(tbl_mstp_global_t *p_mstp_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MSTP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mstp_global_field_value_dump(tbl_mstp_global_t *p_mstp_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_type_strs, MSTP_TYPE_MAX, p_mstp_glb->type));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->no_mst_config);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->external_root_path_cost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->internal_root_path_cost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_portId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->root_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->root_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->root_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bridge_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bridge_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bridge_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bridge_maxHops);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bridge_priority);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_ID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->bridge_id);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->last_tc_time);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->num_topo_changes);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mlag_role_strs, MLAG_ROLE_MAX, p_mstp_glb->mlag_role));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->mlag_enable);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->enable);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->topology_change_detected);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bpduguard);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUFILTER, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->bpdufilter);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->admin_cisco);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OPER_CISCO, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->oper_cisco);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->txHoldCount);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_path_cost_strs, MSTP_PATH_COST_MAX, p_mstp_glb->pathcost_standard));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->tc_protection);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_HOPCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->recv_tcn_threshold);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->recv_tcn_number);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->root_port_ifindex);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->alternate_port_ifndex);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PRS_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, p_mstp_glb->prs_state));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->mstp_format_id);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->mstp_revision_level);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_mstp_glb->mstp_region_name);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST, field_id))
    {
        cdb_mstp_digest_val2str(str, MAX_CMD_STR_LEN, &p_mstp_glb->mstp_digest);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_glb->tc_protection_timer);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_glb->of_exclude_all_vlan);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mstp_glb->of_include_vlan_bmp, sizeof(p_mstp_glb->of_include_vlan_bmp)/4);
    }
    return str;
}

char**
tbl_mstp_global_table_dump(tbl_mstp_global_t *p_mstp_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mstp_global_key_name_dump(p_mstp_glb, buf));
    for(i=1; i<TBL_MSTP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mstp_global_field_name_dump(p_mstp_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mstp_global_field_value_dump(p_mstp_glb, i, buf));
    }
    return str;
}

int32
tbl_mstp_global_field_value_parser(char *str, int32 field_id, tbl_mstp_global_t *p_mstp_glb)
{
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(mstp_type_strs, MSTP_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG, field_id))
    {
        int32 ret;
        p_mstp_glb->no_mst_config = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST, field_id))
    {
        int32 ret;
        p_mstp_glb->external_root_path_cost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST, field_id))
    {
        int32 ret;
        p_mstp_glb->internal_root_path_cost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORTID, field_id))
    {
        int32 ret;
        p_mstp_glb->root_portId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_glb->root_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_glb->root_rootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_glb->root_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_glb->root_intRootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_glb->root_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_mstp_glb->root_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_mstp_glb->root_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_mstp_glb->root_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY, field_id))
    {
        int32 ret;
        p_mstp_glb->root_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME, field_id))
    {
        int32 ret;
        p_mstp_glb->root_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE, field_id))
    {
        int32 ret;
        p_mstp_glb->root_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY, field_id))
    {
        int32 ret;
        p_mstp_glb->bridge_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME, field_id))
    {
        int32 ret;
        p_mstp_glb->bridge_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE, field_id))
    {
        int32 ret;
        p_mstp_glb->bridge_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS, field_id))
    {
        int32 ret;
        p_mstp_glb->bridge_maxHops = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY, field_id))
    {
        int32 ret;
        p_mstp_glb->bridge_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_ID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_glb->bridge_id);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_mstp_glb->last_tc_time);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES, field_id))
    {
        int32 ret;
        p_mstp_glb->num_topo_changes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ROLE, field_id))
    {
        cdb_enum_str2val(mlag_role_strs, MLAG_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE, field_id))
    {
        int32 ret;
        p_mstp_glb->mlag_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_mstp_glb->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED, field_id))
    {
        int32 ret;
        p_mstp_glb->topology_change_detected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUGUARD, field_id))
    {
        int32 ret;
        p_mstp_glb->bpduguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUFILTER, field_id))
    {
        int32 ret;
        p_mstp_glb->bpdufilter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO, field_id))
    {
        int32 ret;
        p_mstp_glb->admin_cisco = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OPER_CISCO, field_id))
    {
        int32 ret;
        p_mstp_glb->oper_cisco = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT, field_id))
    {
        int32 ret;
        p_mstp_glb->txHoldCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD, field_id))
    {
        cdb_enum_str2val(mstp_path_cost_strs, MSTP_PATH_COST_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION, field_id))
    {
        int32 ret;
        p_mstp_glb->tc_protection = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_HOPCOUNT, field_id))
    {
        int32 ret;
        p_mstp_glb->hopCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD, field_id))
    {
        int32 ret;
        p_mstp_glb->recv_tcn_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER, field_id))
    {
        int32 ret;
        p_mstp_glb->recv_tcn_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_mstp_glb->root_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX, field_id))
    {
        int32 ret;
        p_mstp_glb->alternate_port_ifndex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PRS_STATE, field_id))
    {
        cdb_enum_str2val(prs_state_strs, PRS_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID, field_id))
    {
        int32 ret;
        p_mstp_glb->mstp_format_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL, field_id))
    {
        int32 ret;
        p_mstp_glb->mstp_revision_level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME, field_id))
    {
        sal_strcpy(p_mstp_glb->mstp_region_name, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST, field_id))
    {
        cdb_mstp_digest_str2val(&p_mstp_glb->mstp_digest, str);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_glb->tc_protection_timer);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN, field_id))
    {
        int32 ret;
        p_mstp_glb->of_exclude_all_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mstp_glb->of_include_vlan_bmp, sizeof(p_mstp_glb->of_include_vlan_bmp)/4, err);
    }
    return PM_E_NONE;
}

int32
tbl_mstp_global_table_parser(char** array, char* key_value,tbl_mstp_global_t *p_mstp_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mstp_global_key_str2val(key_value, p_mstp_glb));

    for(i=1; i<TBL_MSTP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mstp_global_field_value_parser( array[j++], i, p_mstp_glb));
    }

    return PM_E_NONE;
}

int32
tbl_mstp_global_dump_one(tbl_mstp_global_t *p_mstp_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_TYPE].name, 
            cdb_enum_val2str(mstp_type_strs, MSTP_TYPE_MAX, p_mstp_glb->type));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG].name,
            p_mstp_glb->no_mst_config);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_EXTERNAL_ROOT_PATH_COST].name,
            p_mstp_glb->external_root_path_cost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_INTERNAL_ROOT_PATH_COST].name,
            p_mstp_glb->internal_root_path_cost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_PORTID].name,
            p_mstp_glb->root_portId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->root_rootId));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST].name,
            p_mstp_glb->root_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_REGIONROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->root_regionRootId));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_INTROOTPATHCOST].name,
            p_mstp_glb->root_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->root_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID].name,
            p_mstp_glb->root_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_BRIDGEPORTID].name,
            p_mstp_glb->root_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_MESSAGEAGE].name,
            p_mstp_glb->root_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY].name,
            p_mstp_glb->root_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME].name,
            p_mstp_glb->root_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE].name,
            p_mstp_glb->root_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY].name,
            p_mstp_glb->bridge_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME].name,
            p_mstp_glb->bridge_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE].name,
            p_mstp_glb->bridge_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS].name,
            p_mstp_glb->bridge_maxHops);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY].name,
            p_mstp_glb->bridge_priority);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BRIDGE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BRIDGE_ID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->bridge_id));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->last_tc_time));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES].name,
            p_mstp_glb->num_topo_changes);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MLAG_ROLE].name, 
            cdb_enum_val2str(mlag_role_strs, MLAG_ROLE_MAX, p_mstp_glb->mlag_role));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MLAG_ENABLE].name,
            p_mstp_glb->mlag_enable);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ENABLE].name,
            p_mstp_glb->enable);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_TOPOLOGY_CHANGE_DETECTED].name,
            p_mstp_glb->topology_change_detected);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BPDUGUARD].name,
            p_mstp_glb->bpduguard);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_BPDUFILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_BPDUFILTER].name,
            p_mstp_glb->bpdufilter);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ADMIN_CISCO].name,
            p_mstp_glb->admin_cisco);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OPER_CISCO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_OPER_CISCO].name,
            p_mstp_glb->oper_cisco);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT].name,
            p_mstp_glb->txHoldCount);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD].name, 
            cdb_enum_val2str(mstp_path_cost_strs, MSTP_PATH_COST_MAX, p_mstp_glb->pathcost_standard));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_TC_PROTECTION].name,
            p_mstp_glb->tc_protection);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_HOPCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_HOPCOUNT].name,
            p_mstp_glb->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_RECV_TCN_THRESHOLD].name,
            p_mstp_glb->recv_tcn_threshold);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_RECV_TCN_NUMBER].name,
            p_mstp_glb->recv_tcn_number);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ROOT_PORT_IFINDEX].name,
            p_mstp_glb->root_port_ifindex);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_ALTERNATE_PORT_IFNDEX].name,
            p_mstp_glb->alternate_port_ifndex);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_PRS_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_PRS_STATE].name, 
            cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, p_mstp_glb->prs_state));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MSTP_FORMAT_ID].name,
            p_mstp_glb->mstp_format_id);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL].name,
            p_mstp_glb->mstp_revision_level);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME].name,
            p_mstp_glb->mstp_region_name);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST].name, 
            cdb_mstp_digest_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_glb->mstp_digest));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_TC_PROTECTION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_glb->tc_protection_timer));
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN].name,
            p_mstp_glb->of_exclude_all_vlan);
    }
    if (FLD_MATCH(TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mstp_glb->of_include_vlan_bmp, sizeof(p_mstp_glb->of_include_vlan_bmp)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mstp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mstp_global_t *p_db_mstp_glb = _g_p_tbl_mstp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_mstp_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_mstp_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mstp_global_t*
tbl_mstp_global_init_mstp_global()
{
    _g_p_tbl_mstp_global = XCALLOC(MEM_TBL_MSTP_GLOBAL, sizeof(tbl_mstp_global_t));
    return _g_p_tbl_mstp_global;
}


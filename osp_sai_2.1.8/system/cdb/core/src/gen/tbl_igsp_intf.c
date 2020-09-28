
#include "proto.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/ds_mrouter_port_define.h"
#include "gen/ds_mrouter_port.h"
#include "gen/ds_query_define.h"
#include "gen/ds_query.h"
#include "gen/ds_join_group_define.h"
#include "gen/ds_join_group.h"
#include "cdb_data_cmp.h"

tbl_igsp_intf_master_t *_g_p_tbl_igsp_intf_master = NULL;

int32
tbl_igsp_intf_add_igsp_intf_sync(tbl_igsp_intf_t *p_if, uint32 sync)
{
    tbl_igsp_intf_master_t *p_master = _g_p_tbl_igsp_intf_master;
    tbl_igsp_intf_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_igsp_intf_get_igsp_intf(&p_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_if = XCALLOC(MEM_TBL_IGSP_INTF, sizeof(tbl_igsp_intf_t));
    if (NULL == p_db_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_if, p_if, sizeof(tbl_igsp_intf_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_if->mrouter_list, CDB_NODE_DS, DS_MROUTER_PORT, ds_mrouter_port_cmp));
    PM_E_RETURN(cdb_reference_list_init(&p_db_if->query_list, CDB_NODE_DS, DS_QUERY, ds_query_cmp));
    PM_E_RETURN(cdb_reference_list_init(&p_db_if->join_list, CDB_NODE_DS, DS_JOIN_GROUP, ds_join_group_cmp));

    /* 4. add to db */
    p_master->igsp_array[p_if->key.vid] = p_db_if;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IGSP_INTF, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IGSP_INTF, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igsp_intf_del_igsp_intf_sync(tbl_igsp_intf_key_t *p_if_key, uint32 sync)
{
    tbl_igsp_intf_master_t *p_master = _g_p_tbl_igsp_intf_master;
    tbl_igsp_intf_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(p_if_key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IGSP_INTF, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IGSP_INTF, p_db_if);
        #endif 
    }

    /* 3. delete from db */
    p_master->igsp_array[p_if_key->vid] = NULL;
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_if->mrouter_list));
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_if->query_list));
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_if->join_list));

    /* 4. free db entry */
    XFREE(MEM_TBL_IGSP_INTF, p_db_if);

    return PM_E_NONE;
}

int32
tbl_igsp_intf_set_igsp_intf_field_sync(tbl_igsp_intf_t *p_if, tbl_igsp_intf_field_id_t field_id, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IGSP_INTF_FLD_ACL_NAME:
        sal_memcpy(p_db_if->acl_name, p_if->acl_name, sizeof(p_if->acl_name));
        break;

    case TBL_IGSP_INTF_FLD_ENABLE:
        p_db_if->enable = p_if->enable;
        break;

    case TBL_IGSP_INTF_FLD_FAST_LEAVE:
        p_db_if->fast_leave = p_if->fast_leave;
        break;

    case TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN:
        p_db_if->discard_unknown = p_if->discard_unknown;
        break;

    case TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT:
        p_db_if->discard_unknown_act = p_if->discard_unknown_act;
        break;

    case TBL_IGSP_INTF_FLD_REPORT_SUPPRESS:
        p_db_if->report_suppress = p_if->report_suppress;
        break;

    case TBL_IGSP_INTF_FLD_VERSION:
        p_db_if->version = p_if->version;
        break;

    case TBL_IGSP_INTF_FLD_QUERIER_ENABLE:
        p_db_if->querier_enable = p_if->querier_enable;
        break;

    case TBL_IGSP_INTF_FLD_QUERIER_OPERATE:
        p_db_if->querier_operate = p_if->querier_operate;
        break;

    case TBL_IGSP_INTF_FLD_QUERY_INTERVAL:
        p_db_if->query_interval = p_if->query_interval;
        break;

    case TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL:
        p_db_if->admin_other_query_interval = p_if->admin_other_query_interval;
        break;

    case TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL:
        p_db_if->other_query_interval = p_if->other_query_interval;
        break;

    case TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME:
        p_db_if->querier_max_response_time = p_if->querier_max_response_time;
        break;

    case TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS:
        sal_memcpy(&p_db_if->querier_config_address, &p_if->querier_config_address, sizeof(p_if->querier_config_address));
        break;

    case TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS:
        sal_memcpy(&p_db_if->querier_oper_address, &p_if->querier_oper_address, sizeof(p_if->querier_oper_address));
        break;

    case TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS:
        sal_memcpy(&p_db_if->other_querier_address, &p_if->other_querier_address, sizeof(p_if->other_querier_address));
        break;

    case TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS:
        sal_memcpy(&p_db_if->vlan_if_address, &p_if->vlan_if_address, sizeof(p_if->vlan_if_address));
        break;

    case TBL_IGSP_INTF_FLD_LMQI:
        p_db_if->lmqi = p_if->lmqi;
        break;

    case TBL_IGSP_INTF_FLD_LMQC:
        p_db_if->lmqc = p_if->lmqc;
        break;

    case TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER:
        p_db_if->max_member_number = p_if->max_member_number;
        break;

    case TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER:
        p_db_if->curr_group_member = p_if->curr_group_member;
        break;

    case TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL:
        p_db_if->group_member_interval = p_if->group_member_interval;
        break;

    case TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR:
        p_db_if->robustness_var = p_if->robustness_var;
        break;

    case TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL:
        p_db_if->mrouter_aging_interval = p_if->mrouter_aging_interval;
        break;

    case TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT:
        p_db_if->startup_query_count = p_if->startup_query_count;
        break;

    case TBL_IGSP_INTF_FLD_T_QUERIER:
        p_db_if->t_querier = p_if->t_querier;
        break;

    case TBL_IGSP_INTF_FLD_T_OTHER_QUERIER:
        p_db_if->t_other_querier = p_if->t_other_querier;
        break;

    case TBL_IGSP_INTF_FLD_MAX:
        sal_memcpy(p_db_if, p_if, sizeof(tbl_igsp_intf_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IGSP_INTF, field_id, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IGSP_INTF, field_id, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igsp_intf_add_igsp_intf(tbl_igsp_intf_t *p_if)
{
    return tbl_igsp_intf_add_igsp_intf_sync(p_if, TRUE);
}

int32
tbl_igsp_intf_del_igsp_intf(tbl_igsp_intf_key_t *p_if_key)
{
    return tbl_igsp_intf_del_igsp_intf_sync(p_if_key, TRUE);
}

int32
tbl_igsp_intf_set_igsp_intf_field(tbl_igsp_intf_t *p_if, tbl_igsp_intf_field_id_t field_id)
{
    return tbl_igsp_intf_set_igsp_intf_field_sync(p_if, field_id, TRUE);
}

tbl_igsp_intf_t*
tbl_igsp_intf_get_igsp_intf(tbl_igsp_intf_key_t *p_if_key)
{
    tbl_igsp_intf_master_t *p_master = _g_p_tbl_igsp_intf_master;

    if (GLB_MAX_VLAN_ID <= p_if_key->vid)
    {
        return NULL;
    }
    return (p_master->igsp_array[p_if_key->vid]);
}

char*
tbl_igsp_intf_key_val2str(tbl_igsp_intf_t *p_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_if->key.vid);
    return str;
}

int32
tbl_igsp_intf_key_str2val(char *str, tbl_igsp_intf_t *p_if)
{
    int32 ret = 0;

    p_if->key.vid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_igsp_intf_key_name_dump(tbl_igsp_intf_t *p_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_INTF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_if->key.vid);
    return str;
}

char*
tbl_igsp_intf_key_value_dump(tbl_igsp_intf_t *p_if, char *str)
{
    sal_sprintf(str, "%u", p_if->key.vid);
    return str;
}

char*
tbl_igsp_intf_field_name_dump(tbl_igsp_intf_t *p_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IGSP_INTF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_igsp_intf_field_value_dump(tbl_igsp_intf_t *p_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_if->key.vid);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ACL_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_if->acl_name);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->enable);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_FAST_LEAVE, field_id))
    {
        sal_sprintf(str, "%u", p_if->fast_leave);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN, field_id))
    {
        sal_sprintf(str, "%u", p_if->discard_unknown);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT, field_id))
    {
        sal_sprintf(str, "%u", p_if->discard_unknown_act);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_REPORT_SUPPRESS, field_id))
    {
        sal_sprintf(str, "%u", p_if->report_suppress);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_if->version);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->querier_enable);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPERATE, field_id))
    {
        sal_sprintf(str, "%u", p_if->querier_operate);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->admin_other_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->other_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_if->querier_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_if->querier_config_address);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_if->querier_oper_address);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_if->other_querier_address);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_if->vlan_if_address);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQI, field_id))
    {
        sal_sprintf(str, "%u", p_if->lmqi);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQC, field_id))
    {
        sal_sprintf(str, "%u", p_if->lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_if->max_member_number);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER, field_id))
    {
        sal_sprintf(str, "%u", p_if->curr_group_member);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->group_member_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR, field_id))
    {
        sal_sprintf(str, "%u", p_if->robustness_var);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->mrouter_aging_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->startup_query_count);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_QUERIER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_if->t_querier);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_OTHER_QUERIER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_if->t_other_querier);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_JOIN_LIST, field_id))
    {
    }
    return str;
}

char**
tbl_igsp_intf_table_dump(tbl_igsp_intf_t *p_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_igsp_intf_key_name_dump(p_if, buf));
    for(i=1; i<TBL_IGSP_INTF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_igsp_intf_field_name_dump(p_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_igsp_intf_field_value_dump(p_if, i, buf));
    }
    return str;
}

int32
tbl_igsp_intf_field_value_parser(char *str, int32 field_id, tbl_igsp_intf_t *p_if)
{
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_KEY, field_id))
    {
        int32 ret;
        p_if->key.vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ACL_NAME, field_id))
    {
        sal_strcpy(p_if->acl_name, str);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_if->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_FAST_LEAVE, field_id))
    {
        int32 ret;
        p_if->fast_leave = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN, field_id))
    {
        int32 ret;
        p_if->discard_unknown = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT, field_id))
    {
        int32 ret;
        p_if->discard_unknown_act = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_REPORT_SUPPRESS, field_id))
    {
        int32 ret;
        p_if->report_suppress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VERSION, field_id))
    {
        int32 ret;
        p_if->version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_ENABLE, field_id))
    {
        int32 ret;
        p_if->querier_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPERATE, field_id))
    {
        int32 ret;
        p_if->querier_operate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_if->query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_if->admin_other_query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_if->other_query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME, field_id))
    {
        int32 ret;
        p_if->querier_max_response_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_if->querier_config_address, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_if->querier_oper_address, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_if->other_querier_address, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_if->vlan_if_address, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQI, field_id))
    {
        int32 ret;
        p_if->lmqi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQC, field_id))
    {
        int32 ret;
        p_if->lmqc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER, field_id))
    {
        int32 ret;
        p_if->max_member_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER, field_id))
    {
        int32 ret;
        p_if->curr_group_member = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL, field_id))
    {
        int32 ret;
        p_if->group_member_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR, field_id))
    {
        int32 ret;
        p_if->robustness_var = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL, field_id))
    {
        int32 ret;
        p_if->mrouter_aging_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT, field_id))
    {
        int32 ret;
        p_if->startup_query_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_QUERIER, field_id))
    {
        ctc_task_str2val(str, p_if->t_querier);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_OTHER_QUERIER, field_id))
    {
        ctc_task_str2val(str, p_if->t_other_querier);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_JOIN_LIST, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_igsp_intf_table_parser(char** array, char* key_value,tbl_igsp_intf_t *p_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_igsp_intf_key_str2val(key_value, p_if));

    for(i=1; i<TBL_IGSP_INTF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_igsp_intf_field_value_parser( array[j++], i, p_if));
    }

    return PM_E_NONE;
}

int32
tbl_igsp_intf_dump_one(tbl_igsp_intf_t *p_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_INTF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ds_query_t *p_db_query = NULL;
    ds_join_group_t *p_db_join_group = NULL;
    ctclib_slistnode_t *listnode = NULL;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IGSP_INTF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_KEY].name,
            p_if->key.vid);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ACL_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_ACL_NAME].name,
            p_if->acl_name);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_ENABLE].name,
            p_if->enable);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_FAST_LEAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_FAST_LEAVE].name,
            p_if->fast_leave);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN].name,
            p_if->discard_unknown);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT].name,
            p_if->discard_unknown_act);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_REPORT_SUPPRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_REPORT_SUPPRESS].name,
            p_if->report_suppress);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_VERSION].name,
            p_if->version);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERIER_ENABLE].name,
            p_if->querier_enable);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPERATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERIER_OPERATE].name,
            p_if->querier_operate);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERY_INTERVAL].name,
            p_if->query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL].name,
            p_if->admin_other_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL].name,
            p_if->other_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME].name,
            p_if->querier_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_if->querier_config_address));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_if->querier_oper_address));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_if->other_querier_address));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_if->vlan_if_address));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_LMQI].name,
            p_if->lmqi);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_LMQC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_LMQC].name,
            p_if->lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER].name,
            p_if->max_member_number);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER].name,
            p_if->curr_group_member);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL].name,
            p_if->group_member_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR].name,
            p_if->robustness_var);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL].name,
            p_if->mrouter_aging_interval);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT].name,
            p_if->startup_query_count);
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_QUERIER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_T_QUERIER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_if->t_querier));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_T_OTHER_QUERIER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_INTF_FLD_T_OTHER_QUERIER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_if->t_other_querier));
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_MROUTER_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_IGSP_INTF_FLD_MROUTER_LIST].name);
        if (p_if->mrouter_list.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
            {
                ds_mrouter_port_dump_one(p_db_mrouter_port, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_QUERY_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_IGSP_INTF_FLD_QUERY_LIST].name);
        if (p_if->query_list.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_if->query_list.obj_list, p_db_query, listnode)
            {
                ds_query_dump_one(p_db_query, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_IGSP_INTF_FLD_JOIN_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_IGSP_INTF_FLD_JOIN_LIST].name);
        if (p_if->join_list.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_if->join_list.obj_list, p_db_join_group, listnode)
            {
                ds_join_group_dump_one(p_db_join_group, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_igsp_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_igsp_intf_master_t *p_master = _g_p_tbl_igsp_intf_master;
    tbl_igsp_intf_t *p_db_if = NULL;
    uint32 vid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_if = p_master->igsp_array[vid];
        if (NULL == p_db_if)
        {
            continue;
        }
        rc |= fn(p_db_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_igsp_intf_master_t*
tbl_igsp_intf_get_master()
{
    return _g_p_tbl_igsp_intf_master;
}

tbl_igsp_intf_master_t*
tbl_igsp_intf_init_igsp_intf()
{
    _g_p_tbl_igsp_intf_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_igsp_intf_master_t));
    return _g_p_tbl_igsp_intf_master;
}


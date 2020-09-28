
#include "proto.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "cdb_data_cmp.h"

tbl_igsp_global_t *_g_p_tbl_igsp_global = NULL;

int32
tbl_igsp_global_set_igsp_global_field_sync(tbl_igsp_global_t *p_glb, tbl_igsp_global_field_id_t field_id, uint32 sync)
{
    tbl_igsp_global_t *p_db_glb = _g_p_tbl_igsp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IGSP_GLOBAL_FLD_ENABLE:
        p_db_glb->enable = p_glb->enable;
        break;

    case TBL_IGSP_GLOBAL_FLD_FAST_LEAVE:
        p_db_glb->fast_leave = p_glb->fast_leave;
        break;

    case TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN:
        p_db_glb->discard_unknown = p_glb->discard_unknown;
        break;

    case TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS:
        p_db_glb->report_suppress = p_glb->report_suppress;
        break;

    case TBL_IGSP_GLOBAL_FLD_VERSION:
        p_db_glb->version = p_glb->version;
        break;

    case TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME:
        p_db_glb->querier_max_response_time = p_glb->querier_max_response_time;
        break;

    case TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL:
        p_db_glb->query_interval = p_glb->query_interval;
        break;

    case TBL_IGSP_GLOBAL_FLD_LMQI:
        p_db_glb->lmqi = p_glb->lmqi;
        break;

    case TBL_IGSP_GLOBAL_FLD_LMQC:
        p_db_glb->lmqc = p_glb->lmqc;
        break;

    case TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR:
        p_db_glb->robustness_var = p_glb->robustness_var;
        break;

    case TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER:
        p_db_glb->max_member_number = p_glb->max_member_number;
        break;

    case TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER:
        p_db_glb->curr_group_member = p_glb->curr_group_member;
        break;

    case TBL_IGSP_GLOBAL_FLD_TCN_ENABLE:
        p_db_glb->tcn_enable = p_glb->tcn_enable;
        break;

    case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT:
        p_db_glb->tcn_query_count = p_glb->tcn_query_count;
        break;

    case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL:
        p_db_glb->tcn_query_interval = p_glb->tcn_query_interval;
        break;

    case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT:
        p_db_glb->tcn_query_current_count = p_glb->tcn_query_current_count;
        break;

    case TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME:
        p_db_glb->tcn_query_max_response_time = p_glb->tcn_query_max_response_time;
        break;

    case TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE:
        p_db_glb->host_join_enable = p_glb->host_join_enable;
        break;

    case TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM:
        p_db_glb->host_joined_num = p_glb->host_joined_num;
        break;

    case TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC:
        sal_memcpy(&p_db_glb->global_src, &p_glb->global_src, sizeof(p_glb->global_src));
        break;

    case TBL_IGSP_GLOBAL_FLD_ALL_HOSTS:
        sal_memcpy(&p_db_glb->all_hosts, &p_glb->all_hosts, sizeof(p_glb->all_hosts));
        break;

    case TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS:
        sal_memcpy(&p_db_glb->all_routers, &p_glb->all_routers, sizeof(p_glb->all_routers));
        break;

    case TBL_IGSP_GLOBAL_FLD_V3_ROUTERS:
        sal_memcpy(&p_db_glb->v3_routers, &p_glb->v3_routers, sizeof(p_glb->v3_routers));
        break;

    case TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE:
        p_db_glb->t_group_expire = p_glb->t_group_expire;
        break;

    case TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY:
        p_db_glb->t_tcn_query = p_glb->t_tcn_query;
        break;

    case TBL_IGSP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_glb, p_glb, sizeof(tbl_igsp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IGSP_GLOBAL, field_id, p_db_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IGSP_GLOBAL, field_id, p_db_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igsp_global_set_igsp_global_field(tbl_igsp_global_t *p_glb, tbl_igsp_global_field_id_t field_id)
{
    return tbl_igsp_global_set_igsp_global_field_sync(p_glb, field_id, TRUE);
}

tbl_igsp_global_t*
tbl_igsp_global_get_igsp_global()
{
    return _g_p_tbl_igsp_global;
}

int32
tbl_igsp_global_key_str2val(char *str, tbl_igsp_global_t *p_glb)
{
    return PM_E_NONE;
}

char*
tbl_igsp_global_key_name_dump(tbl_igsp_global_t *p_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_igsp_global_key_value_dump(tbl_igsp_global_t *p_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_igsp_global_field_name_dump(tbl_igsp_global_t *p_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IGSP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_igsp_global_field_value_dump(tbl_igsp_global_t *p_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_glb->enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_FAST_LEAVE, field_id))
    {
        sal_sprintf(str, "%u", p_glb->fast_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN, field_id))
    {
        sal_sprintf(str, "%u", p_glb->discard_unknown);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS, field_id))
    {
        sal_sprintf(str, "%u", p_glb->report_suppress);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_glb->version);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_glb->querier_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_glb->query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQI, field_id))
    {
        sal_sprintf(str, "%u", p_glb->lmqi);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQC, field_id))
    {
        sal_sprintf(str, "%u", p_glb->lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR, field_id))
    {
        sal_sprintf(str, "%u", p_glb->robustness_var);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_glb->max_member_number);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER, field_id))
    {
        sal_sprintf(str, "%u", p_glb->curr_group_member);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_glb->tcn_enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_glb->tcn_query_count);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_glb->tcn_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_glb->tcn_query_current_count);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_glb->tcn_query_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_glb->host_join_enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_glb->host_joined_num);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_glb->global_src);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_HOSTS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_glb->all_hosts);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_glb->all_routers);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_V3_ROUTERS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_glb->v3_routers);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_glb->t_group_expire);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_glb->t_tcn_query);
    }
    return str;
}

char**
tbl_igsp_global_table_dump(tbl_igsp_global_t *p_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_igsp_global_key_name_dump(p_glb, buf));
    for(i=1; i<TBL_IGSP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_igsp_global_field_name_dump(p_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_igsp_global_field_value_dump(p_glb, i, buf));
    }
    return str;
}

int32
tbl_igsp_global_field_value_parser(char *str, int32 field_id, tbl_igsp_global_t *p_glb)
{
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_glb->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_FAST_LEAVE, field_id))
    {
        int32 ret;
        p_glb->fast_leave = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN, field_id))
    {
        int32 ret;
        p_glb->discard_unknown = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS, field_id))
    {
        int32 ret;
        p_glb->report_suppress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_VERSION, field_id))
    {
        int32 ret;
        p_glb->version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME, field_id))
    {
        int32 ret;
        p_glb->querier_max_response_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_glb->query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQI, field_id))
    {
        int32 ret;
        p_glb->lmqi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQC, field_id))
    {
        int32 ret;
        p_glb->lmqc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR, field_id))
    {
        int32 ret;
        p_glb->robustness_var = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER, field_id))
    {
        int32 ret;
        p_glb->max_member_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER, field_id))
    {
        int32 ret;
        p_glb->curr_group_member = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_ENABLE, field_id))
    {
        int32 ret;
        p_glb->tcn_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT, field_id))
    {
        int32 ret;
        p_glb->tcn_query_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_glb->tcn_query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT, field_id))
    {
        int32 ret;
        p_glb->tcn_query_current_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME, field_id))
    {
        int32 ret;
        p_glb->tcn_query_max_response_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE, field_id))
    {
        int32 ret;
        p_glb->host_join_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM, field_id))
    {
        int32 ret;
        p_glb->host_joined_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC, field_id))
    {
        cdb_addr_ipv4_str2val(&p_glb->global_src, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_HOSTS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_glb->all_hosts, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_glb->all_routers, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_V3_ROUTERS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_glb->v3_routers, str, 0);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE, field_id))
    {
        ctc_task_str2val(str, p_glb->t_group_expire);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY, field_id))
    {
        ctc_task_str2val(str, p_glb->t_tcn_query);
    }
    return PM_E_NONE;
}

int32
tbl_igsp_global_table_parser(char** array, char* key_value,tbl_igsp_global_t *p_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_igsp_global_key_str2val(key_value, p_glb));

    for(i=1; i<TBL_IGSP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_igsp_global_field_value_parser( array[j++], i, p_glb));
    }

    return PM_E_NONE;
}

int32
tbl_igsp_global_dump_one(tbl_igsp_global_t *p_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGSP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_ENABLE].name,
            p_glb->enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_FAST_LEAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_FAST_LEAVE].name,
            p_glb->fast_leave);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN].name,
            p_glb->discard_unknown);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS].name,
            p_glb->report_suppress);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_VERSION].name,
            p_glb->version);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME].name,
            p_glb->querier_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL].name,
            p_glb->query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_LMQI].name,
            p_glb->lmqi);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_LMQC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_LMQC].name,
            p_glb->lmqc);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR].name,
            p_glb->robustness_var);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER].name,
            p_glb->max_member_number);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER].name,
            p_glb->curr_group_member);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_TCN_ENABLE].name,
            p_glb->tcn_enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT].name,
            p_glb->tcn_query_count);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL].name,
            p_glb->tcn_query_interval);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT].name,
            p_glb->tcn_query_current_count);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME].name,
            p_glb->tcn_query_max_response_time);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE].name,
            p_glb->host_join_enable);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM].name,
            p_glb->host_joined_num);
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_glb->global_src));
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_HOSTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_ALL_HOSTS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_glb->all_hosts));
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_glb->all_routers));
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_V3_ROUTERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_V3_ROUTERS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_glb->v3_routers));
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_glb->t_group_expire));
    }
    if (FLD_MATCH(TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_glb->t_tcn_query));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_igsp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_igsp_global_t *p_db_glb = _g_p_tbl_igsp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_igsp_global_t*
tbl_igsp_global_init_igsp_global()
{
    _g_p_tbl_igsp_global = XCALLOC(MEM_TBL_IGSP_GLOBAL, sizeof(tbl_igsp_global_t));
    return _g_p_tbl_igsp_global;
}


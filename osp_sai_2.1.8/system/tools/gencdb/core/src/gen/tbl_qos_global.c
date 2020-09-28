
#include "proto.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "cdb_data_cmp.h"

tbl_qos_global_t *_g_p_tbl_qos_global = NULL;

int32
tbl_qos_global_set_qos_global_field_sync(tbl_qos_global_t *p_qos_glb, tbl_qos_global_field_id_t field_id, uint32 sync)
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_GLOBAL_FLD_QOS_ENABLE:
        p_db_qos_glb->qos_enable = p_qos_glb->qos_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_PHB_ENABLE:
        p_db_qos_glb->phb_enable = p_qos_glb->phb_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE:
        p_db_qos_glb->port_policer_first_enable = p_qos_glb->port_policer_first_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE:
        p_db_qos_glb->policer_stats_enable = p_qos_glb->policer_stats_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE:
        p_db_qos_glb->cur_cpu_rate = p_qos_glb->cur_cpu_rate;
        break;

    case TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE:
        p_db_qos_glb->def_cpu_rate = p_qos_glb->def_cpu_rate;
        break;

    case TBL_QOS_GLOBAL_FLD_LATENCY_RANGE:
        sal_memcpy(p_db_qos_glb->latency_range, p_qos_glb->latency_range, sizeof(p_qos_glb->latency_range));
        break;

    case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE:
        p_db_qos_glb->buffer_event_enable = p_qos_glb->buffer_event_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH:
        p_db_qos_glb->buffer_event_high = p_qos_glb->buffer_event_high;
        break;

    case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW:
        p_db_qos_glb->buffer_event_low = p_qos_glb->buffer_event_low;
        break;

    case TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE:
        p_db_qos_glb->check_qos_enable = p_qos_glb->check_qos_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE:
        p_db_qos_glb->qos_ipg_enable = p_qos_glb->qos_ipg_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT:
        p_db_qos_glb->in_port_policer_res_cnt = p_qos_glb->in_port_policer_res_cnt;
        break;

    case TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT:
        p_db_qos_glb->eg_port_policer_res_cnt = p_qos_glb->eg_port_policer_res_cnt;
        break;

    case TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT:
        p_db_qos_glb->in_flow_policer_res_cnt = p_qos_glb->in_flow_policer_res_cnt;
        break;

    case TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT:
        p_db_qos_glb->eg_flow_policer_res_cnt = p_qos_glb->eg_flow_policer_res_cnt;
        break;

    case TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE:
        p_db_qos_glb->queue_shape_enable = p_qos_glb->queue_shape_enable;
        break;

    case TBL_QOS_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_qos_glb, p_qos_glb, sizeof(tbl_qos_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_GLOBAL, field_id, p_db_qos_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_GLOBAL, field_id, p_db_qos_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_global_set_qos_global_field(tbl_qos_global_t *p_qos_glb, tbl_qos_global_field_id_t field_id)
{
    return tbl_qos_global_set_qos_global_field_sync(p_qos_glb, field_id, TRUE);
}

tbl_qos_global_t*
tbl_qos_global_get_qos_global()
{
    return _g_p_tbl_qos_global;
}

int32
tbl_qos_global_key_str2val(char *str, tbl_qos_global_t *p_qos_glb)
{
    return PM_E_NONE;
}

char*
tbl_qos_global_key_name_dump(tbl_qos_global_t *p_qos_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_qos_global_key_value_dump(tbl_qos_global_t *p_qos_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_qos_global_field_name_dump(tbl_qos_global_t *p_qos_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_global_field_value_dump(tbl_qos_global_t *p_qos_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->qos_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PHB_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->phb_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->port_policer_first_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->policer_stats_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->cur_cpu_rate);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->def_cpu_rate);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_LATENCY_RANGE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_qos_glb->latency_range, sizeof(p_qos_glb->latency_range)/4);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->buffer_event_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->buffer_event_high);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->buffer_event_low);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->check_qos_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->qos_ipg_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->in_port_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->eg_port_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->in_flow_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->eg_flow_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_glb->queue_shape_enable);
    }
    return str;
}

char**
tbl_qos_global_table_dump(tbl_qos_global_t *p_qos_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_global_key_name_dump(p_qos_glb, buf));
    for(i=1; i<TBL_QOS_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_global_field_name_dump(p_qos_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_global_field_value_dump(p_qos_glb, i, buf));
    }
    return str;
}

int32
tbl_qos_global_field_value_parser(char *str, int32 field_id, tbl_qos_global_t *p_qos_glb)
{
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->qos_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PHB_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->phb_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->port_policer_first_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->policer_stats_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE, field_id))
    {
        int32 ret;
        p_qos_glb->cur_cpu_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE, field_id))
    {
        int32 ret;
        p_qos_glb->def_cpu_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_LATENCY_RANGE, field_id))
    {
        cdb_uint32_array_str2val(str, p_qos_glb->latency_range);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->buffer_event_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH, field_id))
    {
        int32 ret;
        p_qos_glb->buffer_event_high = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW, field_id))
    {
        int32 ret;
        p_qos_glb->buffer_event_low = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->check_qos_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->qos_ipg_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT, field_id))
    {
        int32 ret;
        p_qos_glb->in_port_policer_res_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT, field_id))
    {
        int32 ret;
        p_qos_glb->eg_port_policer_res_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT, field_id))
    {
        int32 ret;
        p_qos_glb->in_flow_policer_res_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT, field_id))
    {
        int32 ret;
        p_qos_glb->eg_flow_policer_res_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE, field_id))
    {
        int32 ret;
        p_qos_glb->queue_shape_enable = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_global_table_parser(char** array, char* key_value,tbl_qos_global_t *p_qos_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_global_key_str2val(key_value, p_qos_glb));

    for(i=1; i<TBL_QOS_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_global_field_value_parser( array[j++], i, p_qos_glb));
    }

    return PM_E_NONE;
}

int32
tbl_qos_global_dump_one(tbl_qos_global_t *p_qos_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_QOS_ENABLE].name,
            p_qos_glb->qos_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PHB_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_PHB_ENABLE].name,
            p_qos_glb->phb_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE].name,
            p_qos_glb->port_policer_first_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE].name,
            p_qos_glb->policer_stats_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE].name,
            p_qos_glb->cur_cpu_rate);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE].name,
            p_qos_glb->def_cpu_rate);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_LATENCY_RANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_LATENCY_RANGE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_qos_glb->latency_range, sizeof(p_qos_glb->latency_range)/4));
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE].name,
            p_qos_glb->buffer_event_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH].name,
            p_qos_glb->buffer_event_high);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW].name,
            p_qos_glb->buffer_event_low);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE].name,
            p_qos_glb->check_qos_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE].name,
            p_qos_glb->qos_ipg_enable);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT].name,
            p_qos_glb->in_port_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT].name,
            p_qos_glb->eg_port_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT].name,
            p_qos_glb->in_flow_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT].name,
            p_qos_glb->eg_flow_policer_res_cnt);
    }
    if (FLD_MATCH(TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE].name,
            p_qos_glb->queue_shape_enable);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_qos_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_qos_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_global_t*
tbl_qos_global_init_qos_global()
{
    _g_p_tbl_qos_global = XCALLOC(MEM_TBL_QOS_GLOBAL, sizeof(tbl_qos_global_t));
    return _g_p_tbl_qos_global;
}

bool
tbl_qos_global_get_phb_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->phb_enable;
}

bool
tbl_qos_global_get_qos_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->qos_enable;
}

bool
tbl_qos_global_get_port_policer_first_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->port_policer_first_enable ;
}

bool
tbl_qos_global_get_policer_stats_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->policer_stats_enable;
}

int32
tbl_qos_global_get_def_cpu_rate()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->def_cpu_rate;
}

int32
tbl_qos_global_get_cur_cpu_rate()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->cur_cpu_rate;
}


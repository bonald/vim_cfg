
#include "proto.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "cdb_data_cmp.h"

tbl_sflow_global_t *_g_p_tbl_sflow_global = NULL;

int32
tbl_sflow_global_set_sflow_global_field_sync(tbl_sflow_global_t *p_sflow_glb, tbl_sflow_global_field_id_t field_id, uint32 sync)
{
    tbl_sflow_global_t *p_db_sflow_glb = _g_p_tbl_sflow_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SFLOW_GLOBAL_FLD_ENABLE:
        p_db_sflow_glb->enable = p_sflow_glb->enable;
        break;

    case TBL_SFLOW_GLOBAL_FLD_VERSION:
        p_db_sflow_glb->version = p_sflow_glb->version;
        break;

    case TBL_SFLOW_GLOBAL_FLD_AGENT:
        sal_memcpy(&p_db_sflow_glb->agent, &p_sflow_glb->agent, sizeof(p_sflow_glb->agent));
        break;

    case TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6:
        sal_memcpy(&p_db_sflow_glb->agent_ipv6, &p_sflow_glb->agent_ipv6, sizeof(p_sflow_glb->agent_ipv6));
        break;

    case TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL:
        p_db_sflow_glb->counter_interval = p_sflow_glb->counter_interval;
        break;

    case TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM:
        p_db_sflow_glb->counter_port_num = p_sflow_glb->counter_port_num;
        break;

    case TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER:
        p_db_sflow_glb->counter_timer = p_sflow_glb->counter_timer;
        break;

    case TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT:
        p_db_sflow_glb->counter_next_port = p_sflow_glb->counter_next_port;
        break;

    case TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL:
        CDB_FLAG_CHECK_SET(p_db_sflow_glb, p_sflow_glb, sflow, SFLOW_FLAG_ALL)
        break;

    case TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER:
        CDB_FLAG_CHECK_SET(p_db_sflow_glb, p_sflow_glb, sflow, SFLOW_FLAG_COUNTER)
        break;

    case TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE:
        CDB_FLAG_CHECK_SET(p_db_sflow_glb, p_sflow_glb, sflow, SFLOW_FLAG_SAMPLE)
        break;

    case TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET:
        CDB_FLAG_CHECK_SET(p_db_sflow_glb, p_sflow_glb, sflow, SFLOW_FLAG_PACKET)
        break;

    case TBL_SFLOW_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_sflow_glb, p_sflow_glb, sizeof(tbl_sflow_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SFLOW_GLOBAL, field_id, p_db_sflow_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SFLOW_GLOBAL, field_id, p_db_sflow_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sflow_global_set_sflow_global_field(tbl_sflow_global_t *p_sflow_glb, tbl_sflow_global_field_id_t field_id)
{
    return tbl_sflow_global_set_sflow_global_field_sync(p_sflow_glb, field_id, TRUE);
}

tbl_sflow_global_t*
tbl_sflow_global_get_sflow_global()
{
    return _g_p_tbl_sflow_global;
}

int32
tbl_sflow_global_key_str2val(char *str, tbl_sflow_global_t *p_sflow_glb)
{
    return PM_E_NONE;
}

char*
tbl_sflow_global_key_name_dump(tbl_sflow_global_t *p_sflow_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_sflow_global_key_value_dump(tbl_sflow_global_t *p_sflow_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_sflow_global_field_name_dump(tbl_sflow_global_t *p_sflow_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SFLOW_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sflow_global_field_value_dump(tbl_sflow_global_t *p_sflow_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_glb->enable);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_glb->version);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_sflow_glb->agent);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_sflow_glb->agent_ipv6);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_glb->counter_interval);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_glb->counter_port_num);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_sflow_glb->counter_timer);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_glb->counter_next_port);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_ALL));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_COUNTER));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_SAMPLE));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_PACKET));
    }
    return str;
}

char**
tbl_sflow_global_table_dump(tbl_sflow_global_t *p_sflow_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sflow_global_key_name_dump(p_sflow_glb, buf));
    for(i=1; i<TBL_SFLOW_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sflow_global_field_name_dump(p_sflow_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sflow_global_field_value_dump(p_sflow_glb, i, buf));
    }
    return str;
}

int32
tbl_sflow_global_field_value_parser(char *str, int32 field_id, tbl_sflow_global_t *p_sflow_glb)
{
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_sflow_glb->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_VERSION, field_id))
    {
        int32 ret;
        p_sflow_glb->version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT, field_id))
    {
        cdb_addr_str2val(&p_sflow_glb->agent, str, 0);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6, field_id))
    {
        cdb_addr_str2val(&p_sflow_glb->agent_ipv6, str, 0);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL, field_id))
    {
        int32 ret;
        p_sflow_glb->counter_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM, field_id))
    {
        int32 ret;
        p_sflow_glb->counter_port_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER, field_id))
    {
        ctc_task_str2val(str, p_sflow_glb->counter_timer);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT, field_id))
    {
        int32 ret;
        p_sflow_glb->counter_next_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL, field_id))
    {
        GLB_SET_FLAG(p_sflow_glb->sflow, SFLOW_FLAG_ALL);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER, field_id))
    {
        GLB_SET_FLAG(p_sflow_glb->sflow, SFLOW_FLAG_COUNTER);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE, field_id))
    {
        GLB_SET_FLAG(p_sflow_glb->sflow, SFLOW_FLAG_SAMPLE);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET, field_id))
    {
        GLB_SET_FLAG(p_sflow_glb->sflow, SFLOW_FLAG_PACKET);
    }
    return PM_E_NONE;
}

int32
tbl_sflow_global_table_parser(char** array, char* key_value,tbl_sflow_global_t *p_sflow_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sflow_global_key_str2val(key_value, p_sflow_glb));

    for(i=1; i<TBL_SFLOW_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sflow_global_field_value_parser( array[j++], i, p_sflow_glb));
    }

    return PM_E_NONE;
}

int32
tbl_sflow_global_dump_one(tbl_sflow_global_t *p_sflow_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_ENABLE].name,
            p_sflow_glb->enable);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_VERSION].name,
            p_sflow_glb->version);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_AGENT].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_sflow_glb->agent));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_sflow_glb->agent_ipv6));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL].name,
            p_sflow_glb->counter_interval);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM].name,
            p_sflow_glb->counter_port_num);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_sflow_glb->counter_timer));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT].name,
            p_sflow_glb->counter_next_port);
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL].name,
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_ALL));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER].name,
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_COUNTER));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE].name,
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_SAMPLE));
    }
    if (FLD_MATCH(TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET].name,
            GLB_FLAG_ISSET(p_sflow_glb->sflow, SFLOW_FLAG_PACKET));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sflow_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sflow_global_t *p_db_sflow_glb = _g_p_tbl_sflow_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_sflow_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_sflow_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sflow_global_t*
tbl_sflow_global_init_sflow_global()
{
    _g_p_tbl_sflow_global = XCALLOC(MEM_TBL_SFLOW_GLOBAL, sizeof(tbl_sflow_global_t));
    return _g_p_tbl_sflow_global;
}



#include "proto.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "cdb_data_cmp.h"

tbl_tpoam_global_t *_g_p_tbl_tpoam_global = NULL;

int32
tbl_tpoam_global_set_tpoam_global_field_sync(tbl_tpoam_global_t *p_tpoam_global, tbl_tpoam_global_field_id_t field_id, uint32 sync)
{
    tbl_tpoam_global_t *p_db_tpoam_global = _g_p_tbl_tpoam_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TPOAM_GLOBAL_FLD_NODE_ID:
        p_db_tpoam_global->node_id = p_tpoam_global->node_id;
        break;

    case TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE:
        p_db_tpoam_global->channel_type = p_tpoam_global->channel_type;
        break;

    case TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN:
        p_db_tpoam_global->fast_aps_en = p_tpoam_global->fast_aps_en;
        break;

    case TBL_TPOAM_GLOBAL_FLD_RDI_REASON:
        p_db_tpoam_global->rdi_reason = p_tpoam_global->rdi_reason;
        break;

    case TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK:
        p_db_tpoam_global->rdi_reason_mask = p_tpoam_global->rdi_reason_mask;
        break;

    case TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK:
        p_db_tpoam_global->sf_reason_mask = p_tpoam_global->sf_reason_mask;
        break;

    case TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK:
        p_db_tpoam_global->sd_reason_mask = p_tpoam_global->sd_reason_mask;
        break;

    case TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX:
        sal_memcpy(&p_db_tpoam_global->resouce_count_max, &p_tpoam_global->resouce_count_max, sizeof(p_tpoam_global->resouce_count_max));
        break;

    case TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR:
        sal_memcpy(&p_db_tpoam_global->resouce_count_curr, &p_tpoam_global->resouce_count_curr, sizeof(p_tpoam_global->resouce_count_curr));
        break;

    case TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP:
        sal_memcpy(p_db_tpoam_global->event_rule_map, p_tpoam_global->event_rule_map, sizeof(p_tpoam_global->event_rule_map));
        break;

    case TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS:
        sal_memcpy(&p_db_tpoam_global->oam_glb_stats, &p_tpoam_global->oam_glb_stats, sizeof(p_tpoam_global->oam_glb_stats));
        break;

    case TBL_TPOAM_GLOBAL_FLD_EXEC:
        sal_memcpy(&p_db_tpoam_global->exec, &p_tpoam_global->exec, sizeof(p_tpoam_global->exec));
        break;

    case TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE:
        sal_memcpy(&p_db_tpoam_global->trap_cache, &p_tpoam_global->trap_cache, sizeof(p_tpoam_global->trap_cache));
        break;

    case TBL_TPOAM_GLOBAL_FLD_TRAP:
        sal_memcpy(&p_db_tpoam_global->trap, &p_tpoam_global->trap, sizeof(p_tpoam_global->trap));
        break;

    case TBL_TPOAM_GLOBAL_FLD_LOG:
        sal_memcpy(&p_db_tpoam_global->log, &p_tpoam_global->log, sizeof(p_tpoam_global->log));
        break;

    case TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL:
        p_db_tpoam_global->dm_enable_global = p_tpoam_global->dm_enable_global;
        break;

    case TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD:
        p_db_tpoam_global->dm_delay_threshold = p_tpoam_global->dm_delay_threshold;
        break;

    case TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL:
        p_db_tpoam_global->dm_calc_interval = p_tpoam_global->dm_calc_interval;
        break;

    case TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL:
        p_db_tpoam_global->lm_enable_global = p_tpoam_global->lm_enable_global;
        break;

    case TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD:
        p_db_tpoam_global->lm_local_threshold = p_tpoam_global->lm_local_threshold;
        break;

    case TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD:
        p_db_tpoam_global->lm_remote_threshold = p_tpoam_global->lm_remote_threshold;
        break;

    case TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL:
        p_db_tpoam_global->lm_calc_interval = p_tpoam_global->lm_calc_interval;
        break;

    case TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER:
        p_db_tpoam_global->lm_calc_interval_timer = p_tpoam_global->lm_calc_interval_timer;
        break;

    case TBL_TPOAM_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_tpoam_global, p_tpoam_global, sizeof(tbl_tpoam_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TPOAM_GLOBAL, field_id, p_db_tpoam_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TPOAM_GLOBAL, field_id, p_db_tpoam_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tpoam_global_set_tpoam_global_field(tbl_tpoam_global_t *p_tpoam_global, tbl_tpoam_global_field_id_t field_id)
{
    return tbl_tpoam_global_set_tpoam_global_field_sync(p_tpoam_global, field_id, TRUE);
}

tbl_tpoam_global_t*
tbl_tpoam_global_get_tpoam_global()
{
    return _g_p_tbl_tpoam_global;
}

int32
tbl_tpoam_global_key_str2val(char *str, tbl_tpoam_global_t *p_tpoam_global)
{
    return PM_E_NONE;
}

char*
tbl_tpoam_global_key_name_dump(tbl_tpoam_global_t *p_tpoam_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_tpoam_global_key_value_dump(tbl_tpoam_global_t *p_tpoam_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_tpoam_global_field_name_dump(tbl_tpoam_global_t *p_tpoam_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TPOAM_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tpoam_global_field_value_dump(tbl_tpoam_global_t *p_tpoam_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_NODE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->node_id);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->channel_type);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->fast_aps_en);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->rdi_reason);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->rdi_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->sf_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->sd_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX, field_id))
    {
        cdb_tpoam_rc_count_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->resouce_count_max);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR, field_id))
    {
        cdb_tpoam_rc_count_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->resouce_count_curr);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_tpoam_global->event_rule_map, sizeof(p_tpoam_global->event_rule_map)/4);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS, field_id))
    {
        cdb_tpoam_stats_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->oam_glb_stats);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EXEC, field_id))
    {
        cdb_tpoam_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->exec);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE, field_id))
    {
        cdb_tpoam_trap_cache_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->trap_cache);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP, field_id))
    {
        cdb_tpoam_ratelimit_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->trap);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LOG, field_id))
    {
        cdb_tpoam_ratelimit_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_global->log);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->dm_enable_global);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->dm_delay_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->dm_calc_interval);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->lm_enable_global);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->lm_local_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->lm_remote_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_global->lm_calc_interval);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_tpoam_global->lm_calc_interval_timer);
    }
    return str;
}

char**
tbl_tpoam_global_table_dump(tbl_tpoam_global_t *p_tpoam_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tpoam_global_key_name_dump(p_tpoam_global, buf));
    for(i=1; i<TBL_TPOAM_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tpoam_global_field_name_dump(p_tpoam_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tpoam_global_field_value_dump(p_tpoam_global, i, buf));
    }
    return str;
}

int32
tbl_tpoam_global_field_value_parser(char *str, int32 field_id, tbl_tpoam_global_t *p_tpoam_global)
{
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_NODE_ID, field_id))
    {
        int32 ret;
        p_tpoam_global->node_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE, field_id))
    {
        int32 ret;
        p_tpoam_global->channel_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN, field_id))
    {
        int32 ret;
        p_tpoam_global->fast_aps_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON, field_id))
    {
        int32 ret;
        p_tpoam_global->rdi_reason = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK, field_id))
    {
        int32 ret;
        p_tpoam_global->rdi_reason_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK, field_id))
    {
        int32 ret;
        p_tpoam_global->sf_reason_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK, field_id))
    {
        int32 ret;
        p_tpoam_global->sd_reason_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX, field_id))
    {
        cdb_tpoam_rc_count_str2val(str, &p_tpoam_global->resouce_count_max);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR, field_id))
    {
        cdb_tpoam_rc_count_str2val(str, &p_tpoam_global->resouce_count_curr);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP, field_id))
    {
        cdb_uint32_array_str2val(str, p_tpoam_global->event_rule_map);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS, field_id))
    {
        cdb_tpoam_stats_str2val(str, &p_tpoam_global->oam_glb_stats);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EXEC, field_id))
    {
        cdb_tpoam_exec_str2val(str, &p_tpoam_global->exec);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE, field_id))
    {
        cdb_tpoam_trap_cache_str2val(str, &p_tpoam_global->trap_cache);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP, field_id))
    {
        cdb_tpoam_ratelimit_str2val(str, &p_tpoam_global->trap);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LOG, field_id))
    {
        cdb_tpoam_ratelimit_str2val(str, &p_tpoam_global->log);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL, field_id))
    {
        int32 ret;
        p_tpoam_global->dm_enable_global = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD, field_id))
    {
        int32 ret;
        p_tpoam_global->dm_delay_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL, field_id))
    {
        int32 ret;
        p_tpoam_global->dm_calc_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL, field_id))
    {
        int32 ret;
        p_tpoam_global->lm_enable_global = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD, field_id))
    {
        int32 ret;
        p_tpoam_global->lm_local_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD, field_id))
    {
        int32 ret;
        p_tpoam_global->lm_remote_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL, field_id))
    {
        int32 ret;
        p_tpoam_global->lm_calc_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER, field_id))
    {
        ctc_task_str2val(str, p_tpoam_global->lm_calc_interval_timer);
    }
    return PM_E_NONE;
}

int32
tbl_tpoam_global_table_parser(char** array, char* key_value,tbl_tpoam_global_t *p_tpoam_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tpoam_global_key_str2val(key_value, p_tpoam_global));

    for(i=1; i<TBL_TPOAM_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tpoam_global_field_value_parser( array[j++], i, p_tpoam_global));
    }

    return PM_E_NONE;
}

int32
tbl_tpoam_global_dump_one(tbl_tpoam_global_t *p_tpoam_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_NODE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_NODE_ID].name,
            p_tpoam_global->node_id);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE].name,
            p_tpoam_global->channel_type);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN].name,
            p_tpoam_global->fast_aps_en);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_RDI_REASON].name,
            p_tpoam_global->rdi_reason);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK].name,
            p_tpoam_global->rdi_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK].name,
            p_tpoam_global->sf_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK].name,
            p_tpoam_global->sd_reason_mask);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX].name, 
            cdb_tpoam_rc_count_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->resouce_count_max));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR].name, 
            cdb_tpoam_rc_count_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->resouce_count_curr));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_tpoam_global->event_rule_map, sizeof(p_tpoam_global->event_rule_map)/4));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS].name, 
            cdb_tpoam_stats_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->oam_glb_stats));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_EXEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_EXEC].name, 
            cdb_tpoam_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->exec));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE].name, 
            cdb_tpoam_trap_cache_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->trap_cache));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_TRAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_TRAP].name, 
            cdb_tpoam_ratelimit_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->trap));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LOG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LOG].name, 
            cdb_tpoam_ratelimit_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_global->log));
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL].name,
            p_tpoam_global->dm_enable_global);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD].name,
            p_tpoam_global->dm_delay_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL].name,
            p_tpoam_global->dm_calc_interval);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL].name,
            p_tpoam_global->lm_enable_global);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD].name,
            p_tpoam_global->lm_local_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD].name,
            p_tpoam_global->lm_remote_threshold);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL].name,
            p_tpoam_global->lm_calc_interval);
    }
    if (FLD_MATCH(TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_tpoam_global->lm_calc_interval_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tpoam_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tpoam_global_t *p_db_tpoam_global = _g_p_tbl_tpoam_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_tpoam_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_tpoam_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tpoam_global_t*
tbl_tpoam_global_init_tpoam_global()
{
    _g_p_tbl_tpoam_global = XCALLOC(MEM_TBL_TPOAM_GLOBAL, sizeof(tbl_tpoam_global_t));
    return _g_p_tbl_tpoam_global;
}


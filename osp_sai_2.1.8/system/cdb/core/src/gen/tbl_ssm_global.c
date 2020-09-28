
#include "proto.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "cdb_data_cmp.h"

tbl_ssm_global_t *_g_p_tbl_ssm_global = NULL;

int32
tbl_ssm_global_set_ssm_global_field_sync(tbl_ssm_global_t *p_ssm_glb, tbl_ssm_global_field_id_t field_id, uint32 sync)
{
    tbl_ssm_global_t *p_db_ssm_glb = _g_p_tbl_ssm_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID:
        sal_memcpy(p_db_ssm_glb->device_clock_id, p_ssm_glb->device_clock_id, sizeof(p_ssm_glb->device_clock_id));
        break;

    case TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE:
        p_db_ssm_glb->synce_enable = p_ssm_glb->synce_enable;
        break;

    case TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY:
        p_db_ssm_glb->local_priority = p_ssm_glb->local_priority;
        break;

    case TBL_SSM_GLOBAL_FLD_OPTION_MODE:
        p_db_ssm_glb->option_mode = p_ssm_glb->option_mode;
        break;

    case TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY:
        p_db_ssm_glb->system_quality = p_ssm_glb->system_quality;
        break;

    case TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY:
        p_db_ssm_glb->current_quality = p_ssm_glb->current_quality;
        break;

    case TBL_SSM_GLOBAL_FLD_DEVICE_ROLE:
        p_db_ssm_glb->device_role = p_ssm_glb->device_role;
        break;

    case TBL_SSM_GLOBAL_FLD_SSM_ENABLE:
        p_db_ssm_glb->ssm_enable = p_ssm_glb->ssm_enable;
        break;

    case TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE:
        p_db_ssm_glb->ssm_extend_enable = p_ssm_glb->ssm_extend_enable;
        break;

    case TBL_SSM_GLOBAL_FLD_HOLDOVER_EN:
        p_db_ssm_glb->holdover_en = p_ssm_glb->holdover_en;
        break;

    case TBL_SSM_GLOBAL_FLD_FREERUN_EN:
        p_db_ssm_glb->freerun_en = p_ssm_glb->freerun_en;
        break;

    case TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX:
        p_db_ssm_glb->select_ifindex = p_ssm_glb->select_ifindex;
        break;

    case TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX:
        p_db_ssm_glb->dpll_selected_ifindex = p_ssm_glb->dpll_selected_ifindex;
        break;

    case TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS:
        p_db_ssm_glb->dpll_current_status = p_ssm_glb->dpll_current_status;
        break;

    case TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT:
        p_db_ssm_glb->force_quality_option1_cnt = p_ssm_glb->force_quality_option1_cnt;
        break;

    case TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT:
        p_db_ssm_glb->force_quality_option2_cnt = p_ssm_glb->force_quality_option2_cnt;
        break;

    case TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT:
        p_db_ssm_glb->clock_recovery_select = p_ssm_glb->clock_recovery_select;
        break;

    case TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX:
        p_db_ssm_glb->next_selected_ifindex = p_ssm_glb->next_selected_ifindex;
        break;

    case TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN:
        p_db_ssm_glb->select_if_linkup_restore_en = p_ssm_glb->select_if_linkup_restore_en;
        break;

    case TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN:
        p_db_ssm_glb->select_if_priority_change_en = p_ssm_glb->select_if_priority_change_en;
        break;

    case TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN:
        p_db_ssm_glb->select_if_quality_change_en = p_ssm_glb->select_if_quality_change_en;
        break;

    case TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME:
        p_db_ssm_glb->holdoff_time = p_ssm_glb->holdoff_time;
        break;

    case TBL_SSM_GLOBAL_FLD_WTR_TIME:
        p_db_ssm_glb->wtr_time = p_ssm_glb->wtr_time;
        break;

    case TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER:
        p_db_ssm_glb->holdoff_send_timer = p_ssm_glb->holdoff_send_timer;
        break;

    case TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER:
        p_db_ssm_glb->wtr_send_timer = p_ssm_glb->wtr_send_timer;
        break;

    case TBL_SSM_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_ssm_glb, p_ssm_glb, sizeof(tbl_ssm_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SSM_GLOBAL, field_id, p_db_ssm_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SSM_GLOBAL, field_id, p_db_ssm_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ssm_global_set_ssm_global_field(tbl_ssm_global_t *p_ssm_glb, tbl_ssm_global_field_id_t field_id)
{
    return tbl_ssm_global_set_ssm_global_field_sync(p_ssm_glb, field_id, TRUE);
}

tbl_ssm_global_t*
tbl_ssm_global_get_ssm_global()
{
    return _g_p_tbl_ssm_global;
}

int32
tbl_ssm_global_key_str2val(char *str, tbl_ssm_global_t *p_ssm_glb)
{
    return PM_E_NONE;
}

char*
tbl_ssm_global_key_name_dump(tbl_ssm_global_t *p_ssm_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ssm_global_key_value_dump(tbl_ssm_global_t *p_ssm_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ssm_global_field_name_dump(tbl_ssm_global_t *p_ssm_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SSM_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ssm_global_field_value_dump(tbl_ssm_global_t *p_ssm_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID, field_id))
    {
        cdb_ssm_clock_id_val2str(str, MAX_CMD_STR_LEN, p_ssm_glb->device_clock_id);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->synce_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->local_priority);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_OPTION_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->option_mode);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->system_quality);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->current_quality);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_ROLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->device_role);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->ssm_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->ssm_extend_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOVER_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->holdover_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FREERUN_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->freerun_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->select_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->dpll_selected_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->dpll_current_status);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->force_quality_option1_cnt);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->force_quality_option2_cnt);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->clock_recovery_select);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->next_selected_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->select_if_linkup_restore_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->select_if_priority_change_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->select_if_quality_change_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->holdoff_time);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_glb->wtr_time);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ssm_glb->holdoff_send_timer);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ssm_glb->wtr_send_timer);
    }
    return str;
}

char**
tbl_ssm_global_table_dump(tbl_ssm_global_t *p_ssm_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ssm_global_key_name_dump(p_ssm_glb, buf));
    for(i=1; i<TBL_SSM_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ssm_global_field_name_dump(p_ssm_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ssm_global_field_value_dump(p_ssm_glb, i, buf));
    }
    return str;
}

int32
tbl_ssm_global_field_value_parser(char *str, int32 field_id, tbl_ssm_global_t *p_ssm_glb)
{
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID, field_id))
    {
        cdb_ssm_clock_id_str2val(str, p_ssm_glb->device_clock_id);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE, field_id))
    {
        int32 ret;
        p_ssm_glb->synce_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY, field_id))
    {
        int32 ret;
        p_ssm_glb->local_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_OPTION_MODE, field_id))
    {
        int32 ret;
        p_ssm_glb->option_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_glb->system_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_glb->current_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_ROLE, field_id))
    {
        int32 ret;
        p_ssm_glb->device_role = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_ENABLE, field_id))
    {
        int32 ret;
        p_ssm_glb->ssm_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE, field_id))
    {
        int32 ret;
        p_ssm_glb->ssm_extend_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOVER_EN, field_id))
    {
        int32 ret;
        p_ssm_glb->holdover_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FREERUN_EN, field_id))
    {
        int32 ret;
        p_ssm_glb->freerun_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX, field_id))
    {
        int32 ret;
        p_ssm_glb->select_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX, field_id))
    {
        int32 ret;
        p_ssm_glb->dpll_selected_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS, field_id))
    {
        int32 ret;
        p_ssm_glb->dpll_current_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT, field_id))
    {
        int32 ret;
        p_ssm_glb->force_quality_option1_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT, field_id))
    {
        int32 ret;
        p_ssm_glb->force_quality_option2_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT, field_id))
    {
        int32 ret;
        p_ssm_glb->clock_recovery_select = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX, field_id))
    {
        int32 ret;
        p_ssm_glb->next_selected_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN, field_id))
    {
        int32 ret;
        p_ssm_glb->select_if_linkup_restore_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN, field_id))
    {
        int32 ret;
        p_ssm_glb->select_if_priority_change_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN, field_id))
    {
        int32 ret;
        p_ssm_glb->select_if_quality_change_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME, field_id))
    {
        int32 ret;
        p_ssm_glb->holdoff_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_TIME, field_id))
    {
        int32 ret;
        p_ssm_glb->wtr_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ssm_glb->holdoff_send_timer);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ssm_glb->wtr_send_timer);
    }
    return PM_E_NONE;
}

int32
tbl_ssm_global_table_parser(char** array, char* key_value,tbl_ssm_global_t *p_ssm_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ssm_global_key_str2val(key_value, p_ssm_glb));

    for(i=1; i<TBL_SSM_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ssm_global_field_value_parser( array[j++], i, p_ssm_glb));
    }

    return PM_E_NONE;
}

int32
tbl_ssm_global_dump_one(tbl_ssm_global_t *p_ssm_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID].name, 
            cdb_ssm_clock_id_val2str(buf, MAX_CMD_STR_LEN, p_ssm_glb->device_clock_id));
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE].name,
            p_ssm_glb->synce_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY].name,
            p_ssm_glb->local_priority);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_OPTION_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_OPTION_MODE].name,
            p_ssm_glb->option_mode);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY].name,
            p_ssm_glb->system_quality);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY].name,
            p_ssm_glb->current_quality);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DEVICE_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_DEVICE_ROLE].name,
            p_ssm_glb->device_role);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SSM_ENABLE].name,
            p_ssm_glb->ssm_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE].name,
            p_ssm_glb->ssm_extend_enable);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOVER_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_HOLDOVER_EN].name,
            p_ssm_glb->holdover_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FREERUN_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_FREERUN_EN].name,
            p_ssm_glb->freerun_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX].name,
            p_ssm_glb->select_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX].name,
            p_ssm_glb->dpll_selected_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS].name,
            p_ssm_glb->dpll_current_status);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT].name,
            p_ssm_glb->force_quality_option1_cnt);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT].name,
            p_ssm_glb->force_quality_option2_cnt);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT].name,
            p_ssm_glb->clock_recovery_select);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX].name,
            p_ssm_glb->next_selected_ifindex);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN].name,
            p_ssm_glb->select_if_linkup_restore_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN].name,
            p_ssm_glb->select_if_priority_change_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN].name,
            p_ssm_glb->select_if_quality_change_en);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME].name,
            p_ssm_glb->holdoff_time);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_WTR_TIME].name,
            p_ssm_glb->wtr_time);
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ssm_glb->holdoff_send_timer));
    }
    if (FLD_MATCH(TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ssm_glb->wtr_send_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ssm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ssm_global_t *p_db_ssm_glb = _g_p_tbl_ssm_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ssm_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ssm_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ssm_global_t*
tbl_ssm_global_init_ssm_global()
{
    _g_p_tbl_ssm_global = XCALLOC(MEM_TBL_SSM_GLOBAL, sizeof(tbl_ssm_global_t));
    return _g_p_tbl_ssm_global;
}


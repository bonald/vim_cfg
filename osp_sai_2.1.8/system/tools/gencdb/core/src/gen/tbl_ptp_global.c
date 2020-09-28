
#include "proto.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "cdb_data_cmp.h"

tbl_ptp_global_t *_g_p_tbl_ptp_global = NULL;

int32
tbl_ptp_global_set_ptp_global_field_sync(tbl_ptp_global_t *p_global, tbl_ptp_global_field_id_t field_id, uint32 sync)
{
    tbl_ptp_global_t *p_db_global = _g_p_tbl_ptp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PTP_GLOBAL_FLD_DEFAULT_DS:
        sal_memcpy(&p_db_global->default_ds, &p_global->default_ds, sizeof(p_global->default_ds));
        break;

    case TBL_PTP_GLOBAL_FLD_CURRENT_DS:
        sal_memcpy(&p_db_global->current_ds, &p_global->current_ds, sizeof(p_global->current_ds));
        break;

    case TBL_PTP_GLOBAL_FLD_PARENT_DS:
        sal_memcpy(&p_db_global->parent_ds, &p_global->parent_ds, sizeof(p_global->parent_ds));
        break;

    case TBL_PTP_GLOBAL_FLD_TIME_PROP_DS:
        sal_memcpy(&p_db_global->time_prop_ds, &p_global->time_prop_ds, sizeof(p_global->time_prop_ds));
        break;

    case TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN:
        sal_memcpy(&p_db_global->time_prop_intern, &p_global->time_prop_intern, sizeof(p_global->time_prop_intern));
        break;

    case TBL_PTP_GLOBAL_FLD_DEVICE_TYPE:
        p_db_global->device_type = p_global->device_type;
        break;

    case TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE:
        p_db_global->global_enable = p_global->global_enable;
        break;

    case TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL:
        p_db_global->master_min_delay_req_interval = p_global->master_min_delay_req_interval;
        break;

    case TBL_PTP_GLOBAL_FLD_C:
        sal_memcpy(&p_db_global->c, &p_global->c, sizeof(p_global->c));
        break;

    case TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER:
        p_db_global->p_internal_timer = p_global->p_internal_timer;
        break;

    case TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT:
        sal_memcpy(p_db_global->current_slave_port, p_global->current_slave_port, sizeof(p_global->current_slave_port));
        break;

    case TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME:
        p_db_global->set_system_time = p_global->set_system_time;
        break;

    case TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP:
        p_db_global->masterChangeTrap = p_global->masterChangeTrap;
        break;

    case TBL_PTP_GLOBAL_FLD_STATECHANGETRAP:
        p_db_global->stateChangeTrap = p_global->stateChangeTrap;
        break;

    case TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE:
        p_db_global->accuracyChange = p_global->accuracyChange;
        break;

    case TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME:
        p_db_global->glb_enable_hw_msec_time = p_global->glb_enable_hw_msec_time;
        break;

    case TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME:
        p_db_global->glb_enable_pf_msec_time = p_global->glb_enable_pf_msec_time;
        break;

    case TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET:
        p_db_global->total_offset = p_global->total_offset;
        break;

    case TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET:
        p_db_global->hsrv_adjust_offset = p_global->hsrv_adjust_offset;
        break;

    case TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO:
        p_db_global->hsrv_adjust_ffo = p_global->hsrv_adjust_ffo;
        break;

    case TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME:
        p_db_global->hsrv_update_system_time = p_global->hsrv_update_system_time;
        break;

    case TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE:
        p_db_global->hsrv_device_type = p_global->hsrv_device_type;
        break;

    case TBL_PTP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_global, p_global, sizeof(tbl_ptp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PTP_GLOBAL, field_id, p_db_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PTP_GLOBAL, field_id, p_db_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_global_set_ptp_global_field(tbl_ptp_global_t *p_global, tbl_ptp_global_field_id_t field_id)
{
    return tbl_ptp_global_set_ptp_global_field_sync(p_global, field_id, TRUE);
}

tbl_ptp_global_t*
tbl_ptp_global_get_ptp_global()
{
    return _g_p_tbl_ptp_global;
}

int32
tbl_ptp_global_key_str2val(char *str, tbl_ptp_global_t *p_global)
{
    return PM_E_NONE;
}

char*
tbl_ptp_global_key_name_dump(tbl_ptp_global_t *p_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ptp_global_key_value_dump(tbl_ptp_global_t *p_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ptp_global_field_name_dump(tbl_ptp_global_t *p_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PTP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ptp_global_field_value_dump(tbl_ptp_global_t *p_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEFAULT_DS, field_id))
    {
        cdb_ptp_default_val2str(str, MAX_CMD_STR_LEN, &p_global->default_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_DS, field_id))
    {
        cdb_ptp_current_val2str(str, MAX_CMD_STR_LEN, &p_global->current_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_PARENT_DS, field_id))
    {
        cdb_ptp_parent_val2str(str, MAX_CMD_STR_LEN, &p_global->parent_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_DS, field_id))
    {
        cdb_ptp_time_properties_val2str(str, MAX_CMD_STR_LEN, &p_global->time_prop_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN, field_id))
    {
        cdb_ptp_time_properties_val2str(str, MAX_CMD_STR_LEN, &p_global->time_prop_intern);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEVICE_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, p_global->device_type));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_global->global_enable);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL, field_id))
    {
        sal_sprintf(str, "%d", p_global->master_min_delay_req_interval);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_C, field_id))
    {
        cdb_ptp_global_c_val2str(str, MAX_CMD_STR_LEN, &p_global->c);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_global->p_internal_timer);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT, field_id))
    {
        sal_sprintf(str, "%s", p_global->current_slave_port);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_global->set_system_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP, field_id))
    {
        sal_sprintf(str, "%u", p_global->masterChangeTrap);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_STATECHANGETRAP, field_id))
    {
        sal_sprintf(str, "%u", p_global->stateChangeTrap);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE, field_id))
    {
        sal_sprintf(str, "%u", p_global->accuracyChange);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_global->glb_enable_hw_msec_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_global->glb_enable_pf_msec_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET, field_id))
    {
        sal_sprintf(str, "%"PRId64, p_global->total_offset);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET, field_id))
    {
        sal_sprintf(str, "%"PRId64, p_global->hsrv_adjust_offset);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO, field_id))
    {
        sal_sprintf(str, "%"PRId64, p_global->hsrv_adjust_ffo);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME, field_id))
    {
        sal_sprintf(str, "%d", p_global->hsrv_update_system_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, p_global->hsrv_device_type));
    }
    return str;
}

char**
tbl_ptp_global_table_dump(tbl_ptp_global_t *p_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ptp_global_key_name_dump(p_global, buf));
    for(i=1; i<TBL_PTP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ptp_global_field_name_dump(p_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ptp_global_field_value_dump(p_global, i, buf));
    }
    return str;
}

int32
tbl_ptp_global_field_value_parser(char *str, int32 field_id, tbl_ptp_global_t *p_global)
{
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEFAULT_DS, field_id))
    {
        cdb_ptp_default_str2val(str, &p_global->default_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_DS, field_id))
    {
        cdb_ptp_current_str2val(str, &p_global->current_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_PARENT_DS, field_id))
    {
        cdb_ptp_parent_str2val(str, &p_global->parent_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_DS, field_id))
    {
        cdb_ptp_time_properties_str2val(str, &p_global->time_prop_ds);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN, field_id))
    {
        cdb_ptp_time_properties_str2val(str, &p_global->time_prop_intern);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEVICE_TYPE, field_id))
    {
        cdb_enum_str2val(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE, field_id))
    {
        int32 ret;
        p_global->global_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL, field_id))
    {
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_C, field_id))
    {
        cdb_ptp_global_c_str2val(str, &p_global->c);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER, field_id))
    {
        ctc_task_str2val(str, p_global->p_internal_timer);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT, field_id))
    {
        sal_strcpy(p_global->current_slave_port, str);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME, field_id))
    {
        int32 ret;
        p_global->set_system_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP, field_id))
    {
        int32 ret;
        p_global->masterChangeTrap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_STATECHANGETRAP, field_id))
    {
        int32 ret;
        p_global->stateChangeTrap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE, field_id))
    {
        int32 ret;
        p_global->accuracyChange = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME, field_id))
    {
        int32 ret;
        p_global->glb_enable_hw_msec_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME, field_id))
    {
        int32 ret;
        p_global->glb_enable_pf_msec_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET, field_id))
    {
        int32 ret;
        p_global->total_offset = cdb_int64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET, field_id))
    {
        int32 ret;
        p_global->hsrv_adjust_offset = cdb_int64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO, field_id))
    {
        int32 ret;
        p_global->hsrv_adjust_ffo = cdb_int64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME, field_id))
    {
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE, field_id))
    {
        cdb_enum_str2val(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, str);
    }
    return PM_E_NONE;
}

int32
tbl_ptp_global_table_parser(char** array, char* key_value,tbl_ptp_global_t *p_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ptp_global_key_str2val(key_value, p_global));

    for(i=1; i<TBL_PTP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ptp_global_field_value_parser( array[j++], i, p_global));
    }

    return PM_E_NONE;
}

int32
tbl_ptp_global_dump_one(tbl_ptp_global_t *p_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEFAULT_DS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_DEFAULT_DS].name, 
            cdb_ptp_default_val2str(buf, MAX_CMD_STR_LEN, &p_global->default_ds));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_DS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_CURRENT_DS].name, 
            cdb_ptp_current_val2str(buf, MAX_CMD_STR_LEN, &p_global->current_ds));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_PARENT_DS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_PARENT_DS].name, 
            cdb_ptp_parent_val2str(buf, MAX_CMD_STR_LEN, &p_global->parent_ds));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_DS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_TIME_PROP_DS].name, 
            cdb_ptp_time_properties_val2str(buf, MAX_CMD_STR_LEN, &p_global->time_prop_ds));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN].name, 
            cdb_ptp_time_properties_val2str(buf, MAX_CMD_STR_LEN, &p_global->time_prop_intern));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_DEVICE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_DEVICE_TYPE].name, 
            cdb_enum_val2str(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, p_global->device_type));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE].name,
            p_global->global_enable);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL].name,
            p_global->master_min_delay_req_interval);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_C, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_C].name, 
            cdb_ptp_global_c_val2str(buf, MAX_CMD_STR_LEN, &p_global->c));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_global->p_internal_timer));
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT].name,
            p_global->current_slave_port);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME].name,
            p_global->set_system_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP].name,
            p_global->masterChangeTrap);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_STATECHANGETRAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_STATECHANGETRAP].name,
            p_global->stateChangeTrap);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE].name,
            p_global->accuracyChange);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME].name,
            p_global->glb_enable_hw_msec_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME].name,
            p_global->glb_enable_pf_msec_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRId64, p_tbl_info->field[TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET].name,
            p_global->total_offset);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRId64, p_tbl_info->field[TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET].name,
            p_global->hsrv_adjust_offset);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRId64, p_tbl_info->field[TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO].name,
            p_global->hsrv_adjust_ffo);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME].name,
            p_global->hsrv_update_system_time);
    }
    if (FLD_MATCH(TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE].name, 
            cdb_enum_val2str(ptp_device_type_strs, PTP_DEVICE_TYPE_MAX, p_global->hsrv_device_type));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ptp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ptp_global_t *p_db_global = _g_p_tbl_ptp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ptp_global_t*
tbl_ptp_global_init_ptp_global()
{
    _g_p_tbl_ptp_global = XCALLOC(MEM_TBL_PTP_GLOBAL, sizeof(tbl_ptp_global_t));
    return _g_p_tbl_ptp_global;
}


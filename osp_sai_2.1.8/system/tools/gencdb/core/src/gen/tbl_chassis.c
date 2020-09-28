
#include "proto.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "cdb_data_cmp.h"

tbl_chassis_t *_g_p_tbl_chassis = NULL;

int32
tbl_chassis_set_chassis_field_sync(tbl_chassis_t *p_chassis, tbl_chassis_field_id_t field_id, uint32 sync)
{
    tbl_chassis_t *p_db_chassis = _g_p_tbl_chassis;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CHASSIS_FLD_TYPE:
        p_db_chassis->type = p_chassis->type;
        break;

    case TBL_CHASSIS_FLD_SLOT_NUM:
        p_db_chassis->slot_num = p_chassis->slot_num;
        break;

    case TBL_CHASSIS_FLD_CUR_STM_MODE:
        p_db_chassis->cur_stm_mode = p_chassis->cur_stm_mode;
        break;

    case TBL_CHASSIS_FLD_NEXT_STM_MODE:
        p_db_chassis->next_stm_mode = p_chassis->next_stm_mode;
        break;

    case TBL_CHASSIS_FLD_MAC_NUM:
        p_db_chassis->mac_num = p_chassis->mac_num;
        break;

    case TBL_CHASSIS_FLD_SYS_MAC:
        sal_memcpy(p_db_chassis->sys_mac, p_chassis->sys_mac, sizeof(p_chassis->sys_mac));
        break;

    case TBL_CHASSIS_FLD_LOCATION_LED:
        p_db_chassis->location_led = p_chassis->location_led;
        break;

    case TBL_CHASSIS_FLD_RESET_REBOOT_INFO:
        p_db_chassis->reset_reboot_info = p_chassis->reset_reboot_info;
        break;

    case TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO:
        p_db_chassis->support_reboot_info = p_chassis->support_reboot_info;
        break;

    case TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM:
        p_db_chassis->one_minute_reboot_alarm = p_chassis->one_minute_reboot_alarm;
        break;

    case TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER:
        p_db_chassis->schedule_reboot_timer = p_chassis->schedule_reboot_timer;
        break;

    case TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER:
        p_db_chassis->schedule_reboot_alarm_timer = p_chassis->schedule_reboot_alarm_timer;
        break;

    case TBL_CHASSIS_FLD_SCHEDULE_REBOOT:
        p_db_chassis->schedule_reboot = p_chassis->schedule_reboot;
        break;

    case TBL_CHASSIS_FLD_MAX:
        sal_memcpy(p_db_chassis, p_chassis, sizeof(tbl_chassis_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CHASSIS, field_id, p_db_chassis);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CHASSIS, field_id, p_db_chassis);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_chassis_set_chassis_field(tbl_chassis_t *p_chassis, tbl_chassis_field_id_t field_id)
{
    return tbl_chassis_set_chassis_field_sync(p_chassis, field_id, TRUE);
}

tbl_chassis_t*
tbl_chassis_get_chassis()
{
    return _g_p_tbl_chassis;
}

int32
tbl_chassis_key_str2val(char *str, tbl_chassis_t *p_chassis)
{
    return PM_E_NONE;
}

char*
tbl_chassis_key_name_dump(tbl_chassis_t *p_chassis, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHASSIS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_chassis_key_value_dump(tbl_chassis_t *p_chassis, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_chassis_field_name_dump(tbl_chassis_t *p_chassis, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CHASSIS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CHASSIS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_chassis_field_value_dump(tbl_chassis_t *p_chassis, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CHASSIS_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_chassis->type);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SLOT_NUM, field_id))
    {
        sal_sprintf(str, "%d", p_chassis->slot_num);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_CUR_STM_MODE, field_id))
    {
        sal_sprintf(str, "%d", p_chassis->cur_stm_mode);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_NEXT_STM_MODE, field_id))
    {
        sal_sprintf(str, "%d", p_chassis->next_stm_mode);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_MAC_NUM, field_id))
    {
        sal_sprintf(str, "%d", p_chassis->mac_num);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SYS_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_chassis->sys_mac);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_LOCATION_LED, field_id))
    {
        sal_sprintf(str, "%u", p_chassis->location_led);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_RESET_REBOOT_INFO, field_id))
    {
        sal_sprintf(str, "%u", p_chassis->reset_reboot_info);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO, field_id))
    {
        sal_sprintf(str, "%u", p_chassis->support_reboot_info);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM, field_id))
    {
        sal_sprintf(str, "%u", p_chassis->one_minute_reboot_alarm);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_chassis->schedule_reboot_timer);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_chassis->schedule_reboot_alarm_timer);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT, field_id))
    {
        sal_sprintf(str, "%u", p_chassis->schedule_reboot);
    }
    return str;
}

char**
tbl_chassis_table_dump(tbl_chassis_t *p_chassis, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_chassis_key_name_dump(p_chassis, buf));
    for(i=1; i<TBL_CHASSIS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_chassis_field_name_dump(p_chassis, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_chassis_field_value_dump(p_chassis, i, buf));
    }
    return str;
}

int32
tbl_chassis_field_value_parser(char *str, int32 field_id, tbl_chassis_t *p_chassis)
{
    if (FLD_MATCH(TBL_CHASSIS_FLD_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SLOT_NUM, field_id))
    {
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_CUR_STM_MODE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_NEXT_STM_MODE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_MAC_NUM, field_id))
    {
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SYS_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_chassis->sys_mac, str);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_LOCATION_LED, field_id))
    {
        int32 ret;
        p_chassis->location_led = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_RESET_REBOOT_INFO, field_id))
    {
        int32 ret;
        p_chassis->reset_reboot_info = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO, field_id))
    {
        int32 ret;
        p_chassis->support_reboot_info = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM, field_id))
    {
        int32 ret;
        p_chassis->one_minute_reboot_alarm = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_chassis->schedule_reboot_timer);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER, field_id))
    {
        ctc_task_str2val(str, p_chassis->schedule_reboot_alarm_timer);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT, field_id))
    {
        int32 ret;
        p_chassis->schedule_reboot = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_chassis_table_parser(char** array, char* key_value,tbl_chassis_t *p_chassis)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHASSIS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_chassis_key_str2val(key_value, p_chassis));

    for(i=1; i<TBL_CHASSIS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_chassis_field_value_parser( array[j++], i, p_chassis));
    }

    return PM_E_NONE;
}

int32
tbl_chassis_dump_one(tbl_chassis_t *p_chassis, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHASSIS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CHASSIS_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CHASSIS_FLD_TYPE].name,
            p_chassis->type);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SLOT_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CHASSIS_FLD_SLOT_NUM].name,
            p_chassis->slot_num);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_CUR_STM_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CHASSIS_FLD_CUR_STM_MODE].name,
            p_chassis->cur_stm_mode);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_NEXT_STM_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CHASSIS_FLD_NEXT_STM_MODE].name,
            p_chassis->next_stm_mode);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_MAC_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CHASSIS_FLD_MAC_NUM].name,
            p_chassis->mac_num);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SYS_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CHASSIS_FLD_SYS_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_chassis->sys_mac));
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_LOCATION_LED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHASSIS_FLD_LOCATION_LED].name,
            p_chassis->location_led);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_RESET_REBOOT_INFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHASSIS_FLD_RESET_REBOOT_INFO].name,
            p_chassis->reset_reboot_info);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO].name,
            p_chassis->support_reboot_info);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM].name,
            p_chassis->one_minute_reboot_alarm);
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_chassis->schedule_reboot_timer));
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_chassis->schedule_reboot_alarm_timer));
    }
    if (FLD_MATCH(TBL_CHASSIS_FLD_SCHEDULE_REBOOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHASSIS_FLD_SCHEDULE_REBOOT].name,
            p_chassis->schedule_reboot);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_chassis_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_chassis_t *p_db_chassis = _g_p_tbl_chassis;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_chassis)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_chassis, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_chassis_t*
tbl_chassis_init_chassis()
{
    _g_p_tbl_chassis = XCALLOC(MEM_TBL_CHASSIS, sizeof(tbl_chassis_t));
    return _g_p_tbl_chassis;
}


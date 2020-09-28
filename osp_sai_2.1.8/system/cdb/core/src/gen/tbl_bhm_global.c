
#include "proto.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "cdb_data_cmp.h"

tbl_bhm_global_t *_g_p_tbl_bhm_global = NULL;

int32
tbl_bhm_global_set_bhm_global_field_sync(tbl_bhm_global_t *p_bhm_glb, tbl_bhm_global_field_id_t field_id, uint32 sync)
{
    tbl_bhm_global_t *p_db_bhm_glb = _g_p_tbl_bhm_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD:
        p_db_bhm_glb->hw_watchdog_fd = p_bhm_glb->hw_watchdog_fd;
        break;

    case TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN:
        p_db_bhm_glb->hw_watchdog_feed_en = p_bhm_glb->hw_watchdog_feed_en;
        break;

    case TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE:
        p_db_bhm_glb->hw_watchdog_enable = p_bhm_glb->hw_watchdog_enable;
        break;

    case TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE:
        p_db_bhm_glb->sw_watchdog_enable = p_bhm_glb->sw_watchdog_enable;
        break;

    case TBL_BHM_GLOBAL_FLD_REACTIVE:
        p_db_bhm_glb->reactive = p_bhm_glb->reactive;
        break;

    case TBL_BHM_GLOBAL_FLD_IS_RELOAD:
        p_db_bhm_glb->is_reload = p_bhm_glb->is_reload;
        break;

    case TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN:
        p_db_bhm_glb->is_shutdown = p_bhm_glb->is_shutdown;
        break;

    case TBL_BHM_GLOBAL_FLD_IS_WARNING:
        p_db_bhm_glb->is_warning = p_bhm_glb->is_warning;
        break;

    case TBL_BHM_GLOBAL_FLD_REBOOT:
        p_db_bhm_glb->reboot = p_bhm_glb->reboot;
        break;

    case TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER:
        p_db_bhm_glb->sw_watchdog_timer = p_bhm_glb->sw_watchdog_timer;
        break;

    case TBL_BHM_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_bhm_glb, p_bhm_glb, sizeof(tbl_bhm_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BHM_GLOBAL, field_id, p_db_bhm_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BHM_GLOBAL, field_id, p_db_bhm_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bhm_global_set_bhm_global_field(tbl_bhm_global_t *p_bhm_glb, tbl_bhm_global_field_id_t field_id)
{
    return tbl_bhm_global_set_bhm_global_field_sync(p_bhm_glb, field_id, TRUE);
}

tbl_bhm_global_t*
tbl_bhm_global_get_bhm_global()
{
    return _g_p_tbl_bhm_global;
}

int32
tbl_bhm_global_key_str2val(char *str, tbl_bhm_global_t *p_bhm_glb)
{
    return PM_E_NONE;
}

char*
tbl_bhm_global_key_name_dump(tbl_bhm_global_t *p_bhm_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_bhm_global_key_value_dump(tbl_bhm_global_t *p_bhm_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_bhm_global_field_name_dump(tbl_bhm_global_t *p_bhm_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BHM_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_bhm_global_field_value_dump(tbl_bhm_global_t *p_bhm_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD, field_id))
    {
        sal_sprintf(str, "%d", p_bhm_glb->hw_watchdog_fd);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->hw_watchdog_feed_en);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->hw_watchdog_enable);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->sw_watchdog_enable);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REACTIVE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_bhm_action_strs, GLB_BHM_ACTION_MAX, p_bhm_glb->reactive));
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_RELOAD, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->is_reload);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->is_shutdown);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_WARNING, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_glb->is_warning);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REBOOT, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, p_bhm_glb->reboot));
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_bhm_glb->sw_watchdog_timer);
    }
    return str;
}

char**
tbl_bhm_global_table_dump(tbl_bhm_global_t *p_bhm_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_bhm_global_key_name_dump(p_bhm_glb, buf));
    for(i=1; i<TBL_BHM_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_bhm_global_field_name_dump(p_bhm_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_bhm_global_field_value_dump(p_bhm_glb, i, buf));
    }
    return str;
}

int32
tbl_bhm_global_field_value_parser(char *str, int32 field_id, tbl_bhm_global_t *p_bhm_glb)
{
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD, field_id))
    {
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN, field_id))
    {
        int32 ret;
        p_bhm_glb->hw_watchdog_feed_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE, field_id))
    {
        int32 ret;
        p_bhm_glb->hw_watchdog_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE, field_id))
    {
        int32 ret;
        p_bhm_glb->sw_watchdog_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REACTIVE, field_id))
    {
        cdb_enum_str2val(glb_bhm_action_strs, GLB_BHM_ACTION_MAX, str);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_RELOAD, field_id))
    {
        int32 ret;
        p_bhm_glb->is_reload = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN, field_id))
    {
        int32 ret;
        p_bhm_glb->is_shutdown = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_WARNING, field_id))
    {
        int32 ret;
        p_bhm_glb->is_warning = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REBOOT, field_id))
    {
        cdb_enum_str2val(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER, field_id))
    {
        ctc_task_str2val(str, p_bhm_glb->sw_watchdog_timer);
    }
    return PM_E_NONE;
}

int32
tbl_bhm_global_table_parser(char** array, char* key_value,tbl_bhm_global_t *p_bhm_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_bhm_global_key_str2val(key_value, p_bhm_glb));

    for(i=1; i<TBL_BHM_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_bhm_global_field_value_parser( array[j++], i, p_bhm_glb));
    }

    return PM_E_NONE;
}

int32
tbl_bhm_global_dump_one(tbl_bhm_global_t *p_bhm_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD].name,
            p_bhm_glb->hw_watchdog_fd);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN].name,
            p_bhm_glb->hw_watchdog_feed_en);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE].name,
            p_bhm_glb->hw_watchdog_enable);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE].name,
            p_bhm_glb->sw_watchdog_enable);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REACTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_REACTIVE].name, 
            cdb_enum_val2str(glb_bhm_action_strs, GLB_BHM_ACTION_MAX, p_bhm_glb->reactive));
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_RELOAD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_IS_RELOAD].name,
            p_bhm_glb->is_reload);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN].name,
            p_bhm_glb->is_shutdown);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_IS_WARNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_IS_WARNING].name,
            p_bhm_glb->is_warning);
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_REBOOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_REBOOT].name, 
            cdb_enum_val2str(glb_warmboot_type_strs, GLB_WARMBOOT_TYPE_MAX, p_bhm_glb->reboot));
    }
    if (FLD_MATCH(TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_bhm_glb->sw_watchdog_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_bhm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_bhm_global_t *p_db_bhm_glb = _g_p_tbl_bhm_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_bhm_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_bhm_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_bhm_global_t*
tbl_bhm_global_init_bhm_global()
{
    _g_p_tbl_bhm_global = XCALLOC(MEM_TBL_BHM_GLOBAL, sizeof(tbl_bhm_global_t));
    return _g_p_tbl_bhm_global;
}


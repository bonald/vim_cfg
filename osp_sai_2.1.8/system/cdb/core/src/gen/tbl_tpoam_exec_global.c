
#include "proto.h"
#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "cdb_data_cmp.h"

tbl_tpoam_exec_global_t *_g_p_tbl_tpoam_exec_global = NULL;

int32
tbl_tpoam_exec_global_set_tpoam_exec_global_field_sync(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_tpoam_exec_global_field_id_t field_id, uint32 sync)
{
    tbl_tpoam_exec_global_t *p_db_tpoam_exec_global = _g_p_tbl_tpoam_exec_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE:
        p_db_tpoam_exec_global->test_type = p_tpoam_exec_global->test_type;
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID:
        p_db_tpoam_exec_global->oam_session_id = p_tpoam_exec_global->oam_session_id;
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_PID:
        p_db_tpoam_exec_global->pid = p_tpoam_exec_global->pid;
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK:
        p_db_tpoam_exec_global->sock = p_tpoam_exec_global->sock;
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM:
        sal_memcpy(&p_db_tpoam_exec_global->exec_lm, &p_tpoam_exec_global->exec_lm, sizeof(p_tpoam_exec_global->exec_lm));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP:
        sal_memcpy(&p_db_tpoam_exec_global->exec_lb_rmep, &p_tpoam_exec_global->exec_lb_rmep, sizeof(p_tpoam_exec_global->exec_lb_rmep));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP:
        sal_memcpy(&p_db_tpoam_exec_global->exec_lb_mip, &p_tpoam_exec_global->exec_lb_mip, sizeof(p_tpoam_exec_global->exec_lb_mip));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY:
        sal_memcpy(&p_db_tpoam_exec_global->exec_lb_discovery, &p_tpoam_exec_global->exec_lb_discovery, sizeof(p_tpoam_exec_global->exec_lb_discovery));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM:
        sal_memcpy(&p_db_tpoam_exec_global->exec_dmm, &p_tpoam_exec_global->exec_dmm, sizeof(p_tpoam_exec_global->exec_dmm));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM:
        sal_memcpy(&p_db_tpoam_exec_global->exec_1dm, &p_tpoam_exec_global->exec_1dm, sizeof(p_tpoam_exec_global->exec_1dm));
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER:
        p_db_tpoam_exec_global->p_t_send_lb_msg_timer = p_tpoam_exec_global->p_t_send_lb_msg_timer;
        break;

    case TBL_TPOAM_EXEC_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_tpoam_exec_global, p_tpoam_exec_global, sizeof(tbl_tpoam_exec_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TPOAM_EXEC_GLOBAL, field_id, p_db_tpoam_exec_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TPOAM_EXEC_GLOBAL, field_id, p_db_tpoam_exec_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tpoam_exec_global_set_tpoam_exec_global_field(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_tpoam_exec_global_field_id_t field_id)
{
    return tbl_tpoam_exec_global_set_tpoam_exec_global_field_sync(p_tpoam_exec_global, field_id, TRUE);
}

tbl_tpoam_exec_global_t*
tbl_tpoam_exec_global_get_tpoam_exec_global()
{
    return _g_p_tbl_tpoam_exec_global;
}

int32
tbl_tpoam_exec_global_key_str2val(char *str, tbl_tpoam_exec_global_t *p_tpoam_exec_global)
{
    return PM_E_NONE;
}

char*
tbl_tpoam_exec_global_key_name_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_EXEC_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_tpoam_exec_global_key_value_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_tpoam_exec_global_field_name_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_EXEC_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TPOAM_EXEC_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tpoam_exec_global_field_value_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_exec_global->test_type);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_exec_global->oam_session_id);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_PID, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_exec_global->pid);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK, field_id))
    {
        sal_sprintf(str, "%u", p_tpoam_exec_global->sock);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM, field_id))
    {
        cdb_tpoam_lmm_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP, field_id))
    {
        cdb_tpoam_lb_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_rmep);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP, field_id))
    {
        cdb_tpoam_lb_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_mip);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY, field_id))
    {
        cdb_tpoam_lb_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_discovery);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM, field_id))
    {
        cdb_tpoam_1dm_dmm_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_dmm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM, field_id))
    {
        cdb_tpoam_1dm_dmm_exec_val2str(str, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_1dm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_tpoam_exec_global->p_t_send_lb_msg_timer);
    }
    return str;
}

char**
tbl_tpoam_exec_global_table_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tpoam_exec_global_key_name_dump(p_tpoam_exec_global, buf));
    for(i=1; i<TBL_TPOAM_EXEC_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tpoam_exec_global_field_name_dump(p_tpoam_exec_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tpoam_exec_global_field_value_dump(p_tpoam_exec_global, i, buf));
    }
    return str;
}

int32
tbl_tpoam_exec_global_field_value_parser(char *str, int32 field_id, tbl_tpoam_exec_global_t *p_tpoam_exec_global)
{
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE, field_id))
    {
        int32 ret;
        p_tpoam_exec_global->test_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID, field_id))
    {
        int32 ret;
        p_tpoam_exec_global->oam_session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_PID, field_id))
    {
        int32 ret;
        p_tpoam_exec_global->pid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK, field_id))
    {
        int32 ret;
        p_tpoam_exec_global->sock = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM, field_id))
    {
        cdb_tpoam_lmm_exec_str2val(str, &p_tpoam_exec_global->exec_lm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP, field_id))
    {
        cdb_tpoam_lb_exec_str2val(str, &p_tpoam_exec_global->exec_lb_rmep);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP, field_id))
    {
        cdb_tpoam_lb_exec_str2val(str, &p_tpoam_exec_global->exec_lb_mip);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY, field_id))
    {
        cdb_tpoam_lb_exec_str2val(str, &p_tpoam_exec_global->exec_lb_discovery);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM, field_id))
    {
        cdb_tpoam_1dm_dmm_exec_str2val(str, &p_tpoam_exec_global->exec_dmm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM, field_id))
    {
        cdb_tpoam_1dm_dmm_exec_str2val(str, &p_tpoam_exec_global->exec_1dm);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER, field_id))
    {
        ctc_task_str2val(str, p_tpoam_exec_global->p_t_send_lb_msg_timer);
    }
    return PM_E_NONE;
}

int32
tbl_tpoam_exec_global_table_parser(char** array, char* key_value,tbl_tpoam_exec_global_t *p_tpoam_exec_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_EXEC_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tpoam_exec_global_key_str2val(key_value, p_tpoam_exec_global));

    for(i=1; i<TBL_TPOAM_EXEC_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tpoam_exec_global_field_value_parser( array[j++], i, p_tpoam_exec_global));
    }

    return PM_E_NONE;
}

int32
tbl_tpoam_exec_global_dump_one(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TPOAM_EXEC_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE].name,
            p_tpoam_exec_global->test_type);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID].name,
            p_tpoam_exec_global->oam_session_id);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_PID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_PID].name,
            p_tpoam_exec_global->pid);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK].name,
            p_tpoam_exec_global->sock);
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM].name, 
            cdb_tpoam_lmm_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lm));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP].name, 
            cdb_tpoam_lb_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_rmep));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP].name, 
            cdb_tpoam_lb_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_mip));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY].name, 
            cdb_tpoam_lb_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_lb_discovery));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM].name, 
            cdb_tpoam_1dm_dmm_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_dmm));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM].name, 
            cdb_tpoam_1dm_dmm_exec_val2str(buf, MAX_CMD_STR_LEN, &p_tpoam_exec_global->exec_1dm));
    }
    if (FLD_MATCH(TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_tpoam_exec_global->p_t_send_lb_msg_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tpoam_exec_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tpoam_exec_global_t *p_db_tpoam_exec_global = _g_p_tbl_tpoam_exec_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_tpoam_exec_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_tpoam_exec_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tpoam_exec_global_t*
tbl_tpoam_exec_global_init_tpoam_exec_global()
{
    _g_p_tbl_tpoam_exec_global = XCALLOC(MEM_TBL_TPOAM_EXEC_GLOBAL, sizeof(tbl_tpoam_exec_global_t));
    return _g_p_tbl_tpoam_exec_global;
}


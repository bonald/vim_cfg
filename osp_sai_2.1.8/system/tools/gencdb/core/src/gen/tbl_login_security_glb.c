
#include "proto.h"
#include "gen/tbl_login_security_glb_define.h"
#include "gen/tbl_login_security_glb.h"
#include "cdb_data_cmp.h"

tbl_login_security_glb_t *_g_p_tbl_login_security_glb = NULL;

int32
tbl_login_security_glb_set_login_security_glb_field_sync(tbl_login_security_glb_t *p_login_security_glb, tbl_login_security_glb_field_id_t field_id, uint32 sync)
{
    tbl_login_security_glb_t *p_db_login_security_glb = _g_p_tbl_login_security_glb;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LOGIN_SECURITY_GLB_FLD_ENABLE:
        p_db_login_security_glb->enable = p_login_security_glb->enable;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM:
        p_db_login_security_glb->max_fail_num = p_login_security_glb->max_fail_num;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD:
        p_db_login_security_glb->fail_period = p_login_security_glb->fail_period;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION:
        p_db_login_security_glb->lock_duration = p_login_security_glb->lock_duration;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER:
        p_db_login_security_glb->curr_invalid_user = p_login_security_glb->curr_invalid_user;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER:
        p_db_login_security_glb->max_invalid_user = p_login_security_glb->max_invalid_user;
        break;

    case TBL_LOGIN_SECURITY_GLB_FLD_MAX:
        sal_memcpy(p_db_login_security_glb, p_login_security_glb, sizeof(tbl_login_security_glb_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LOGIN_SECURITY_GLB, field_id, p_db_login_security_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LOGIN_SECURITY_GLB, field_id, p_db_login_security_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_login_security_glb_set_login_security_glb_field(tbl_login_security_glb_t *p_login_security_glb, tbl_login_security_glb_field_id_t field_id)
{
    return tbl_login_security_glb_set_login_security_glb_field_sync(p_login_security_glb, field_id, TRUE);
}

tbl_login_security_glb_t*
tbl_login_security_glb_get_login_security_glb()
{
    return _g_p_tbl_login_security_glb;
}

int32
tbl_login_security_glb_key_str2val(char *str, tbl_login_security_glb_t *p_login_security_glb)
{
    return PM_E_NONE;
}

char*
tbl_login_security_glb_key_name_dump(tbl_login_security_glb_t *p_login_security_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_SECURITY_GLB);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_login_security_glb_key_value_dump(tbl_login_security_glb_t *p_login_security_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_login_security_glb_field_name_dump(tbl_login_security_glb_t *p_login_security_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_SECURITY_GLB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LOGIN_SECURITY_GLB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_login_security_glb_field_value_dump(tbl_login_security_glb_t *p_login_security_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->enable);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->max_fail_num);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->fail_period);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->lock_duration);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->curr_invalid_user);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER, field_id))
    {
        sal_sprintf(str, "%u", p_login_security_glb->max_invalid_user);
    }
    return str;
}

char**
tbl_login_security_glb_table_dump(tbl_login_security_glb_t *p_login_security_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_login_security_glb_key_name_dump(p_login_security_glb, buf));
    for(i=1; i<TBL_LOGIN_SECURITY_GLB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_login_security_glb_field_name_dump(p_login_security_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_login_security_glb_field_value_dump(p_login_security_glb, i, buf));
    }
    return str;
}

int32
tbl_login_security_glb_field_value_parser(char *str, int32 field_id, tbl_login_security_glb_t *p_login_security_glb)
{
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_login_security_glb->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM, field_id))
    {
        int32 ret;
        p_login_security_glb->max_fail_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD, field_id))
    {
        int32 ret;
        p_login_security_glb->fail_period = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION, field_id))
    {
        int32 ret;
        p_login_security_glb->lock_duration = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER, field_id))
    {
        int32 ret;
        p_login_security_glb->curr_invalid_user = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER, field_id))
    {
        int32 ret;
        p_login_security_glb->max_invalid_user = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_login_security_glb_table_parser(char** array, char* key_value,tbl_login_security_glb_t *p_login_security_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_SECURITY_GLB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_login_security_glb_key_str2val(key_value, p_login_security_glb));

    for(i=1; i<TBL_LOGIN_SECURITY_GLB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_login_security_glb_field_value_parser( array[j++], i, p_login_security_glb));
    }

    return PM_E_NONE;
}

int32
tbl_login_security_glb_dump_one(tbl_login_security_glb_t *p_login_security_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_SECURITY_GLB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_ENABLE].name,
            p_login_security_glb->enable);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_MAX_FAIL_NUM].name,
            p_login_security_glb->max_fail_num);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_FAIL_PERIOD].name,
            p_login_security_glb->fail_period);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_LOCK_DURATION].name,
            p_login_security_glb->lock_duration);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_CURR_INVALID_USER].name,
            p_login_security_glb->curr_invalid_user);
    }
    if (FLD_MATCH(TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_SECURITY_GLB_FLD_MAX_INVALID_USER].name,
            p_login_security_glb->max_invalid_user);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_login_security_glb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_login_security_glb_t *p_db_login_security_glb = _g_p_tbl_login_security_glb;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_login_security_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_login_security_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_login_security_glb_t*
tbl_login_security_glb_init_login_security_glb()
{
    _g_p_tbl_login_security_glb = XCALLOC(MEM_TBL_LOGIN_SECURITY_GLB, sizeof(tbl_login_security_glb_t));
    return _g_p_tbl_login_security_glb;
}


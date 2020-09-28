
#include "proto.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "cdb_data_cmp.h"

tbl_l2_action_t *_g_p_tbl_l2_action = NULL;

int32
tbl_l2_action_set_l2_action_field_sync(tbl_l2_action_t *p_l2_act, tbl_l2_action_field_id_t field_id, uint32 sync)
{
    tbl_l2_action_t *p_db_l2_act = _g_p_tbl_l2_action;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_L2_ACTION_FLD_MAX:
        sal_memcpy(p_db_l2_act, p_l2_act, sizeof(tbl_l2_action_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_L2_ACTION, field_id, p_db_l2_act);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_L2_ACTION, field_id, p_db_l2_act);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_l2_action_set_l2_action_field(tbl_l2_action_t *p_l2_act, tbl_l2_action_field_id_t field_id)
{
    return tbl_l2_action_set_l2_action_field_sync(p_l2_act, field_id, TRUE);
}

tbl_l2_action_t*
tbl_l2_action_get_l2_action()
{
    return _g_p_tbl_l2_action;
}

int32
tbl_l2_action_key_str2val(char *str, tbl_l2_action_t *p_l2_act)
{
    return PM_E_NONE;
}

char*
tbl_l2_action_key_name_dump(tbl_l2_action_t *p_l2_act, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_ACTION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_l2_action_key_value_dump(tbl_l2_action_t *p_l2_act, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_l2_action_field_name_dump(tbl_l2_action_t *p_l2_act, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_L2_ACTION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_l2_action_field_value_dump(tbl_l2_action_t *p_l2_act, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_L2_ACTION_FLD_FLUSH_FDB, field_id))
    {
    }
    return str;
}

char**
tbl_l2_action_table_dump(tbl_l2_action_t *p_l2_act, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_l2_action_key_name_dump(p_l2_act, buf));
    for(i=1; i<TBL_L2_ACTION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_l2_action_field_name_dump(p_l2_act, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_l2_action_field_value_dump(p_l2_act, i, buf));
    }
    return str;
}

int32
tbl_l2_action_field_value_parser(char *str, int32 field_id, tbl_l2_action_t *p_l2_act)
{
    if (FLD_MATCH(TBL_L2_ACTION_FLD_FLUSH_FDB, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_l2_action_table_parser(char** array, char* key_value,tbl_l2_action_t *p_l2_act)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_l2_action_key_str2val(key_value, p_l2_act));

    for(i=1; i<TBL_L2_ACTION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_l2_action_field_value_parser( array[j++], i, p_l2_act));
    }

    return PM_E_NONE;
}

int32
tbl_l2_action_dump_one(tbl_l2_action_t *p_l2_act, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_ACTION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_L2_ACTION_FLD_FLUSH_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_L2_ACTION_FLD_FLUSH_FDB].name);
        if (p_l2_act->flush_fdb)
        {
            ds_flush_fdb_dump_one(p_l2_act->flush_fdb, pargs);
        }
        sal_fprintf(fp, "}");
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_l2_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_l2_action_t *p_db_l2_act = _g_p_tbl_l2_action;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_l2_act)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_l2_act, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_l2_action_t*
tbl_l2_action_init_l2_action()
{
    _g_p_tbl_l2_action = XCALLOC(MEM_TBL_L2_ACTION, sizeof(tbl_l2_action_t));
    return _g_p_tbl_l2_action;
}


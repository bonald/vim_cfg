
#include "proto.h"
#include "gen/tbl_chsm_debug_define.h"
#include "gen/tbl_chsm_debug.h"
#include "cdb_data_cmp.h"

tbl_chsm_debug_t *_g_p_tbl_chsm_debug = NULL;

int32
tbl_chsm_debug_set_chsm_debug_field_sync(tbl_chsm_debug_t *p_chsmdbg, tbl_chsm_debug_field_id_t field_id, uint32 sync)
{
    tbl_chsm_debug_t *p_db_chsmdbg = _g_p_tbl_chsm_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS:
        CDB_FLAG_CHECK_SET(p_db_chsmdbg, p_chsmdbg, chsm, CHSMDBG_FLAG_CHSM_CHASSIS)
        break;

    case TBL_CHSM_DEBUG_FLD_MAX:
        sal_memcpy(p_db_chsmdbg, p_chsmdbg, sizeof(tbl_chsm_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CHSM_DEBUG, field_id, p_db_chsmdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CHSM_DEBUG, field_id, p_db_chsmdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_chsm_debug_set_chsm_debug_field(tbl_chsm_debug_t *p_chsmdbg, tbl_chsm_debug_field_id_t field_id)
{
    return tbl_chsm_debug_set_chsm_debug_field_sync(p_chsmdbg, field_id, TRUE);
}

tbl_chsm_debug_t*
tbl_chsm_debug_get_chsm_debug()
{
    return _g_p_tbl_chsm_debug;
}

int32
tbl_chsm_debug_key_str2val(char *str, tbl_chsm_debug_t *p_chsmdbg)
{
    return PM_E_NONE;
}

char*
tbl_chsm_debug_key_name_dump(tbl_chsm_debug_t *p_chsmdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHSM_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_chsm_debug_key_value_dump(tbl_chsm_debug_t *p_chsmdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_chsm_debug_field_name_dump(tbl_chsm_debug_t *p_chsmdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CHSM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CHSM_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_chsm_debug_field_value_dump(tbl_chsm_debug_t *p_chsmdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_chsmdbg->chsm, CHSMDBG_FLAG_CHSM_CHASSIS));
    }
    return str;
}

char**
tbl_chsm_debug_table_dump(tbl_chsm_debug_t *p_chsmdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_chsm_debug_key_name_dump(p_chsmdbg, buf));
    for(i=1; i<TBL_CHSM_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_chsm_debug_field_name_dump(p_chsmdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_chsm_debug_field_value_dump(p_chsmdbg, i, buf));
    }
    return str;
}

int32
tbl_chsm_debug_field_value_parser(char *str, int32 field_id, tbl_chsm_debug_t *p_chsmdbg)
{
    if (FLD_MATCH(TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS, field_id))
    {
        GLB_SET_FLAG(p_chsmdbg->chsm, CHSMDBG_FLAG_CHSM_CHASSIS);
    }
    return PM_E_NONE;
}

int32
tbl_chsm_debug_table_parser(char** array, char* key_value,tbl_chsm_debug_t *p_chsmdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHSM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_chsm_debug_key_str2val(key_value, p_chsmdbg));

    for(i=1; i<TBL_CHSM_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_chsm_debug_field_value_parser( array[j++], i, p_chsmdbg));
    }

    return PM_E_NONE;
}

int32
tbl_chsm_debug_dump_one(tbl_chsm_debug_t *p_chsmdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CHSM_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CHSM_DEBUG_FLD_CHSM_CHASSIS].name,
            GLB_FLAG_ISSET(p_chsmdbg->chsm, CHSMDBG_FLAG_CHSM_CHASSIS));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_chsm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_chsm_debug_t *p_db_chsmdbg = _g_p_tbl_chsm_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_chsmdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_chsmdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_chsm_debug_t*
tbl_chsm_debug_init_chsm_debug()
{
    _g_p_tbl_chsm_debug = XCALLOC(MEM_TBL_CHSM_DEBUG, sizeof(tbl_chsm_debug_t));
    return _g_p_tbl_chsm_debug;
}


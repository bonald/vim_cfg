
#include "proto.h"
#include "gen/tbl_openflow_debug_define.h"
#include "gen/tbl_openflow_debug.h"
#include "cdb_data_cmp.h"

tbl_openflow_debug_t *_g_p_tbl_openflow_debug = NULL;

int32
tbl_openflow_debug_set_openflow_debug_field_sync(tbl_openflow_debug_t *p_openflowdbg, tbl_openflow_debug_field_id_t field_id, uint32 sync)
{
    tbl_openflow_debug_t *p_db_openflowdbg = _g_p_tbl_openflow_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN:
        CDB_FLAG_CHECK_SET(p_db_openflowdbg, p_openflowdbg, openflow, OPENFLOWDBG_FLAG_VCONN)
        break;

    case TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN:
        CDB_FLAG_CHECK_SET(p_db_openflowdbg, p_openflowdbg, openflow, OPENFLOWDBG_FLAG_RCONN)
        break;

    case TBL_OPENFLOW_DEBUG_FLD_MAX:
        sal_memcpy(p_db_openflowdbg, p_openflowdbg, sizeof(tbl_openflow_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW_DEBUG, field_id, p_db_openflowdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW_DEBUG, field_id, p_db_openflowdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_debug_set_openflow_debug_field(tbl_openflow_debug_t *p_openflowdbg, tbl_openflow_debug_field_id_t field_id)
{
    return tbl_openflow_debug_set_openflow_debug_field_sync(p_openflowdbg, field_id, TRUE);
}

tbl_openflow_debug_t*
tbl_openflow_debug_get_openflow_debug()
{
    return _g_p_tbl_openflow_debug;
}

int32
tbl_openflow_debug_key_str2val(char *str, tbl_openflow_debug_t *p_openflowdbg)
{
    return PM_E_NONE;
}

char*
tbl_openflow_debug_key_name_dump(tbl_openflow_debug_t *p_openflowdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_openflow_debug_key_value_dump(tbl_openflow_debug_t *p_openflowdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_openflow_debug_field_name_dump(tbl_openflow_debug_t *p_openflowdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_debug_field_value_dump(tbl_openflow_debug_t *p_openflowdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_VCONN));
    }
    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_RCONN));
    }
    return str;
}

char**
tbl_openflow_debug_table_dump(tbl_openflow_debug_t *p_openflowdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_debug_key_name_dump(p_openflowdbg, buf));
    for(i=1; i<TBL_OPENFLOW_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_debug_field_name_dump(p_openflowdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_debug_field_value_dump(p_openflowdbg, i, buf));
    }
    return str;
}

int32
tbl_openflow_debug_field_value_parser(char *str, int32 field_id, tbl_openflow_debug_t *p_openflowdbg)
{
    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN, field_id))
    {
        GLB_SET_FLAG(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_VCONN);
    }
    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN, field_id))
    {
        GLB_SET_FLAG(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_RCONN);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_debug_table_parser(char** array, char* key_value,tbl_openflow_debug_t *p_openflowdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_debug_key_str2val(key_value, p_openflowdbg));

    for(i=1; i<TBL_OPENFLOW_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_debug_field_value_parser( array[j++], i, p_openflowdbg));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_debug_dump_one(tbl_openflow_debug_t *p_openflowdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN].name,
            GLB_FLAG_ISSET(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_VCONN));
    }
    if (FLD_MATCH(TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN].name,
            GLB_FLAG_ISSET(p_openflowdbg->openflow, OPENFLOWDBG_FLAG_RCONN));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_debug_t *p_db_openflowdbg = _g_p_tbl_openflow_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_openflowdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_openflowdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_openflow_debug_t*
tbl_openflow_debug_init_openflow_debug()
{
    _g_p_tbl_openflow_debug = XCALLOC(MEM_TBL_OPENFLOW_DEBUG, sizeof(tbl_openflow_debug_t));
    return _g_p_tbl_openflow_debug;
}


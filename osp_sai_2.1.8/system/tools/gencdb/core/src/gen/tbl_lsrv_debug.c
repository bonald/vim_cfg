
#include "proto.h"
#include "gen/tbl_lsrv_debug_define.h"
#include "gen/tbl_lsrv_debug.h"
#include "cdb_data_cmp.h"

tbl_lsrv_debug_t *_g_p_tbl_lsrv_debug = NULL;

int32
tbl_lsrv_debug_set_lsrv_debug_field_sync(tbl_lsrv_debug_t *p_lsrvdbg, tbl_lsrv_debug_field_id_t field_id, uint32 sync)
{
    tbl_lsrv_debug_t *p_db_lsrvdbg = _g_p_tbl_lsrv_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LSRV_DEBUG_FLD_CARD:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_CARD)
        break;

    case TBL_LSRV_DEBUG_FLD_PORT:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_PORT)
        break;

    case TBL_LSRV_DEBUG_FLD_FIBER:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_FIBER)
        break;

    case TBL_LSRV_DEBUG_FLD_FAN:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_FAN)
        break;

    case TBL_LSRV_DEBUG_FLD_PSU:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_PSU)
        break;

    case TBL_LSRV_DEBUG_FLD_LED:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_LED)
        break;

    case TBL_LSRV_DEBUG_FLD_THERMAL:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_SENSOR)
        break;

    case TBL_LSRV_DEBUG_FLD_VCM:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_VCM)
        break;

    case TBL_LSRV_DEBUG_FLD_POE:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, card, LSRVDBG_FLAG_CARD_POE)
        break;

    case TBL_LSRV_DEBUG_FLD_SYNCE:
        CDB_FLAG_CHECK_SET(p_db_lsrvdbg, p_lsrvdbg, synce, LSRVDBG_FLAG_SYNCE_SYNCE)
        break;

    case TBL_LSRV_DEBUG_FLD_MAX:
        sal_memcpy(p_db_lsrvdbg, p_lsrvdbg, sizeof(tbl_lsrv_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LSRV_DEBUG, field_id, p_db_lsrvdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LSRV_DEBUG, field_id, p_db_lsrvdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lsrv_debug_set_lsrv_debug_field(tbl_lsrv_debug_t *p_lsrvdbg, tbl_lsrv_debug_field_id_t field_id)
{
    return tbl_lsrv_debug_set_lsrv_debug_field_sync(p_lsrvdbg, field_id, TRUE);
}

tbl_lsrv_debug_t*
tbl_lsrv_debug_get_lsrv_debug()
{
    return _g_p_tbl_lsrv_debug;
}

int32
tbl_lsrv_debug_key_str2val(char *str, tbl_lsrv_debug_t *p_lsrvdbg)
{
    return PM_E_NONE;
}

char*
tbl_lsrv_debug_key_name_dump(tbl_lsrv_debug_t *p_lsrvdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSRV_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_lsrv_debug_key_value_dump(tbl_lsrv_debug_t *p_lsrvdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_lsrv_debug_field_name_dump(tbl_lsrv_debug_t *p_lsrvdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LSRV_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LSRV_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lsrv_debug_field_value_dump(tbl_lsrv_debug_t *p_lsrvdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_CARD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_CARD));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PORT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PORT));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FIBER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FIBER));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FAN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FAN));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PSU, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PSU));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_LED, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_LED));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_THERMAL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_SENSOR));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_VCM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_VCM));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_POE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_POE));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_SYNCE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_lsrvdbg->synce, LSRVDBG_FLAG_SYNCE_SYNCE));
    }
    return str;
}

char**
tbl_lsrv_debug_table_dump(tbl_lsrv_debug_t *p_lsrvdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lsrv_debug_key_name_dump(p_lsrvdbg, buf));
    for(i=1; i<TBL_LSRV_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lsrv_debug_field_name_dump(p_lsrvdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lsrv_debug_field_value_dump(p_lsrvdbg, i, buf));
    }
    return str;
}

int32
tbl_lsrv_debug_field_value_parser(char *str, int32 field_id, tbl_lsrv_debug_t *p_lsrvdbg)
{
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_CARD, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_CARD);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PORT, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PORT);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FIBER, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FIBER);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FAN, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FAN);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PSU, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PSU);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_LED, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_LED);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_THERMAL, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_SENSOR);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_VCM, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_VCM);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_POE, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_POE);
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_SYNCE, field_id))
    {
        GLB_SET_FLAG(p_lsrvdbg->synce, LSRVDBG_FLAG_SYNCE_SYNCE);
    }
    return PM_E_NONE;
}

int32
tbl_lsrv_debug_table_parser(char** array, char* key_value,tbl_lsrv_debug_t *p_lsrvdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSRV_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lsrv_debug_key_str2val(key_value, p_lsrvdbg));

    for(i=1; i<TBL_LSRV_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lsrv_debug_field_value_parser( array[j++], i, p_lsrvdbg));
    }

    return PM_E_NONE;
}

int32
tbl_lsrv_debug_dump_one(tbl_lsrv_debug_t *p_lsrvdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSRV_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_CARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_CARD].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_CARD));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_PORT].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PORT));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FIBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_FIBER].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FIBER));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_FAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_FAN].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_FAN));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_PSU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_PSU].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_PSU));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_LED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_LED].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_LED));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_THERMAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_THERMAL].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_SENSOR));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_VCM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_VCM].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_VCM));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_POE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_POE].name,
            GLB_FLAG_ISSET(p_lsrvdbg->card, LSRVDBG_FLAG_CARD_POE));
    }
    if (FLD_MATCH(TBL_LSRV_DEBUG_FLD_SYNCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSRV_DEBUG_FLD_SYNCE].name,
            GLB_FLAG_ISSET(p_lsrvdbg->synce, LSRVDBG_FLAG_SYNCE_SYNCE));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lsrv_debug_t *p_db_lsrvdbg = _g_p_tbl_lsrv_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_lsrvdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_lsrvdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lsrv_debug_t*
tbl_lsrv_debug_init_lsrv_debug()
{
    _g_p_tbl_lsrv_debug = XCALLOC(MEM_TBL_LSRV_DEBUG, sizeof(tbl_lsrv_debug_t));
    return _g_p_tbl_lsrv_debug;
}


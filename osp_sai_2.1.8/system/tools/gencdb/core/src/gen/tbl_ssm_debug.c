
#include "proto.h"
#include "gen/tbl_ssm_debug_define.h"
#include "gen/tbl_ssm_debug.h"
#include "cdb_data_cmp.h"

tbl_ssm_debug_t *_g_p_tbl_ssm_debug = NULL;

int32
tbl_ssm_debug_set_ssm_debug_field_sync(tbl_ssm_debug_t *p_ssmdbg, tbl_ssm_debug_field_id_t field_id, uint32 sync)
{
    tbl_ssm_debug_t *p_db_ssmdbg = _g_p_tbl_ssm_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SSM_DEBUG_FLD_SSM_EVENT:
        CDB_FLAG_CHECK_SET(p_db_ssmdbg, p_ssmdbg, ssm, SSM_DEBUG_FLAG_EVENT)
        break;

    case TBL_SSM_DEBUG_FLD_SSM_PACKET_TX:
        CDB_FLAG_CHECK_SET(p_db_ssmdbg, p_ssmdbg, ssm, SSM_DEBUG_FLAG_PACKET_TX)
        break;

    case TBL_SSM_DEBUG_FLD_SSM_PACKET_RX:
        CDB_FLAG_CHECK_SET(p_db_ssmdbg, p_ssmdbg, ssm, SSM_DEBUG_FLAG_PACKET_RX)
        break;

    case TBL_SSM_DEBUG_FLD_SSM_TIMER:
        CDB_FLAG_CHECK_SET(p_db_ssmdbg, p_ssmdbg, ssm, SSM_DEBUG_FLAG_TIMER)
        break;

    case TBL_SSM_DEBUG_FLD_MAX:
        sal_memcpy(p_db_ssmdbg, p_ssmdbg, sizeof(tbl_ssm_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SSM_DEBUG, field_id, p_db_ssmdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SSM_DEBUG, field_id, p_db_ssmdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ssm_debug_set_ssm_debug_field(tbl_ssm_debug_t *p_ssmdbg, tbl_ssm_debug_field_id_t field_id)
{
    return tbl_ssm_debug_set_ssm_debug_field_sync(p_ssmdbg, field_id, TRUE);
}

tbl_ssm_debug_t*
tbl_ssm_debug_get_ssm_debug()
{
    return _g_p_tbl_ssm_debug;
}

int32
tbl_ssm_debug_key_str2val(char *str, tbl_ssm_debug_t *p_ssmdbg)
{
    return PM_E_NONE;
}

char*
tbl_ssm_debug_key_name_dump(tbl_ssm_debug_t *p_ssmdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ssm_debug_key_value_dump(tbl_ssm_debug_t *p_ssmdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ssm_debug_field_name_dump(tbl_ssm_debug_t *p_ssmdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SSM_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ssm_debug_field_value_dump(tbl_ssm_debug_t *p_ssmdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_TIMER));
    }
    return str;
}

char**
tbl_ssm_debug_table_dump(tbl_ssm_debug_t *p_ssmdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ssm_debug_key_name_dump(p_ssmdbg, buf));
    for(i=1; i<TBL_SSM_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ssm_debug_field_name_dump(p_ssmdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ssm_debug_field_value_dump(p_ssmdbg, i, buf));
    }
    return str;
}

int32
tbl_ssm_debug_field_value_parser(char *str, int32 field_id, tbl_ssm_debug_t *p_ssmdbg)
{
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_EVENT, field_id))
    {
        GLB_SET_FLAG(p_ssmdbg->ssm, SSM_DEBUG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_TX, field_id))
    {
        GLB_SET_FLAG(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_TX);
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_RX, field_id))
    {
        GLB_SET_FLAG(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_RX);
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_TIMER, field_id))
    {
        GLB_SET_FLAG(p_ssmdbg->ssm, SSM_DEBUG_FLAG_TIMER);
    }
    return PM_E_NONE;
}

int32
tbl_ssm_debug_table_parser(char** array, char* key_value,tbl_ssm_debug_t *p_ssmdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ssm_debug_key_str2val(key_value, p_ssmdbg));

    for(i=1; i<TBL_SSM_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ssm_debug_field_value_parser( array[j++], i, p_ssmdbg));
    }

    return PM_E_NONE;
}

int32
tbl_ssm_debug_dump_one(tbl_ssm_debug_t *p_ssmdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_DEBUG_FLD_SSM_EVENT].name,
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_DEBUG_FLD_SSM_PACKET_TX].name,
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_PACKET_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_DEBUG_FLD_SSM_PACKET_RX].name,
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SSM_DEBUG_FLD_SSM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_DEBUG_FLD_SSM_TIMER].name,
            GLB_FLAG_ISSET(p_ssmdbg->ssm, SSM_DEBUG_FLAG_TIMER));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ssm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ssm_debug_t *p_db_ssmdbg = _g_p_tbl_ssm_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ssmdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ssmdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ssm_debug_t*
tbl_ssm_debug_init_ssm_debug()
{
    _g_p_tbl_ssm_debug = XCALLOC(MEM_TBL_SSM_DEBUG, sizeof(tbl_ssm_debug_t));
    return _g_p_tbl_ssm_debug;
}


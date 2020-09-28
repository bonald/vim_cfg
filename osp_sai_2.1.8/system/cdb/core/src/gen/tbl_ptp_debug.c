
#include "proto.h"
#include "gen/tbl_ptp_debug_define.h"
#include "gen/tbl_ptp_debug.h"
#include "cdb_data_cmp.h"

tbl_ptp_debug_t *_g_p_tbl_ptp_debug = NULL;

int32
tbl_ptp_debug_set_ptp_debug_field_sync(tbl_ptp_debug_t *p_ptpdbg, tbl_ptp_debug_field_id_t field_id, uint32 sync)
{
    tbl_ptp_debug_t *p_db_ptpdbg = _g_p_tbl_ptp_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_COMMUNICATION_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_BMC_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_TIMER_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_PACKET_RX_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_PACKET_TX_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_ptpdbg, p_ptpdbg, ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG)
        break;

    case TBL_PTP_DEBUG_FLD_MAX:
        sal_memcpy(p_db_ptpdbg, p_ptpdbg, sizeof(tbl_ptp_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PTP_DEBUG, field_id, p_db_ptpdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PTP_DEBUG, field_id, p_db_ptpdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_debug_set_ptp_debug_field(tbl_ptp_debug_t *p_ptpdbg, tbl_ptp_debug_field_id_t field_id)
{
    return tbl_ptp_debug_set_ptp_debug_field_sync(p_ptpdbg, field_id, TRUE);
}

tbl_ptp_debug_t*
tbl_ptp_debug_get_ptp_debug()
{
    return _g_p_tbl_ptp_debug;
}

int32
tbl_ptp_debug_key_str2val(char *str, tbl_ptp_debug_t *p_ptpdbg)
{
    return PM_E_NONE;
}

char*
tbl_ptp_debug_key_name_dump(tbl_ptp_debug_t *p_ptpdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ptp_debug_key_value_dump(tbl_ptp_debug_t *p_ptpdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ptp_debug_field_name_dump(tbl_ptp_debug_t *p_ptpdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PTP_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ptp_debug_field_value_dump(tbl_ptp_debug_t *p_ptpdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_COMMUNICATION_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_BMC_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_TIMER_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_RX_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_TX_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG));
    }
    return str;
}

char**
tbl_ptp_debug_table_dump(tbl_ptp_debug_t *p_ptpdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ptp_debug_key_name_dump(p_ptpdbg, buf));
    for(i=1; i<TBL_PTP_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ptp_debug_field_name_dump(p_ptpdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ptp_debug_field_value_dump(p_ptpdbg, i, buf));
    }
    return str;
}

int32
tbl_ptp_debug_field_value_parser(char *str, int32 field_id, tbl_ptp_debug_t *p_ptpdbg)
{
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_COMMUNICATION_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_BMC_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_TIMER_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_RX_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_TX_DBG);
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG);
    }
    return PM_E_NONE;
}

int32
tbl_ptp_debug_table_parser(char** array, char* key_value,tbl_ptp_debug_t *p_ptpdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ptp_debug_key_str2val(key_value, p_ptpdbg));

    for(i=1; i<TBL_PTP_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ptp_debug_field_value_parser( array[j++], i, p_ptpdbg));
    }

    return PM_E_NONE;
}

int32
tbl_ptp_debug_dump_one(tbl_ptp_debug_t *p_ptpdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_COMMUNICATION_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_BMC_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_TIMER_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_SYNCHRONIZATION_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_RX_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_TX_DBG));
    }
    if (FLD_MATCH(TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG].name,
            GLB_FLAG_ISSET(p_ptpdbg->ptp, PTPDBG_FLAG_PACKET_DISCARD_DBG));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ptp_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ptp_debug_t *p_db_ptpdbg = _g_p_tbl_ptp_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ptpdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ptpdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ptp_debug_t*
tbl_ptp_debug_init_ptp_debug()
{
    _g_p_tbl_ptp_debug = XCALLOC(MEM_TBL_PTP_DEBUG, sizeof(tbl_ptp_debug_t));
    return _g_p_tbl_ptp_debug;
}


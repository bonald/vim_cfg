
#include "proto.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "cdb_data_cmp.h"

tbl_opm_debug_t *_g_p_tbl_opm_debug = NULL;

int32
tbl_opm_debug_set_opm_debug_field_sync(tbl_opm_debug_t *p_opmdbg, tbl_opm_debug_field_id_t field_id, uint32 sync)
{
    tbl_opm_debug_t *p_db_opmdbg = _g_p_tbl_opm_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPM_DEBUG_FLD_ERPS_ALL:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, erps, OPMDBG_FLAG_ERPS_ALL)
        break;

    case TBL_OPM_DEBUG_FLD_ERPS_PACKET:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, erps, OPMDBG_FLAG_ERPS_PACKET)
        break;

    case TBL_OPM_DEBUG_FLD_ERPS_TIMER:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, erps, OPMDBG_FLAG_ERPS_TIMER)
        break;

    case TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, erps, OPMDBG_FLAG_ERPS_PROTO)
        break;

    case TBL_OPM_DEBUG_FLD_ERPS_EVENT:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, erps, OPMDBG_FLAG_ERPS_EVENTS)
        break;

    case TBL_OPM_DEBUG_FLD_G8131_TX:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8131, OPMDBG_FLAG_G8131_TX)
        break;

    case TBL_OPM_DEBUG_FLD_G8131_RX:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8131, OPMDBG_FLAG_G8131_RX)
        break;

    case TBL_OPM_DEBUG_FLD_G8131_EVENT:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8131, OPMDBG_FLAG_G8131_EVENT)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG)
        break;

    case TBL_OPM_DEBUG_FLD_G8032_ALL:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8032, OPMDBG_FLAG_G8032_ALL)
        break;

    case TBL_OPM_DEBUG_FLD_G8032_PACKET_TX:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8032, OPMDBG_FLAG_G8032_TX)
        break;

    case TBL_OPM_DEBUG_FLD_G8032_PACKET_RX:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8032, OPMDBG_FLAG_G8032_RX)
        break;

    case TBL_OPM_DEBUG_FLD_G8032_EVENT:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8032, OPMDBG_FLAG_G8032_EVENT)
        break;

    case TBL_OPM_DEBUG_FLD_G8032_TIMER:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, g8032, OPMDBG_FLAG_G8032_TIMER)
        break;

    case TBL_OPM_DEBUG_FLD_CFM_ALL:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, cfm, OPMDBG_FLAG_CFM_ALL)
        break;

    case TBL_OPM_DEBUG_FLD_CFM_PACKET:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, cfm, OPMDBG_FLAG_CFM_PACKET)
        break;

    case TBL_OPM_DEBUG_FLD_CFM_TIMER:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, cfm, OPMDBG_FLAG_CFM_TIMER)
        break;

    case TBL_OPM_DEBUG_FLD_CFM_EVENT:
        CDB_FLAG_CHECK_SET(p_db_opmdbg, p_opmdbg, cfm, OPMDBG_FLAG_CFM_EVENT)
        break;

    case TBL_OPM_DEBUG_FLD_MAX:
        sal_memcpy(p_db_opmdbg, p_opmdbg, sizeof(tbl_opm_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPM_DEBUG, field_id, p_db_opmdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPM_DEBUG, field_id, p_db_opmdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_opm_debug_set_opm_debug_field(tbl_opm_debug_t *p_opmdbg, tbl_opm_debug_field_id_t field_id)
{
    return tbl_opm_debug_set_opm_debug_field_sync(p_opmdbg, field_id, TRUE);
}

tbl_opm_debug_t*
tbl_opm_debug_get_opm_debug()
{
    return _g_p_tbl_opm_debug;
}

int32
tbl_opm_debug_key_str2val(char *str, tbl_opm_debug_t *p_opmdbg)
{
    return PM_E_NONE;
}

char*
tbl_opm_debug_key_name_dump(tbl_opm_debug_t *p_opmdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_opm_debug_key_value_dump(tbl_opm_debug_t *p_opmdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_opm_debug_field_name_dump(tbl_opm_debug_t *p_opmdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPM_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_opm_debug_field_value_dump(tbl_opm_debug_t *p_opmdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PACKET));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PROTO));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_EVENTS));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_TX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_RX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_EVENT));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_RX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_EVENT));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_PACKET));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_EVENT));
    }
    return str;
}

char**
tbl_opm_debug_table_dump(tbl_opm_debug_t *p_opmdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_opm_debug_key_name_dump(p_opmdbg, buf));
    for(i=1; i<TBL_OPM_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_opm_debug_field_name_dump(p_opmdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_opm_debug_field_value_dump(p_opmdbg, i, buf));
    }
    return str;
}

int32
tbl_opm_debug_field_value_parser(char *str, int32 field_id, tbl_opm_debug_t *p_opmdbg)
{
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_ALL, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->erps, OPMDBG_FLAG_ERPS_ALL);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PACKET, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PACKET);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_TIMER, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->erps, OPMDBG_FLAG_ERPS_TIMER);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PROTO);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_EVENT, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->erps, OPMDBG_FLAG_ERPS_EVENTS);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_TX, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8131, OPMDBG_FLAG_G8131_TX);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_RX, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8131, OPMDBG_FLAG_G8131_RX);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_EVENT, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8131, OPMDBG_FLAG_G8131_EVENT);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_ALL, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8032, OPMDBG_FLAG_G8032_ALL);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_TX, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TX);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_RX, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8032, OPMDBG_FLAG_G8032_RX);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_EVENT, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8032, OPMDBG_FLAG_G8032_EVENT);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_TIMER, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TIMER);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_ALL, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->cfm, OPMDBG_FLAG_CFM_ALL);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_PACKET, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->cfm, OPMDBG_FLAG_CFM_PACKET);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_TIMER, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->cfm, OPMDBG_FLAG_CFM_TIMER);
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_EVENT, field_id))
    {
        GLB_SET_FLAG(p_opmdbg->cfm, OPMDBG_FLAG_CFM_EVENT);
    }
    return PM_E_NONE;
}

int32
tbl_opm_debug_table_parser(char** array, char* key_value,tbl_opm_debug_t *p_opmdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_opm_debug_key_str2val(key_value, p_opmdbg));

    for(i=1; i<TBL_OPM_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_opm_debug_field_value_parser( array[j++], i, p_opmdbg));
    }

    return PM_E_NONE;
}

int32
tbl_opm_debug_dump_one(tbl_opm_debug_t *p_opmdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_ERPS_ALL].name,
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_ERPS_PACKET].name,
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PACKET));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_ERPS_TIMER].name,
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL].name,
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_PROTO));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_ERPS_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_ERPS_EVENT].name,
            GLB_FLAG_ISSET(p_opmdbg->erps, OPMDBG_FLAG_ERPS_EVENTS));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8131_TX].name,
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_TX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8131_RX].name,
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_RX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8131_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8131_EVENT].name,
            GLB_FLAG_ISSET(p_opmdbg->g8131, OPMDBG_FLAG_G8131_EVENT));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LB_LBR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_COMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_LM_LMR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_COMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMM_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_DMR_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_RX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_PACKET_TX_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG].name,
            GLB_FLAG_ISSET(p_opmdbg->tpoam, OPMDBG_FLAG_TPOAM_EVENT_DBG));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8032_ALL].name,
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8032_PACKET_TX].name,
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_PACKET_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8032_PACKET_RX].name,
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_RX));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8032_EVENT].name,
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_EVENT));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_G8032_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_G8032_TIMER].name,
            GLB_FLAG_ISSET(p_opmdbg->g8032, OPMDBG_FLAG_G8032_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_CFM_ALL].name,
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_ALL));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_CFM_PACKET].name,
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_PACKET));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_CFM_TIMER].name,
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_TIMER));
    }
    if (FLD_MATCH(TBL_OPM_DEBUG_FLD_CFM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_DEBUG_FLD_CFM_EVENT].name,
            GLB_FLAG_ISSET(p_opmdbg->cfm, OPMDBG_FLAG_CFM_EVENT));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_opm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_opm_debug_t *p_db_opmdbg = _g_p_tbl_opm_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_opmdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_opmdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_opm_debug_t*
tbl_opm_debug_init_opm_debug()
{
    _g_p_tbl_opm_debug = XCALLOC(MEM_TBL_OPM_DEBUG, sizeof(tbl_opm_debug_t));
    return _g_p_tbl_opm_debug;
}


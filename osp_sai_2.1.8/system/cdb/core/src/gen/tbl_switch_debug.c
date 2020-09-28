
#include "proto.h"
#include "gen/tbl_switch_debug_define.h"
#include "gen/tbl_switch_debug.h"
#include "cdb_data_cmp.h"

tbl_switch_debug_t *_g_p_tbl_switch_debug = NULL;

int32
tbl_switch_debug_set_switch_debug_field_sync(tbl_switch_debug_t *p_swthdbg, tbl_switch_debug_field_id_t field_id, uint32 sync)
{
    tbl_switch_debug_t *p_db_swthdbg = _g_p_tbl_switch_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SWITCH_DEBUG_FLD_STP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_PACKET_TX)
        break;

    case TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_PACKET_RX)
        break;

    case TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_PROTO_CIST)
        break;

    case TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_PROTO_MSTI)
        break;

    case TBL_SWITCH_DEBUG_FLD_STP_TIMER:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, stp, SWTHDBG_FLAG_STP_TIMER)
        break;

    case TBL_SWITCH_DEBUG_FLD_MLAG_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, mlag, SWTHDBG_FLAG_MLAG_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_MLAG_PACKET:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, mlag, SWTHDBG_FLAG_MLAG_PACKET)
        break;

    case TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, mlag, SWTHDBG_FLAG_MLAG_PROTO)
        break;

    case TBL_SWITCH_DEBUG_FLD_MLAG_TIMER:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, mlag, SWTHDBG_FLAG_MLAG_TIMER)
        break;

    case TBL_SWITCH_DEBUG_FLD_LACP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lacp, SWTHDBG_FLAG_LACP_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_LACP_PACKET:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lacp, SWTHDBG_FLAG_LACP_PACKET)
        break;

    case TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lacp, SWTHDBG_FLAG_LACP_PROTO)
        break;

    case TBL_SWITCH_DEBUG_FLD_LACP_TIMER:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lacp, SWTHDBG_FLAG_LACP_TIMER)
        break;

    case TBL_SWITCH_DEBUG_FLD_IGSP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, igsp, SWTHDBG_FLAG_IGSP_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, igsp, SWTHDBG_FLAG_IGSP_PACKET_TX)
        break;

    case TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, igsp, SWTHDBG_FLAG_IGSP_PACKET_RX)
        break;

    case TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, igsp, SWTHDBG_FLAG_IGSP_PROTO)
        break;

    case TBL_SWITCH_DEBUG_FLD_IGSP_TIMER:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, igsp, SWTHDBG_FLAG_IGSP_TIMER)
        break;

    case TBL_SWITCH_DEBUG_FLD_IPSG_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, ipsg, SWTHDBG_FLAG_IPSG_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_IPSG_ERROR:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, ipsg, SWTHDBG_FLAG_IPSG_ERROR)
        break;

    case TBL_SWITCH_DEBUG_FLD_LLDP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lldp, SWTHDBG_FLAG_LLDP_EVENT)
        break;

    case TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lldp, SWTHDBG_FLAG_LLDP_PACKET_TX)
        break;

    case TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lldp, SWTHDBG_FLAG_LLDP_PACKET_RX)
        break;

    case TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lldp, SWTHDBG_FLAG_LLDP_PROTO)
        break;

    case TBL_SWITCH_DEBUG_FLD_LLDP_TIMER:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, lldp, SWTHDBG_FLAG_LLDP_TIMER)
        break;

    case TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST)
        break;

    case TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND:
        CDB_FLAG_CHECK_SET(p_db_swthdbg, p_swthdbg, rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND)
        break;

    case TBL_SWITCH_DEBUG_FLD_MAX:
        sal_memcpy(p_db_swthdbg, p_swthdbg, sizeof(tbl_switch_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SWITCH_DEBUG, field_id, p_db_swthdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SWITCH_DEBUG, field_id, p_db_swthdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_switch_debug_set_switch_debug_field(tbl_switch_debug_t *p_swthdbg, tbl_switch_debug_field_id_t field_id)
{
    return tbl_switch_debug_set_switch_debug_field_sync(p_swthdbg, field_id, TRUE);
}

tbl_switch_debug_t*
tbl_switch_debug_get_switch_debug()
{
    return _g_p_tbl_switch_debug;
}

int32
tbl_switch_debug_key_str2val(char *str, tbl_switch_debug_t *p_swthdbg)
{
    return PM_E_NONE;
}

char*
tbl_switch_debug_key_name_dump(tbl_switch_debug_t *p_swthdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SWITCH_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_switch_debug_key_value_dump(tbl_switch_debug_t *p_swthdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_switch_debug_field_name_dump(tbl_switch_debug_t *p_swthdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SWITCH_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SWITCH_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_switch_debug_field_value_dump(tbl_switch_debug_t *p_swthdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_CIST));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_MSTI));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PACKET));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PACKET));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_ERROR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_ERROR));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND));
    }
    return str;
}

char**
tbl_switch_debug_table_dump(tbl_switch_debug_t *p_swthdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_switch_debug_key_name_dump(p_swthdbg, buf));
    for(i=1; i<TBL_SWITCH_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_switch_debug_field_name_dump(p_swthdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_switch_debug_field_value_dump(p_swthdbg, i, buf));
    }
    return str;
}

int32
tbl_switch_debug_field_value_parser(char *str, int32 field_id, tbl_switch_debug_t *p_swthdbg)
{
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_TX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_RX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_CIST);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_MSTI);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_TIMER, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->stp, SWTHDBG_FLAG_STP_TIMER);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PACKET, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PACKET);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PROTO);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_TIMER, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_TIMER);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PACKET, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PACKET);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PROTO);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_TIMER, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_TIMER);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_TX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_RX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PROTO);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_TIMER, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_TIMER);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_ERROR, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_ERROR);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_EVENT);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_TX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_RX);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PROTO);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_TIMER, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_TIMER);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST);
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND, field_id))
    {
        GLB_SET_FLAG(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND);
    }
    return PM_E_NONE;
}

int32
tbl_switch_debug_table_parser(char** array, char* key_value,tbl_switch_debug_t *p_swthdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SWITCH_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_switch_debug_key_str2val(key_value, p_swthdbg));

    for(i=1; i<TBL_SWITCH_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_switch_debug_field_value_parser( array[j++], i, p_swthdbg));
    }

    return PM_E_NONE;
}

int32
tbl_switch_debug_dump_one(tbl_switch_debug_t *p_swthdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SWITCH_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_CIST));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_PROTO_MSTI));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_STP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_STP_TIMER].name,
            GLB_FLAG_ISSET(p_swthdbg->stp, SWTHDBG_FLAG_STP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_MLAG_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_MLAG_PACKET].name,
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PACKET));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL].name,
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_MLAG_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_MLAG_TIMER].name,
            GLB_FLAG_ISSET(p_swthdbg->mlag, SWTHDBG_FLAG_MLAG_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LACP_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LACP_PACKET].name,
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PACKET));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL].name,
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LACP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LACP_TIMER].name,
            GLB_FLAG_ISSET(p_swthdbg->lacp, SWTHDBG_FLAG_LACP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IGSP_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX].name,
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX].name,
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL].name,
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IGSP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IGSP_TIMER].name,
            GLB_FLAG_ISSET(p_swthdbg->igsp, SWTHDBG_FLAG_IGSP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IPSG_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_IPSG_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_IPSG_ERROR].name,
            GLB_FLAG_ISSET(p_swthdbg->ipsg, SWTHDBG_FLAG_IPSG_ERROR));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LLDP_EVENT].name,
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_EVENT));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX].name,
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX].name,
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL].name,
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_PROTO));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_LLDP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_LLDP_TIMER].name,
            GLB_FLAG_ISSET(p_swthdbg->lldp, SWTHDBG_FLAG_LLDP_TIMER));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST].name,
            GLB_FLAG_ISSET(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST));
    }
    if (FLD_MATCH(TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND].name,
            GLB_FLAG_ISSET(p_swthdbg->rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_switch_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_switch_debug_t *p_db_swthdbg = _g_p_tbl_switch_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_swthdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_swthdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_switch_debug_t*
tbl_switch_debug_init_switch_debug()
{
    _g_p_tbl_switch_debug = XCALLOC(MEM_TBL_SWITCH_DEBUG, sizeof(tbl_switch_debug_t));
    return _g_p_tbl_switch_debug;
}


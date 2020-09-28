
#include "proto.h"
#include "gen/tbl_quagga_debug_define.h"
#include "gen/tbl_quagga_debug.h"
#include "cdb_data_cmp.h"

tbl_quagga_debug_t *_g_p_tbl_quagga_debug = NULL;

int32
tbl_quagga_debug_set_quagga_debug_field_sync(tbl_quagga_debug_t *p_dbg, tbl_quagga_debug_field_id_t field_id, uint32 sync)
{
    tbl_quagga_debug_t *p_db_dbg = _g_p_tbl_quagga_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QUAGGA_DEBUG_FLD_RIP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, rip, RIPDBG_FLAG_EVENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_RIP_PACKET:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, rip, RIPDBG_FLAG_PACKET)
        break;

    case TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, rip, RIPDBG_FLAG_ZEBRA)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_EVENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_ISM_EVENTS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_ISM_STATUS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_ISM_TIMERS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_LSA_FLOODING)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_LSA_GENERATE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_LSA_INSTALL)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_LSA_REFRESH)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_NSM_EVENTS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_NSM_STATUS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_NSM_TIMERS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_NSSA)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_PACKET_DD)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_PACKET_HELLO)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_PACKET_LS_ACK)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_EVENT_ABR)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_EVENT_IA)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_EVENT_ROUTE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, ospf, OSPFDBG_FLAG_EVENT_SPF)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_AS4_SEGMENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_EVENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_FILTERS)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_FSM:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_FSM)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_KEEPALIVES)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_UPDATES_IN)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_UPDATES_OUT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, bgp, BGPDBG_FLAG_ZEBRA)
        break;

    case TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, igmp, IGMPDBG_FLAG_EVENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, igmp, IGMPDBG_FLAG_PACKET)
        break;

    case TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, igmp, IGMPDBG_FLAG_TRACE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_MROUTE_ON:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, mroute, MROUTEDBG_FLAG_ON)
        break;

    case TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, mroute, MROUTEDBG_FLAG_DETAIL)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_EVENT)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_PACKET:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_PACKET)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_PACKET_DUMP_SEND)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_PACKET_DUMP_RCV)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_TRACE:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_TRACE)
        break;

    case TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA:
        CDB_FLAG_CHECK_SET(p_db_dbg, p_dbg, pim, PIMDBG_FLAG_ZEBRA)
        break;

    case TBL_QUAGGA_DEBUG_FLD_MAX:
        sal_memcpy(p_db_dbg, p_dbg, sizeof(tbl_quagga_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QUAGGA_DEBUG, field_id, p_db_dbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QUAGGA_DEBUG, field_id, p_db_dbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_quagga_debug_set_quagga_debug_field(tbl_quagga_debug_t *p_dbg, tbl_quagga_debug_field_id_t field_id)
{
    return tbl_quagga_debug_set_quagga_debug_field_sync(p_dbg, field_id, TRUE);
}

tbl_quagga_debug_t*
tbl_quagga_debug_get_quagga_debug()
{
    return _g_p_tbl_quagga_debug;
}

int32
tbl_quagga_debug_key_str2val(char *str, tbl_quagga_debug_t *p_dbg)
{
    return PM_E_NONE;
}

char*
tbl_quagga_debug_key_name_dump(tbl_quagga_debug_t *p_dbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QUAGGA_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_quagga_debug_key_value_dump(tbl_quagga_debug_t *p_dbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_quagga_debug_field_name_dump(tbl_quagga_debug_t *p_dbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QUAGGA_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QUAGGA_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_quagga_debug_field_value_dump(tbl_quagga_debug_t *p_dbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_ZEBRA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_EVENTS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_STATUS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_TIMERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_FLOODING));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_GENERATE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_INSTALL));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_REFRESH));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_EVENTS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_STATUS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_TIMERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSSA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_DD));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_HELLO));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_ACK));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ABR));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_IA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ROUTE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_SPF));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_AS4_SEGMENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_FILTERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FSM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_FSM));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_KEEPALIVES));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_UPDATES_IN));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_UPDATES_OUT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_ZEBRA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_TRACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_ON, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->mroute, MROUTEDBG_FLAG_ON));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->mroute, MROUTEDBG_FLAG_DETAIL));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_SEND));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_RCV));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_TRACE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_TRACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_ZEBRA));
    }
    return str;
}

char**
tbl_quagga_debug_table_dump(tbl_quagga_debug_t *p_dbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_quagga_debug_key_name_dump(p_dbg, buf));
    for(i=1; i<TBL_QUAGGA_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_quagga_debug_field_name_dump(p_dbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_quagga_debug_field_value_dump(p_dbg, i, buf));
    }
    return str;
}

int32
tbl_quagga_debug_field_value_parser(char *str, int32 field_id, tbl_quagga_debug_t *p_dbg)
{
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->rip, RIPDBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_PACKET, field_id))
    {
        GLB_SET_FLAG(p_dbg->rip, RIPDBG_FLAG_PACKET);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA, field_id))
    {
        GLB_SET_FLAG(p_dbg->rip, RIPDBG_FLAG_ZEBRA);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_ISM_EVENTS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_ISM_STATUS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_ISM_TIMERS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_LSA_FLOODING);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_LSA_GENERATE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_LSA_INSTALL);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_LSA_REFRESH);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_NSM_EVENTS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_NSM_STATUS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_NSM_TIMERS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_NSSA);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_PACKET_DD);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_PACKET_HELLO);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_ACK);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ABR);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_EVENT_IA);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ROUTE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF, field_id))
    {
        GLB_SET_FLAG(p_dbg->ospf, OSPFDBG_FLAG_EVENT_SPF);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_AS4_SEGMENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_FILTERS);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FSM, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_FSM);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_KEEPALIVES);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_UPDATES_IN);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_UPDATES_OUT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA, field_id))
    {
        GLB_SET_FLAG(p_dbg->bgp, BGPDBG_FLAG_ZEBRA);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->igmp, IGMPDBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET, field_id))
    {
        GLB_SET_FLAG(p_dbg->igmp, IGMPDBG_FLAG_PACKET);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE, field_id))
    {
        GLB_SET_FLAG(p_dbg->igmp, IGMPDBG_FLAG_TRACE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_ON, field_id))
    {
        GLB_SET_FLAG(p_dbg->mroute, MROUTEDBG_FLAG_ON);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL, field_id))
    {
        GLB_SET_FLAG(p_dbg->mroute, MROUTEDBG_FLAG_DETAIL);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKET, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_PACKET);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_SEND);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_RCV);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_TRACE, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_TRACE);
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA, field_id))
    {
        GLB_SET_FLAG(p_dbg->pim, PIMDBG_FLAG_ZEBRA);
    }
    return PM_E_NONE;
}

int32
tbl_quagga_debug_table_parser(char** array, char* key_value,tbl_quagga_debug_t *p_dbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QUAGGA_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_quagga_debug_key_str2val(key_value, p_dbg));

    for(i=1; i<TBL_QUAGGA_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_quagga_debug_field_value_parser( array[j++], i, p_dbg));
    }

    return PM_E_NONE;
}

int32
tbl_quagga_debug_dump_one(tbl_quagga_debug_t *p_dbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QUAGGA_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_RIP_EVENT].name,
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_RIP_PACKET].name,
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA].name,
            GLB_FLAG_ISSET(p_dbg->rip, RIPDBG_FLAG_ZEBRA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_EVENTS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_STATUS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ISM_TIMERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_FLOODING));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_GENERATE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_INSTALL));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_LSA_REFRESH));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_EVENTS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_STATUS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSM_TIMERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_NSSA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_DD));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_HELLO));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_ACK));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ABR));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_IA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_ROUTE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF].name,
            GLB_FLAG_ISSET(p_dbg->ospf, OSPFDBG_FLAG_EVENT_SPF));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_AS4_SEGMENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_EVENT].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_FILTERS));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_FSM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_FSM].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_FSM));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_KEEPALIVES));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_UPDATES_IN));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_UPDATES_OUT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA].name,
            GLB_FLAG_ISSET(p_dbg->bgp, BGPDBG_FLAG_ZEBRA));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT].name,
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET].name,
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE].name,
            GLB_FLAG_ISSET(p_dbg->igmp, IGMPDBG_FLAG_TRACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_ON, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_MROUTE_ON].name,
            GLB_FLAG_ISSET(p_dbg->mroute, MROUTEDBG_FLAG_ON));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL].name,
            GLB_FLAG_ISSET(p_dbg->mroute, MROUTEDBG_FLAG_DETAIL));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_EVENT].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_PACKET].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_SEND));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_PACKET_DUMP_RCV));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_TRACE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_TRACE].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_TRACE));
    }
    if (FLD_MATCH(TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA].name,
            GLB_FLAG_ISSET(p_dbg->pim, PIMDBG_FLAG_ZEBRA));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_quagga_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_quagga_debug_t *p_db_dbg = _g_p_tbl_quagga_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_quagga_debug_t*
tbl_quagga_debug_init_quagga_debug()
{
    _g_p_tbl_quagga_debug = XCALLOC(MEM_TBL_QUAGGA_DEBUG, sizeof(tbl_quagga_debug_t));
    return _g_p_tbl_quagga_debug;
}


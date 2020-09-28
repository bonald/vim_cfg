
#include "proto.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "cdb_data_cmp.h"

tbl_route_debug_t *_g_p_tbl_route_debug = NULL;

int32
tbl_route_debug_set_route_debug_field_sync(tbl_route_debug_t *p_rtdbg, tbl_route_debug_field_id_t field_id, uint32 sync)
{
    tbl_route_debug_t *p_db_rtdbg = _g_p_tbl_route_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, route, RTDBG_FLAG_ROUTE_IPV4)
        break;

    case TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, route, RTDBG_FLAG_ROUTE_IPV6)
        break;

    case TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, route, RTDBG_FLAG_ROUTE_NETLINK)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARP_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arp, RTDBG_FLAG_ARP_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARP_PACKET:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arp, RTDBG_FLAG_ARP_PACKET)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arp, RTDBG_FLAG_ARP_PROTO)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARP_TIMER:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arp, RTDBG_FLAG_ARP_TIMER)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO)
        break;

    case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER)
        break;

    case TBL_ROUTE_DEBUG_FLD_VRRP_ALL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, vrrp, RTDBG_FLAG_VRRP_ALL)
        break;

    case TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, vrrp, RTDBG_FLAG_VRRP_EVENTS)
        break;

    case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, vrrp, RTDBG_FLAG_VRRP_PACKET_TX)
        break;

    case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, vrrp, RTDBG_FLAG_VRRP_PACKET_RX)
        break;

    case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, vrrp, RTDBG_FLAG_VRRP_PACKET_DETAIL)
        break;

    case TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, ipsla, RTDBG_FLAG_IPSLA_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, ipsla, RTDBG_FLAG_IPSLA_PACKET)
        break;

    case TBL_ROUTE_DEBUG_FLD_TRACK_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, track, RTDBG_FLAG_TRACK_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_ND_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nd, RTDBG_FLAG_ND_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_ND_PACKET:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nd, RTDBG_FLAG_ND_PACKET)
        break;

    case TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nd, RTDBG_FLAG_ND_PROTO)
        break;

    case TBL_ROUTE_DEBUG_FLD_ND_TIMER:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nd, RTDBG_FLAG_ND_TIMER)
        break;

    case TBL_ROUTE_DEBUG_FLD_NAT_EVENT:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nat, RTDBG_FLAG_NAT_EVENT)
        break;

    case TBL_ROUTE_DEBUG_FLD_NAT_TIMER:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, nat, RTDBG_FLAG_NAT_TIMER)
        break;

    case TBL_ROUTE_DEBUG_FLD_PIMSM_ALL:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, pimsm, RTDBG_FLAG_PIMSM_ALL)
        break;

    case TBL_ROUTE_DEBUG_FLD_PIMSM_PIM:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, pimsm, RTDBG_FLAG_PIMSM_PIM)
        break;

    case TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, pimsm, RTDBG_FLAG_PIMSM_IGMP)
        break;

    case TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER:
        CDB_FLAG_CHECK_SET(p_db_rtdbg, p_rtdbg, pimsm, RTDBG_FLAG_PIMSM_TIMER)
        break;

    case TBL_ROUTE_DEBUG_FLD_MAX:
        sal_memcpy(p_db_rtdbg, p_rtdbg, sizeof(tbl_route_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ROUTE_DEBUG, field_id, p_db_rtdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ROUTE_DEBUG, field_id, p_db_rtdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_route_debug_set_route_debug_field(tbl_route_debug_t *p_rtdbg, tbl_route_debug_field_id_t field_id)
{
    return tbl_route_debug_set_route_debug_field_sync(p_rtdbg, field_id, TRUE);
}

tbl_route_debug_t*
tbl_route_debug_get_route_debug()
{
    return _g_p_tbl_route_debug;
}

int32
tbl_route_debug_key_str2val(char *str, tbl_route_debug_t *p_rtdbg)
{
    return PM_E_NONE;
}

char*
tbl_route_debug_key_name_dump(tbl_route_debug_t *p_rtdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_route_debug_key_value_dump(tbl_route_debug_t *p_rtdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_route_debug_field_name_dump(tbl_route_debug_t *p_rtdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ROUTE_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_route_debug_field_value_dump(tbl_route_debug_t *p_rtdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV4));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV6));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_NETLINK));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_ALL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_EVENTS));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_DETAIL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_TRACK_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->track, RTDBG_FLAG_TRACK_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nat, RTDBG_FLAG_NAT_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->nat, RTDBG_FLAG_NAT_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_ALL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_PIM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_PIM));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_IGMP));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_TIMER));
    }
    return str;
}

char**
tbl_route_debug_table_dump(tbl_route_debug_t *p_rtdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_route_debug_key_name_dump(p_rtdbg, buf));
    for(i=1; i<TBL_ROUTE_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_route_debug_field_name_dump(p_rtdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_route_debug_field_value_dump(p_rtdbg, i, buf));
    }
    return str;
}

int32
tbl_route_debug_field_value_parser(char *str, int32 field_id, tbl_route_debug_t *p_rtdbg)
{
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV4);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV6);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->route, RTDBG_FLAG_ROUTE_NETLINK);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arp, RTDBG_FLAG_ARP_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PACKET, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arp, RTDBG_FLAG_ARP_PACKET);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arp, RTDBG_FLAG_ARP_PROTO);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_TIMER, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arp, RTDBG_FLAG_ARP_TIMER);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_ALL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_ALL);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_EVENTS);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_TX);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_RX);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_DETAIL);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_PACKET);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_TRACK_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->track, RTDBG_FLAG_TRACK_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nd, RTDBG_FLAG_ND_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PACKET, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nd, RTDBG_FLAG_ND_PACKET);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nd, RTDBG_FLAG_ND_PROTO);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_TIMER, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nd, RTDBG_FLAG_ND_TIMER);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_EVENT, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nat, RTDBG_FLAG_NAT_EVENT);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_TIMER, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->nat, RTDBG_FLAG_NAT_TIMER);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_ALL, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_ALL);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_PIM, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_PIM);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_IGMP);
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER, field_id))
    {
        GLB_SET_FLAG(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_TIMER);
    }
    return PM_E_NONE;
}

int32
tbl_route_debug_table_parser(char** array, char* key_value,tbl_route_debug_t *p_rtdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_route_debug_key_str2val(key_value, p_rtdbg));

    for(i=1; i<TBL_ROUTE_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_route_debug_field_value_parser( array[j++], i, p_rtdbg));
    }

    return PM_E_NONE;
}

int32
tbl_route_debug_dump_one(tbl_route_debug_t *p_rtdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4].name,
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV4));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6].name,
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_IPV6));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK].name,
            GLB_FLAG_ISSET(p_rtdbg->route, RTDBG_FLAG_ROUTE_NETLINK));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARP_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARP_PACKET].name,
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL].name,
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARP_TIMER].name,
            GLB_FLAG_ISSET(p_rtdbg->arp, RTDBG_FLAG_ARP_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET].name,
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL].name,
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER].name,
            GLB_FLAG_ISSET(p_rtdbg->arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_VRRP_ALL].name,
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_ALL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS].name,
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_EVENTS));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND].name,
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_TX));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV].name,
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_RX));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL].name,
            GLB_FLAG_ISSET(p_rtdbg->vrrp, RTDBG_FLAG_VRRP_PACKET_DETAIL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET].name,
            GLB_FLAG_ISSET(p_rtdbg->ipsla, RTDBG_FLAG_IPSLA_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_TRACK_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_TRACK_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->track, RTDBG_FLAG_TRACK_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ND_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ND_PACKET].name,
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_PACKET));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL].name,
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_PROTO));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_ND_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_ND_TIMER].name,
            GLB_FLAG_ISSET(p_rtdbg->nd, RTDBG_FLAG_ND_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_NAT_EVENT].name,
            GLB_FLAG_ISSET(p_rtdbg->nat, RTDBG_FLAG_NAT_EVENT));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_NAT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_NAT_TIMER].name,
            GLB_FLAG_ISSET(p_rtdbg->nat, RTDBG_FLAG_NAT_TIMER));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_PIMSM_ALL].name,
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_ALL));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_PIM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_PIMSM_PIM].name,
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_PIM));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP].name,
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_IGMP));
    }
    if (FLD_MATCH(TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER].name,
            GLB_FLAG_ISSET(p_rtdbg->pimsm, RTDBG_FLAG_PIMSM_TIMER));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_route_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_route_debug_t *p_db_rtdbg = _g_p_tbl_route_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_rtdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_rtdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_route_debug_t*
tbl_route_debug_init_route_debug()
{
    _g_p_tbl_route_debug = XCALLOC(MEM_TBL_ROUTE_DEBUG, sizeof(tbl_route_debug_t));
    return _g_p_tbl_route_debug;
}


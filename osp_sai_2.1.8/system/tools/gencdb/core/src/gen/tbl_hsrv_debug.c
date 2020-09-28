
#include "proto.h"
#include "gen/tbl_hsrv_debug_define.h"
#include "gen/tbl_hsrv_debug.h"
#include "cdb_data_cmp.h"

tbl_hsrv_debug_t *_g_p_tbl_hsrv_debug = NULL;

int32
tbl_hsrv_debug_set_hsrv_debug_field_sync(tbl_hsrv_debug_t *p_hsrvdbg, tbl_hsrv_debug_field_id_t field_id, uint32 sync)
{
    tbl_hsrv_debug_t *p_db_hsrvdbg = _g_p_tbl_hsrv_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_HSRV_DEBUG_FLD_VSWITCH:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, vswitch, HSRVDBG_FLAG_VSWITCH_VSWITCH)
        break;

    case TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, interface, HSRVDBG_FLAG_INTERFACE_L2IF)
        break;

    case TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, interface, HSRVDBG_FLAG_INTERFACE_L3IF)
        break;

    case TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, interface, HSRVDBG_FLAG_INTERFACE_IFDB)
        break;

    case TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, message, HSRVDBG_FLAG_MESSAGE_PM2HSRV)
        break;

    case TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, message, HSRVDBG_FLAG_MESSAGE_HSRV2PM)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_RX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_TX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_BPDU:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_EAPOL:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_LLDP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_ERPS:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_MACDA:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_RIP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_OSPF:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_BGP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_ARP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_DHCP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_IPDA:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_IGMP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_OTHER:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_RAW:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW)
        break;

    case TBL_HSRV_DEBUG_FLD_CPU_PTP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP)
        break;

    case TBL_HSRV_DEBUG_FLD_IP_ICMP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ip, HSRVDBG_FLAG_IP_ICMP)
        break;

    case TBL_HSRV_DEBUG_FLD_NEXTHOP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, nexthop, HSRVDBG_FLAG_NEXTHOP_NEXTHOP)
        break;

    case TBL_HSRV_DEBUG_FLD_VLAN:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, vlan, HSRVDBG_FLAG_VLAN_VLAN)
        break;

    case TBL_HSRV_DEBUG_FLD_MIRROR:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, mirror, HSRVDBG_FLAG_MIRROR_MIRROR)
        break;

    case TBL_HSRV_DEBUG_FLD_FDB:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, fdb, HSRVDBG_FLAG_FDB_FDB)
        break;

    case TBL_HSRV_DEBUG_FLD_L2MC:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, l2mc, HSRVDBG_FLAG_L2MC_L2MC)
        break;

    case TBL_HSRV_DEBUG_FLD_AGG:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, agg, HSRVDBG_FLAG_AGG_AGG)
        break;

    case TBL_HSRV_DEBUG_FLD_IPUC:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ipuc, HSRVDBG_FLAG_IPUC_IPUC)
        break;

    case TBL_HSRV_DEBUG_FLD_NEIGHBOR:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, neighbor, HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR)
        break;

    case TBL_HSRV_DEBUG_FLD_ECMP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ecmp, HSRVDBG_FLAG_ECMP_ECMP)
        break;

    case TBL_HSRV_DEBUG_FLD_ACL:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, acl, HSRVDBG_FLAG_ACL_ACL)
        break;

    case TBL_HSRV_DEBUG_FLD_STATS:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, stats, HSRVDBG_FLAG_STATS_STATS)
        break;

    case TBL_HSRV_DEBUG_FLD_QOS_CLASS:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, qos, HSRVDBG_FLAG_QOS_CLASS)
        break;

    case TBL_HSRV_DEBUG_FLD_QOS_POLICER:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, qos, HSRVDBG_FLAG_QOS_POLICER)
        break;

    case TBL_HSRV_DEBUG_FLD_QOS_PROCESS:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, qos, HSRVDBG_FLAG_QOS_PROCESS)
        break;

    case TBL_HSRV_DEBUG_FLD_QOS_QUEUE:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, qos, HSRVDBG_FLAG_QOS_QUEUE)
        break;

    case TBL_HSRV_DEBUG_FLD_IPSOUR:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ipsour, HSRVDBG_FLAG_IPSOUR_IPSOUR)
        break;

    case TBL_HSRV_DEBUG_FLD_DOT1X:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, dot1x, HSRVDBG_FLAG_DOT1X_DOT1X)
        break;

    case TBL_HSRV_DEBUG_FLD_STP:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, stp, HSRVDBG_FLAG_STP_STP)
        break;

    case TBL_HSRV_DEBUG_FLD_PTP_PKT_RX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ptp, HSRVDBG_FLAG_PTP_PKT_RX)
        break;

    case TBL_HSRV_DEBUG_FLD_PTP_PKT_TX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ptp, HSRVDBG_FLAG_PTP_PKT_TX)
        break;

    case TBL_HSRV_DEBUG_FLD_PTP_CFG:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ptp, HSRVDBG_FLAG_PTP_CFG)
        break;

    case TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ptp, HSRVDBG_FLAG_PTP_INTERRUPT)
        break;

    case TBL_HSRV_DEBUG_FLD_IPMC:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, ipmc, HSRVDBG_FLAG_IPMC_IPMC)
        break;

    case TBL_HSRV_DEBUG_FLD_TPOAM_COMMON:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, tpoam, HSRVDBG_FLAG_TPOAM_COMMON)
        break;

    case TBL_HSRV_DEBUG_FLD_TPOAM_EVENT:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, tpoam, HSRVDBG_FLAG_TPOAM_EVENT)
        break;

    case TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, tpoam, HSRVDBG_FLAG_TPOAM_UPDATE)
        break;

    case TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, tpoam, HSRVDBG_FLAG_TPOAM_PKT_RX)
        break;

    case TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, tpoam, HSRVDBG_FLAG_TPOAM_PKT_TX)
        break;

    case TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_RX)
        break;

    case TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_TX)
        break;

    case TBL_HSRV_DEBUG_FLD_NAT_RULE:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, nat, HSRVDBG_FLAG_NAT_RULE)
        break;

    case TBL_HSRV_DEBUG_FLD_NAT_SESSION:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, nat, HSRVDBG_FLAG_NAT_SESSION)
        break;

    case TBL_HSRV_DEBUG_FLD_NAT_TIMER:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, nat, HSRVDBG_FLAG_NAT_TIMER)
        break;

    case TBL_HSRV_DEBUG_FLD_G8032_EVENT:
        CDB_FLAG_CHECK_SET(p_db_hsrvdbg, p_hsrvdbg, g8032, HSRVDBG_FLAG_G8032_EVENT)
        break;

    case TBL_HSRV_DEBUG_FLD_MAX:
        sal_memcpy(p_db_hsrvdbg, p_hsrvdbg, sizeof(tbl_hsrv_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_HSRV_DEBUG, field_id, p_db_hsrvdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_HSRV_DEBUG, field_id, p_db_hsrvdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hsrv_debug_set_hsrv_debug_field(tbl_hsrv_debug_t *p_hsrvdbg, tbl_hsrv_debug_field_id_t field_id)
{
    return tbl_hsrv_debug_set_hsrv_debug_field_sync(p_hsrvdbg, field_id, TRUE);
}

tbl_hsrv_debug_t*
tbl_hsrv_debug_get_hsrv_debug()
{
    return _g_p_tbl_hsrv_debug;
}

int32
tbl_hsrv_debug_key_str2val(char *str, tbl_hsrv_debug_t *p_hsrvdbg)
{
    return PM_E_NONE;
}

char*
tbl_hsrv_debug_key_name_dump(tbl_hsrv_debug_t *p_hsrvdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HSRV_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_hsrv_debug_key_value_dump(tbl_hsrv_debug_t *p_hsrvdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_hsrv_debug_field_name_dump(tbl_hsrv_debug_t *p_hsrvdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_HSRV_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_HSRV_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_hsrv_debug_field_value_dump(tbl_hsrv_debug_t *p_hsrvdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VSWITCH, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->vswitch, HSRVDBG_FLAG_VSWITCH_VSWITCH));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L2IF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L3IF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_IFDB));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_PM2HSRV));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_HSRV2PM));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BPDU, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_EAPOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_LLDP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ERPS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACDA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RIP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OSPF, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BGP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ARP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_DHCP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IPDA, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IGMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OTHER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RAW, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_PTP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IP_ICMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ip, HSRVDBG_FLAG_IP_ICMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEXTHOP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->nexthop, HSRVDBG_FLAG_NEXTHOP_NEXTHOP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VLAN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->vlan, HSRVDBG_FLAG_VLAN_VLAN));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MIRROR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->mirror, HSRVDBG_FLAG_MIRROR_MIRROR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_FDB, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->fdb, HSRVDBG_FLAG_FDB_FDB));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2MC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->l2mc, HSRVDBG_FLAG_L2MC_L2MC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_AGG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->agg, HSRVDBG_FLAG_AGG_AGG));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPUC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ipuc, HSRVDBG_FLAG_IPUC_IPUC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->neighbor, HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ECMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ecmp, HSRVDBG_FLAG_ECMP_ECMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ACL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->acl, HSRVDBG_FLAG_ACL_ACL));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->stats, HSRVDBG_FLAG_STATS_STATS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_CLASS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_CLASS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_POLICER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_POLICER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_PROCESS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_PROCESS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_QUEUE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_QUEUE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPSOUR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ipsour, HSRVDBG_FLAG_IPSOUR_IPSOUR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_DOT1X, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->dot1x, HSRVDBG_FLAG_DOT1X_DOT1X));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->stp, HSRVDBG_FLAG_STP_STP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_CFG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_CFG));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_INTERRUPT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPMC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->ipmc, HSRVDBG_FLAG_IPMC_IPMC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_COMMON, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_COMMON));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_EVENT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_UPDATE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_RULE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_RULE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_SESSION, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_SESSION));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_TIMER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_G8032_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_hsrvdbg->g8032, HSRVDBG_FLAG_G8032_EVENT));
    }
    return str;
}

char**
tbl_hsrv_debug_table_dump(tbl_hsrv_debug_t *p_hsrvdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_hsrv_debug_key_name_dump(p_hsrvdbg, buf));
    for(i=1; i<TBL_HSRV_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_hsrv_debug_field_name_dump(p_hsrvdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_hsrv_debug_field_value_dump(p_hsrvdbg, i, buf));
    }
    return str;
}

int32
tbl_hsrv_debug_field_value_parser(char *str, int32 field_id, tbl_hsrv_debug_t *p_hsrvdbg)
{
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VSWITCH, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->vswitch, HSRVDBG_FLAG_VSWITCH_VSWITCH);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L2IF);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L3IF);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_IFDB);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_PM2HSRV);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_HSRV2PM);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_TX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BPDU, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_EAPOL, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_LLDP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ERPS, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACDA, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RIP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OSPF, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BGP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ARP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_DHCP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IPDA, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IGMP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OTHER, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RAW, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_PTP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IP_ICMP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ip, HSRVDBG_FLAG_IP_ICMP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEXTHOP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->nexthop, HSRVDBG_FLAG_NEXTHOP_NEXTHOP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VLAN, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->vlan, HSRVDBG_FLAG_VLAN_VLAN);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MIRROR, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->mirror, HSRVDBG_FLAG_MIRROR_MIRROR);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_FDB, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->fdb, HSRVDBG_FLAG_FDB_FDB);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2MC, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->l2mc, HSRVDBG_FLAG_L2MC_L2MC);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_AGG, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->agg, HSRVDBG_FLAG_AGG_AGG);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPUC, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ipuc, HSRVDBG_FLAG_IPUC_IPUC);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEIGHBOR, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->neighbor, HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ECMP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ecmp, HSRVDBG_FLAG_ECMP_ECMP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ACL, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->acl, HSRVDBG_FLAG_ACL_ACL);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STATS, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->stats, HSRVDBG_FLAG_STATS_STATS);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_CLASS, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_CLASS);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_POLICER, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_POLICER);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_PROCESS, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_PROCESS);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_QUEUE, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_QUEUE);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPSOUR, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ipsour, HSRVDBG_FLAG_IPSOUR_IPSOUR);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_DOT1X, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->dot1x, HSRVDBG_FLAG_DOT1X_DOT1X);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STP, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->stp, HSRVDBG_FLAG_STP_STP);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_RX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_RX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_TX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_TX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_CFG, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_CFG);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_INTERRUPT);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPMC, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->ipmc, HSRVDBG_FLAG_IPMC_IPMC);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_COMMON, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_COMMON);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_EVENT, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_EVENT);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_UPDATE);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_RX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_TX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_RX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_TX);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_RULE, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_RULE);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_SESSION, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_SESSION);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_TIMER, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_TIMER);
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_G8032_EVENT, field_id))
    {
        GLB_SET_FLAG(p_hsrvdbg->g8032, HSRVDBG_FLAG_G8032_EVENT);
    }
    return PM_E_NONE;
}

int32
tbl_hsrv_debug_table_parser(char** array, char* key_value,tbl_hsrv_debug_t *p_hsrvdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_HSRV_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_hsrv_debug_key_str2val(key_value, p_hsrvdbg));

    for(i=1; i<TBL_HSRV_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_hsrv_debug_field_value_parser( array[j++], i, p_hsrvdbg));
    }

    return PM_E_NONE;
}

int32
tbl_hsrv_debug_dump_one(tbl_hsrv_debug_t *p_hsrvdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HSRV_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VSWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_VSWITCH].name,
            GLB_FLAG_ISSET(p_hsrvdbg->vswitch, HSRVDBG_FLAG_VSWITCH_VSWITCH));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF].name,
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L2IF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF].name,
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_L3IF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB].name,
            GLB_FLAG_ISSET(p_hsrvdbg->interface, HSRVDBG_FLAG_INTERFACE_IFDB));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV].name,
            GLB_FLAG_ISSET(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_PM2HSRV));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM].name,
            GLB_FLAG_ISSET(p_hsrvdbg->message, HSRVDBG_FLAG_MESSAGE_HSRV2PM));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_RX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_TX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_BPDU].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_EAPOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_EAPOL].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_LLDP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_LLDP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ERPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_ERPS].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_MACDA].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_RIP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OSPF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_OSPF].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_BGP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_BGP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_ARP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_ARP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_DHCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_DHCP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IPDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_IPDA].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_IGMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_IGMP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_OTHER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_OTHER].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_RAW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_RAW].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_CPU_PTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_CPU_PTP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IP_ICMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_IP_ICMP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ip, HSRVDBG_FLAG_IP_ICMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEXTHOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_NEXTHOP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->nexthop, HSRVDBG_FLAG_NEXTHOP_NEXTHOP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_VLAN].name,
            GLB_FLAG_ISSET(p_hsrvdbg->vlan, HSRVDBG_FLAG_VLAN_VLAN));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_MIRROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_MIRROR].name,
            GLB_FLAG_ISSET(p_hsrvdbg->mirror, HSRVDBG_FLAG_MIRROR_MIRROR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_FDB].name,
            GLB_FLAG_ISSET(p_hsrvdbg->fdb, HSRVDBG_FLAG_FDB_FDB));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2MC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_L2MC].name,
            GLB_FLAG_ISSET(p_hsrvdbg->l2mc, HSRVDBG_FLAG_L2MC_L2MC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_AGG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_AGG].name,
            GLB_FLAG_ISSET(p_hsrvdbg->agg, HSRVDBG_FLAG_AGG_AGG));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPUC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_IPUC].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ipuc, HSRVDBG_FLAG_IPUC_IPUC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_NEIGHBOR].name,
            GLB_FLAG_ISSET(p_hsrvdbg->neighbor, HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ECMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_ECMP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ecmp, HSRVDBG_FLAG_ECMP_ECMP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_ACL].name,
            GLB_FLAG_ISSET(p_hsrvdbg->acl, HSRVDBG_FLAG_ACL_ACL));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_STATS].name,
            GLB_FLAG_ISSET(p_hsrvdbg->stats, HSRVDBG_FLAG_STATS_STATS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_QOS_CLASS].name,
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_CLASS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_QOS_POLICER].name,
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_POLICER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_PROCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_QOS_PROCESS].name,
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_PROCESS));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_QOS_QUEUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_QOS_QUEUE].name,
            GLB_FLAG_ISSET(p_hsrvdbg->qos, HSRVDBG_FLAG_QOS_QUEUE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPSOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_IPSOUR].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ipsour, HSRVDBG_FLAG_IPSOUR_IPSOUR));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_DOT1X, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_DOT1X].name,
            GLB_FLAG_ISSET(p_hsrvdbg->dot1x, HSRVDBG_FLAG_DOT1X_DOT1X));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_STP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_STP].name,
            GLB_FLAG_ISSET(p_hsrvdbg->stp, HSRVDBG_FLAG_STP_STP));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_PTP_PKT_RX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_PKT_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_PTP_PKT_TX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_CFG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_PTP_CFG].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_CFG));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ptp, HSRVDBG_FLAG_PTP_INTERRUPT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_IPMC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_IPMC].name,
            GLB_FLAG_ISSET(p_hsrvdbg->ipmc, HSRVDBG_FLAG_IPMC_IPMC));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_COMMON, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_TPOAM_COMMON].name,
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_COMMON));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_TPOAM_EVENT].name,
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_EVENT));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE].name,
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_UPDATE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->tpoam, HSRVDBG_FLAG_TPOAM_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_RX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX].name,
            GLB_FLAG_ISSET(p_hsrvdbg->l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_TX));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_RULE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_NAT_RULE].name,
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_RULE));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_SESSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_NAT_SESSION].name,
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_SESSION));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_NAT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_NAT_TIMER].name,
            GLB_FLAG_ISSET(p_hsrvdbg->nat, HSRVDBG_FLAG_NAT_TIMER));
    }
    if (FLD_MATCH(TBL_HSRV_DEBUG_FLD_G8032_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HSRV_DEBUG_FLD_G8032_EVENT].name,
            GLB_FLAG_ISSET(p_hsrvdbg->g8032, HSRVDBG_FLAG_G8032_EVENT));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_hsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_hsrv_debug_t *p_db_hsrvdbg = _g_p_tbl_hsrv_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_hsrvdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_hsrvdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_hsrv_debug_t*
tbl_hsrv_debug_init_hsrv_debug()
{
    _g_p_tbl_hsrv_debug = XCALLOC(MEM_TBL_HSRV_DEBUG, sizeof(tbl_hsrv_debug_t));
    return _g_p_tbl_hsrv_debug;
}


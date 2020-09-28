
#include "proto.h"
#include "gen/tbl_cpu_packets_define.h"
#include "gen/tbl_cpu_packets.h"
#include "cdb_data_cmp.h"

tbl_cpu_packets_t *_g_p_tbl_cpu_packets = NULL;

int32
tbl_cpu_packets_set_cpu_packets_field_sync(tbl_cpu_packets_t *p_cpu_packets, tbl_cpu_packets_field_id_t field_id, uint32 sync)
{
    tbl_cpu_packets_t *p_db_cpu_packets = _g_p_tbl_cpu_packets;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_PACKETS_FLD_TOTAL:
        p_db_cpu_packets->total = p_cpu_packets->total;
        break;

    case TBL_CPU_PACKETS_FLD_RX:
        p_db_cpu_packets->rx = p_cpu_packets->rx;
        break;

    case TBL_CPU_PACKETS_FLD_TX:
        p_db_cpu_packets->tx = p_cpu_packets->tx;
        break;

    case TBL_CPU_PACKETS_FLD_RX_BPDU:
        p_db_cpu_packets->rx_bpdu = p_cpu_packets->rx_bpdu;
        break;

    case TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO:
        p_db_cpu_packets->rx_slow_proto = p_cpu_packets->rx_slow_proto;
        break;

    case TBL_CPU_PACKETS_FLD_RX_EAPOL:
        p_db_cpu_packets->rx_eapol = p_cpu_packets->rx_eapol;
        break;

    case TBL_CPU_PACKETS_FLD_RX_LLDP:
        p_db_cpu_packets->rx_lldp = p_cpu_packets->rx_lldp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_ERPS:
        p_db_cpu_packets->rx_erps = p_cpu_packets->rx_erps;
        break;

    case TBL_CPU_PACKETS_FLD_RX_MACDA:
        p_db_cpu_packets->rx_macda = p_cpu_packets->rx_macda;
        break;

    case TBL_CPU_PACKETS_FLD_RX_RIP:
        p_db_cpu_packets->rx_rip = p_cpu_packets->rx_rip;
        break;

    case TBL_CPU_PACKETS_FLD_RX_OSPF:
        p_db_cpu_packets->rx_ospf = p_cpu_packets->rx_ospf;
        break;

    case TBL_CPU_PACKETS_FLD_RX_BGP:
        p_db_cpu_packets->rx_bgp = p_cpu_packets->rx_bgp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_ARP:
        p_db_cpu_packets->rx_arp = p_cpu_packets->rx_arp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_DHCP:
        p_db_cpu_packets->rx_dhcp = p_cpu_packets->rx_dhcp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_IPDA:
        p_db_cpu_packets->rx_ipda = p_cpu_packets->rx_ipda;
        break;

    case TBL_CPU_PACKETS_FLD_RX_IGMP:
        p_db_cpu_packets->rx_igmp = p_cpu_packets->rx_igmp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT:
        p_db_cpu_packets->rx_mac_limit = p_cpu_packets->rx_mac_limit;
        break;

    case TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH:
        p_db_cpu_packets->rx_mac_mismatch = p_cpu_packets->rx_mac_mismatch;
        break;

    case TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU:
        p_db_cpu_packets->rx_l3copy_cpu = p_cpu_packets->rx_l3copy_cpu;
        break;

    case TBL_CPU_PACKETS_FLD_RX_TTL_ERROR:
        p_db_cpu_packets->rx_ttl_error = p_cpu_packets->rx_ttl_error;
        break;

    case TBL_CPU_PACKETS_FLD_RX_PTP:
        p_db_cpu_packets->rx_ptp = p_cpu_packets->rx_ptp;
        break;

    case TBL_CPU_PACKETS_FLD_RX_TPOAM:
        p_db_cpu_packets->rx_tpoam = p_cpu_packets->rx_tpoam;
        break;

    case TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC:
        p_db_cpu_packets->rx_l2pro_protocol_mac = p_cpu_packets->rx_l2pro_protocol_mac;
        break;

    case TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC:
        p_db_cpu_packets->rx_l2pro_group_mac = p_cpu_packets->rx_l2pro_group_mac;
        break;

    case TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR:
        p_db_cpu_packets->rx_cpu_mirror = p_cpu_packets->rx_cpu_mirror;
        break;

    case TBL_CPU_PACKETS_FLD_RX_ND:
        p_db_cpu_packets->rx_nd = p_cpu_packets->rx_nd;
        break;

    case TBL_CPU_PACKETS_FLD_RX_PACKETIN:
        p_db_cpu_packets->rx_packetin = p_cpu_packets->rx_packetin;
        break;

    case TBL_CPU_PACKETS_FLD_RX_MTU_FAIL:
        p_db_cpu_packets->rx_mtu_fail = p_cpu_packets->rx_mtu_fail;
        break;

    case TBL_CPU_PACKETS_FLD_RX_G8032:
        p_db_cpu_packets->rx_g8032 = p_cpu_packets->rx_g8032;
        break;

    case TBL_CPU_PACKETS_FLD_RX_OTHER:
        p_db_cpu_packets->rx_other = p_cpu_packets->rx_other;
        break;

    case TBL_CPU_PACKETS_FLD_TX_IGMP:
        p_db_cpu_packets->tx_igmp = p_cpu_packets->tx_igmp;
        break;

    case TBL_CPU_PACKETS_FLD_TX_BPDU:
        p_db_cpu_packets->tx_bpdu = p_cpu_packets->tx_bpdu;
        break;

    case TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO:
        p_db_cpu_packets->tx_slow_proto = p_cpu_packets->tx_slow_proto;
        break;

    case TBL_CPU_PACKETS_FLD_TX_ARP:
        p_db_cpu_packets->tx_arp = p_cpu_packets->tx_arp;
        break;

    case TBL_CPU_PACKETS_FLD_TX_DHCP:
        p_db_cpu_packets->tx_dhcp = p_cpu_packets->tx_dhcp;
        break;

    case TBL_CPU_PACKETS_FLD_TX_LLDP:
        p_db_cpu_packets->tx_lldp = p_cpu_packets->tx_lldp;
        break;

    case TBL_CPU_PACKETS_FLD_TX_ERPS:
        p_db_cpu_packets->tx_erps = p_cpu_packets->tx_erps;
        break;

    case TBL_CPU_PACKETS_FLD_TX_EAPOL:
        p_db_cpu_packets->tx_eapol = p_cpu_packets->tx_eapol;
        break;

    case TBL_CPU_PACKETS_FLD_TX_PTP:
        p_db_cpu_packets->tx_ptp = p_cpu_packets->tx_ptp;
        break;

    case TBL_CPU_PACKETS_FLD_TX_TPOAM:
        p_db_cpu_packets->tx_tpoam = p_cpu_packets->tx_tpoam;
        break;

    case TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC:
        p_db_cpu_packets->tx_l2pro_protocol_mac = p_cpu_packets->tx_l2pro_protocol_mac;
        break;

    case TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC:
        p_db_cpu_packets->tx_l2pro_group_mac = p_cpu_packets->tx_l2pro_group_mac;
        break;

    case TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR:
        p_db_cpu_packets->tx_cpu_mirror = p_cpu_packets->tx_cpu_mirror;
        break;

    case TBL_CPU_PACKETS_FLD_TX_ND:
        p_db_cpu_packets->tx_nd = p_cpu_packets->tx_nd;
        break;

    case TBL_CPU_PACKETS_FLD_TX_G8032:
        p_db_cpu_packets->tx_g8032 = p_cpu_packets->tx_g8032;
        break;

    case TBL_CPU_PACKETS_FLD_TX_OTHER:
        p_db_cpu_packets->tx_other = p_cpu_packets->tx_other;
        break;

    case TBL_CPU_PACKETS_FLD_MAX:
        sal_memcpy(p_db_cpu_packets, p_cpu_packets, sizeof(tbl_cpu_packets_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_PACKETS, field_id, p_db_cpu_packets);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_PACKETS, field_id, p_db_cpu_packets);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_packets_set_cpu_packets_field(tbl_cpu_packets_t *p_cpu_packets, tbl_cpu_packets_field_id_t field_id)
{
    return tbl_cpu_packets_set_cpu_packets_field_sync(p_cpu_packets, field_id, TRUE);
}

tbl_cpu_packets_t*
tbl_cpu_packets_get_cpu_packets()
{
    return _g_p_tbl_cpu_packets;
}

int32
tbl_cpu_packets_key_str2val(char *str, tbl_cpu_packets_t *p_cpu_packets)
{
    return PM_E_NONE;
}

char*
tbl_cpu_packets_key_name_dump(tbl_cpu_packets_t *p_cpu_packets, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_PACKETS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_cpu_packets_key_value_dump(tbl_cpu_packets_t *p_cpu_packets, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_cpu_packets_field_name_dump(tbl_cpu_packets_t *p_cpu_packets, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_PACKETS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_PACKETS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_packets_field_value_dump(tbl_cpu_packets_t *p_cpu_packets, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TOTAL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->total);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BPDU, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_bpdu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_slow_proto);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_EAPOL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_eapol);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_LLDP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_lldp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ERPS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_erps);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MACDA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_macda);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_RIP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_rip);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OSPF, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_ospf);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BGP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_bgp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ARP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_arp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_DHCP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_dhcp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IPDA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_ipda);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IGMP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_igmp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_mac_limit);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_mac_mismatch);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_l3copy_cpu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TTL_ERROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_ttl_error);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PTP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_ptp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TPOAM, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_tpoam);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_l2pro_protocol_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_l2pro_group_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_cpu_mirror);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_nd);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PACKETIN, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_packetin);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MTU_FAIL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_mtu_fail);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_G8032, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_g8032);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OTHER, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->rx_other);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_IGMP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_igmp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_BPDU, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_bpdu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_slow_proto);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ARP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_arp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_DHCP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_dhcp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_LLDP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_lldp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ERPS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_erps);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_EAPOL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_eapol);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_PTP, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_ptp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_TPOAM, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_tpoam);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_l2pro_protocol_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_l2pro_group_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_cpu_mirror);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ND, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_nd);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_G8032, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_g8032);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_OTHER, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_packets->tx_other);
    }
    return str;
}

char**
tbl_cpu_packets_table_dump(tbl_cpu_packets_t *p_cpu_packets, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_packets_key_name_dump(p_cpu_packets, buf));
    for(i=1; i<TBL_CPU_PACKETS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_packets_field_name_dump(p_cpu_packets, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_packets_field_value_dump(p_cpu_packets, i, buf));
    }
    return str;
}

int32
tbl_cpu_packets_field_value_parser(char *str, int32 field_id, tbl_cpu_packets_t *p_cpu_packets)
{
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TOTAL, field_id))
    {
        int32 ret;
        p_cpu_packets->total = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX, field_id))
    {
        int32 ret;
        p_cpu_packets->rx = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX, field_id))
    {
        int32 ret;
        p_cpu_packets->tx = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BPDU, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_bpdu = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_slow_proto = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_EAPOL, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_eapol = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_LLDP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_lldp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ERPS, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_erps = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MACDA, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_macda = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_RIP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_rip = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OSPF, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_ospf = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BGP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_bgp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ARP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_arp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_DHCP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_dhcp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IPDA, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_ipda = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IGMP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_igmp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_mac_limit = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_mac_mismatch = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_l3copy_cpu = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TTL_ERROR, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_ttl_error = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PTP, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_ptp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TPOAM, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_tpoam = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_l2pro_protocol_mac = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_l2pro_group_mac = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_cpu_mirror = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ND, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_nd = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PACKETIN, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_packetin = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MTU_FAIL, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_mtu_fail = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_G8032, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_g8032 = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OTHER, field_id))
    {
        int32 ret;
        p_cpu_packets->rx_other = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_IGMP, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_igmp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_BPDU, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_bpdu = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_slow_proto = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ARP, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_arp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_DHCP, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_dhcp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_LLDP, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_lldp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ERPS, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_erps = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_EAPOL, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_eapol = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_PTP, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_ptp = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_TPOAM, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_tpoam = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_l2pro_protocol_mac = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_l2pro_group_mac = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_cpu_mirror = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ND, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_nd = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_G8032, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_g8032 = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_OTHER, field_id))
    {
        int32 ret;
        p_cpu_packets->tx_other = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_packets_table_parser(char** array, char* key_value,tbl_cpu_packets_t *p_cpu_packets)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_PACKETS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_packets_key_str2val(key_value, p_cpu_packets));

    for(i=1; i<TBL_CPU_PACKETS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_packets_field_value_parser( array[j++], i, p_cpu_packets));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_packets_dump_one(tbl_cpu_packets_t *p_cpu_packets, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_PACKETS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TOTAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TOTAL].name,
            p_cpu_packets->total);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX].name,
            p_cpu_packets->rx);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX].name,
            p_cpu_packets->tx);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_BPDU].name,
            p_cpu_packets->rx_bpdu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO].name,
            p_cpu_packets->rx_slow_proto);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_EAPOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_EAPOL].name,
            p_cpu_packets->rx_eapol);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_LLDP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_LLDP].name,
            p_cpu_packets->rx_lldp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ERPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_ERPS].name,
            p_cpu_packets->rx_erps);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MACDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_MACDA].name,
            p_cpu_packets->rx_macda);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_RIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_RIP].name,
            p_cpu_packets->rx_rip);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OSPF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_OSPF].name,
            p_cpu_packets->rx_ospf);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_BGP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_BGP].name,
            p_cpu_packets->rx_bgp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ARP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_ARP].name,
            p_cpu_packets->rx_arp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_DHCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_DHCP].name,
            p_cpu_packets->rx_dhcp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IPDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_IPDA].name,
            p_cpu_packets->rx_ipda);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_IGMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_IGMP].name,
            p_cpu_packets->rx_igmp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT].name,
            p_cpu_packets->rx_mac_limit);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH].name,
            p_cpu_packets->rx_mac_mismatch);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU].name,
            p_cpu_packets->rx_l3copy_cpu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TTL_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_TTL_ERROR].name,
            p_cpu_packets->rx_ttl_error);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_PTP].name,
            p_cpu_packets->rx_ptp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_TPOAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_TPOAM].name,
            p_cpu_packets->rx_tpoam);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC].name,
            p_cpu_packets->rx_l2pro_protocol_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC].name,
            p_cpu_packets->rx_l2pro_group_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR].name,
            p_cpu_packets->rx_cpu_mirror);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_ND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_ND].name,
            p_cpu_packets->rx_nd);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_PACKETIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_PACKETIN].name,
            p_cpu_packets->rx_packetin);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_MTU_FAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_MTU_FAIL].name,
            p_cpu_packets->rx_mtu_fail);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_G8032, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_G8032].name,
            p_cpu_packets->rx_g8032);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_RX_OTHER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_RX_OTHER].name,
            p_cpu_packets->rx_other);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_IGMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_IGMP].name,
            p_cpu_packets->tx_igmp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_BPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_BPDU].name,
            p_cpu_packets->tx_bpdu);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO].name,
            p_cpu_packets->tx_slow_proto);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ARP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_ARP].name,
            p_cpu_packets->tx_arp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_DHCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_DHCP].name,
            p_cpu_packets->tx_dhcp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_LLDP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_LLDP].name,
            p_cpu_packets->tx_lldp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ERPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_ERPS].name,
            p_cpu_packets->tx_erps);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_EAPOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_EAPOL].name,
            p_cpu_packets->tx_eapol);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_PTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_PTP].name,
            p_cpu_packets->tx_ptp);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_TPOAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_TPOAM].name,
            p_cpu_packets->tx_tpoam);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC].name,
            p_cpu_packets->tx_l2pro_protocol_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC].name,
            p_cpu_packets->tx_l2pro_group_mac);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR].name,
            p_cpu_packets->tx_cpu_mirror);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_ND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_ND].name,
            p_cpu_packets->tx_nd);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_G8032, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_G8032].name,
            p_cpu_packets->tx_g8032);
    }
    if (FLD_MATCH(TBL_CPU_PACKETS_FLD_TX_OTHER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_PACKETS_FLD_TX_OTHER].name,
            p_cpu_packets->tx_other);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_packets_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_packets_t *p_db_cpu_packets = _g_p_tbl_cpu_packets;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_cpu_packets)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_cpu_packets, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_packets_t*
tbl_cpu_packets_init_cpu_packets()
{
    _g_p_tbl_cpu_packets = XCALLOC(MEM_TBL_CPU_PACKETS, sizeof(tbl_cpu_packets_t));
    return _g_p_tbl_cpu_packets;
}


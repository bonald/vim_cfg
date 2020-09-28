
#include "proto.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "cdb_data_cmp.h"

tbl_route_global_t *_g_p_tbl_route_global = NULL;

int32
tbl_route_global_set_route_global_field_sync(tbl_route_global_t *p_rt_glb, tbl_route_global_field_id_t field_id, uint32 sync)
{
    tbl_route_global_t *p_db_rt_glb = _g_p_tbl_route_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN:
        p_db_rt_glb->gratuitous_arp_learn_en = p_rt_glb->gratuitous_arp_learn_en;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT:
        p_db_rt_glb->arp_pkt_rx_count = p_rt_glb->arp_pkt_rx_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT:
        p_db_rt_glb->arp_pkt_tx_count = p_rt_glb->arp_pkt_tx_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT:
        p_db_rt_glb->arp_pkt_discard_count = p_rt_glb->arp_pkt_discard_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARP_COUNT:
        p_db_rt_glb->arp_count = p_rt_glb->arp_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM:
        p_db_rt_glb->multipath_num = p_rt_glb->multipath_num;
        break;

    case TBL_ROUTE_GLOBAL_FLD_MAX_STATIC:
        p_db_rt_glb->max_static = p_rt_glb->max_static;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT:
        p_db_rt_glb->icmp_error_ratelimit = p_rt_glb->icmp_error_ratelimit;
        break;

    case TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN:
        p_db_rt_glb->ip_redirects_global_en = p_rt_glb->ip_redirects_global_en;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES:
        p_db_rt_glb->current_static_routes = p_rt_glb->current_static_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES:
        p_db_rt_glb->current_ecmp_routes = p_rt_glb->current_ecmp_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS:
        p_db_rt_glb->current_ecmp_groups = p_rt_glb->current_ecmp_groups;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES:
        p_db_rt_glb->current_remote_routes = p_rt_glb->current_remote_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES:
        p_db_rt_glb->current_host_routes = p_rt_glb->current_host_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM:
        p_db_rt_glb->current_indirect_ipv4_route_num = p_rt_glb->current_indirect_ipv4_route_num;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES:
        p_db_rt_glb->current_static_v6_routes = p_rt_glb->current_static_v6_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES:
        p_db_rt_glb->current_ecmp_v6_routes = p_rt_glb->current_ecmp_v6_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS:
        p_db_rt_glb->current_ecmp_v6_groups = p_rt_glb->current_ecmp_v6_groups;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES:
        p_db_rt_glb->current_remote_v6_routes = p_rt_glb->current_remote_v6_routes;
        break;

    case TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM:
        p_db_rt_glb->current_indirect_ipv6_route_num = p_rt_glb->current_indirect_ipv6_route_num;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL:
        p_db_rt_glb->active_local = p_rt_glb->active_local;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC:
        p_db_rt_glb->active_static = p_rt_glb->active_static;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED:
        p_db_rt_glb->active_connected = p_rt_glb->active_connected;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP:
        p_db_rt_glb->active_rip = p_rt_glb->active_rip;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF:
        p_db_rt_glb->active_ospf = p_rt_glb->active_ospf;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP:
        p_db_rt_glb->active_bgp = p_rt_glb->active_bgp;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM:
        p_db_rt_glb->arpinsp_logbuf_entrynum = p_rt_glb->arpinsp_logbuf_entrynum;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM:
        p_db_rt_glb->arpinsp_logbuf_curnum = p_rt_glb->arpinsp_logbuf_curnum;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM:
        p_db_rt_glb->arpinsp_logbuf_lognum = p_rt_glb->arpinsp_logbuf_lognum;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC:
        p_db_rt_glb->arpinsp_logbuf_logsec = p_rt_glb->arpinsp_logbuf_logsec;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG:
        p_db_rt_glb->arpinsp_validate_flag = p_rt_glb->arpinsp_validate_flag;
        break;

    case TBL_ROUTE_GLOBAL_FLD_FIB_FULL:
        p_db_rt_glb->fib_full = p_rt_glb->fib_full;
        break;

    case TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE:
        p_db_rt_glb->ipv6_enable = p_rt_glb->ipv6_enable;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT:
        p_db_rt_glb->nd_hop_limit = p_rt_glb->nd_hop_limit;
        break;

    case TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED:
        p_db_rt_glb->vty_connected = p_rt_glb->vty_connected;
        break;

    case TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT:
        p_db_rt_glb->vty_nonconn_count = p_rt_glb->vty_nonconn_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_VTY_MODE:
        p_db_rt_glb->vty_mode = p_rt_glb->vty_mode;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER:
        p_db_rt_glb->arp_aging_timer = p_rt_glb->arp_aging_timer;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER:
        p_db_rt_glb->arpratelimit_timer = p_rt_glb->arpratelimit_timer;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC:
        sal_memcpy(&p_db_rt_glb->arpratelimit_timer_start_time_sec, &p_rt_glb->arpratelimit_timer_start_time_sec, sizeof(p_rt_glb->arpratelimit_timer_start_time_sec));
        break;

    case TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC:
        sal_memcpy(&p_db_rt_glb->arpratelimit_timer_start_time_msec, &p_rt_glb->arpratelimit_timer_start_time_msec, sizeof(p_rt_glb->arpratelimit_timer_start_time_msec));
        break;

    case TBL_ROUTE_GLOBAL_FLD_BUF:
        sal_memcpy(p_db_rt_glb->buf, p_rt_glb->buf, sizeof(p_rt_glb->buf));
        break;

    case TBL_ROUTE_GLOBAL_FLD_PKT_LEN:
        p_db_rt_glb->pkt_len = p_rt_glb->pkt_len;
        break;

    case TBL_ROUTE_GLOBAL_FLD_ND_COUNT:
        p_db_rt_glb->nd_count = p_rt_glb->nd_count;
        break;

    case TBL_ROUTE_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_rt_glb, p_rt_glb, sizeof(tbl_route_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ROUTE_GLOBAL, field_id, p_db_rt_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ROUTE_GLOBAL, field_id, p_db_rt_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_route_global_set_route_global_field(tbl_route_global_t *p_rt_glb, tbl_route_global_field_id_t field_id)
{
    return tbl_route_global_set_route_global_field_sync(p_rt_glb, field_id, TRUE);
}

tbl_route_global_t*
tbl_route_global_get_route_global()
{
    return _g_p_tbl_route_global;
}

int32
tbl_route_global_key_str2val(char *str, tbl_route_global_t *p_rt_glb)
{
    return PM_E_NONE;
}

char*
tbl_route_global_key_name_dump(tbl_route_global_t *p_rt_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_route_global_key_value_dump(tbl_route_global_t *p_rt_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_route_global_field_name_dump(tbl_route_global_t *p_rt_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ROUTE_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_route_global_field_value_dump(tbl_route_global_t *p_rt_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->gratuitous_arp_learn_en);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arp_pkt_rx_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arp_pkt_tx_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arp_pkt_discard_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arp_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->multipath_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MAX_STATIC, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->max_static);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->icmp_error_ratelimit);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->ip_redirects_global_en);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_static_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_ecmp_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_ecmp_groups);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_remote_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_host_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_indirect_ipv4_route_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_static_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_ecmp_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_ecmp_v6_groups);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_remote_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->current_indirect_ipv6_route_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_local);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_static);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_connected);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_rip);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_ospf);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->active_bgp);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arpinsp_logbuf_entrynum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arpinsp_logbuf_curnum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arpinsp_logbuf_lognum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arpinsp_logbuf_logsec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->arpinsp_validate_flag);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_FIB_FULL, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->fib_full);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->ipv6_enable);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->nd_hop_limit);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->vty_connected);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->vty_nonconn_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_vty_mode_strs, GLB_VTY_MODE_MAX, p_rt_glb->vty_mode));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rt_glb->arp_aging_timer);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_rt_glb->arpratelimit_timer);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_rt_glb->arpratelimit_timer_start_time_sec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_rt_glb->arpratelimit_timer_start_time_msec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_rt_glb->buf, sizeof(p_rt_glb->buf));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_PKT_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->pkt_len);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_glb->nd_count);
    }
    return str;
}

char**
tbl_route_global_table_dump(tbl_route_global_t *p_rt_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_route_global_key_name_dump(p_rt_glb, buf));
    for(i=1; i<TBL_ROUTE_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_route_global_field_name_dump(p_rt_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_route_global_field_value_dump(p_rt_glb, i, buf));
    }
    return str;
}

int32
tbl_route_global_field_value_parser(char *str, int32 field_id, tbl_route_global_t *p_rt_glb)
{
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN, field_id))
    {
        int32 ret;
        p_rt_glb->gratuitous_arp_learn_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->arp_pkt_rx_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->arp_pkt_tx_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->arp_pkt_discard_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->arp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM, field_id))
    {
        int32 ret;
        p_rt_glb->multipath_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MAX_STATIC, field_id))
    {
        int32 ret;
        p_rt_glb->max_static = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT, field_id))
    {
        int32 ret;
        p_rt_glb->icmp_error_ratelimit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN, field_id))
    {
        int32 ret;
        p_rt_glb->ip_redirects_global_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_static_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_ecmp_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS, field_id))
    {
        int32 ret;
        p_rt_glb->current_ecmp_groups = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_remote_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_host_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM, field_id))
    {
        int32 ret;
        p_rt_glb->current_indirect_ipv4_route_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_static_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_ecmp_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS, field_id))
    {
        int32 ret;
        p_rt_glb->current_ecmp_v6_groups = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES, field_id))
    {
        int32 ret;
        p_rt_glb->current_remote_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM, field_id))
    {
        int32 ret;
        p_rt_glb->current_indirect_ipv6_route_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL, field_id))
    {
        int32 ret;
        p_rt_glb->active_local = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC, field_id))
    {
        int32 ret;
        p_rt_glb->active_static = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED, field_id))
    {
        int32 ret;
        p_rt_glb->active_connected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP, field_id))
    {
        int32 ret;
        p_rt_glb->active_rip = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF, field_id))
    {
        int32 ret;
        p_rt_glb->active_ospf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP, field_id))
    {
        int32 ret;
        p_rt_glb->active_bgp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM, field_id))
    {
        int32 ret;
        p_rt_glb->arpinsp_logbuf_entrynum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM, field_id))
    {
        int32 ret;
        p_rt_glb->arpinsp_logbuf_curnum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM, field_id))
    {
        int32 ret;
        p_rt_glb->arpinsp_logbuf_lognum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC, field_id))
    {
        int32 ret;
        p_rt_glb->arpinsp_logbuf_logsec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG, field_id))
    {
        int32 ret;
        p_rt_glb->arpinsp_validate_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_FIB_FULL, field_id))
    {
        int32 ret;
        p_rt_glb->fib_full = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE, field_id))
    {
        int32 ret;
        p_rt_glb->ipv6_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT, field_id))
    {
        int32 ret;
        p_rt_glb->nd_hop_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED, field_id))
    {
        int32 ret;
        p_rt_glb->vty_connected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->vty_nonconn_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_MODE, field_id))
    {
        cdb_enum_str2val(glb_vty_mode_strs, GLB_VTY_MODE_MAX, str);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rt_glb->arp_aging_timer);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_rt_glb->arpratelimit_timer);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC, field_id))
    {
        cdb_sal_time_str2val(str, &p_rt_glb->arpratelimit_timer_start_time_sec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC, field_id))
    {
        cdb_sal_time_str2val(str, &p_rt_glb->arpratelimit_timer_start_time_msec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_rt_glb->buf);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_PKT_LEN, field_id))
    {
        int32 ret;
        p_rt_glb->pkt_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_COUNT, field_id))
    {
        int32 ret;
        p_rt_glb->nd_count = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_route_global_table_parser(char** array, char* key_value,tbl_route_global_t *p_rt_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_route_global_key_str2val(key_value, p_rt_glb));

    for(i=1; i<TBL_ROUTE_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_route_global_field_value_parser( array[j++], i, p_rt_glb));
    }

    return PM_E_NONE;
}

int32
tbl_route_global_dump_one(tbl_route_global_t *p_rt_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ROUTE_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN].name,
            p_rt_glb->gratuitous_arp_learn_en);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT].name,
            p_rt_glb->arp_pkt_rx_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT].name,
            p_rt_glb->arp_pkt_tx_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT].name,
            p_rt_glb->arp_pkt_discard_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARP_COUNT].name,
            p_rt_glb->arp_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM].name,
            p_rt_glb->multipath_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_MAX_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_MAX_STATIC].name,
            p_rt_glb->max_static);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT].name,
            p_rt_glb->icmp_error_ratelimit);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN].name,
            p_rt_glb->ip_redirects_global_en);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES].name,
            p_rt_glb->current_static_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES].name,
            p_rt_glb->current_ecmp_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS].name,
            p_rt_glb->current_ecmp_groups);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES].name,
            p_rt_glb->current_remote_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES].name,
            p_rt_glb->current_host_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM].name,
            p_rt_glb->current_indirect_ipv4_route_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES].name,
            p_rt_glb->current_static_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES].name,
            p_rt_glb->current_ecmp_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS].name,
            p_rt_glb->current_ecmp_v6_groups);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES].name,
            p_rt_glb->current_remote_v6_routes);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM].name,
            p_rt_glb->current_indirect_ipv6_route_num);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL].name,
            p_rt_glb->active_local);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC].name,
            p_rt_glb->active_static);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED].name,
            p_rt_glb->active_connected);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP].name,
            p_rt_glb->active_rip);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF].name,
            p_rt_glb->active_ospf);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP].name,
            p_rt_glb->active_bgp);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM].name,
            p_rt_glb->arpinsp_logbuf_entrynum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM].name,
            p_rt_glb->arpinsp_logbuf_curnum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM].name,
            p_rt_glb->arpinsp_logbuf_lognum);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC].name,
            p_rt_glb->arpinsp_logbuf_logsec);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG].name,
            p_rt_glb->arpinsp_validate_flag);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_FIB_FULL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_FIB_FULL].name,
            p_rt_glb->fib_full);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE].name,
            p_rt_glb->ipv6_enable);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT].name,
            p_rt_glb->nd_hop_limit);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED].name,
            p_rt_glb->vty_connected);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT].name,
            p_rt_glb->vty_nonconn_count);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_VTY_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_VTY_MODE].name, 
            cdb_enum_val2str(glb_vty_mode_strs, GLB_VTY_MODE_MAX, p_rt_glb->vty_mode));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rt_glb->arp_aging_timer));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_rt_glb->arpratelimit_timer));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_rt_glb->arpratelimit_timer_start_time_sec));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_rt_glb->arpratelimit_timer_start_time_msec));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_rt_glb->buf, sizeof(p_rt_glb->buf)));
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_PKT_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_PKT_LEN].name,
            p_rt_glb->pkt_len);
    }
    if (FLD_MATCH(TBL_ROUTE_GLOBAL_FLD_ND_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ROUTE_GLOBAL_FLD_ND_COUNT].name,
            p_rt_glb->nd_count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_route_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_route_global_t *p_db_rt_glb = _g_p_tbl_route_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_rt_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_rt_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_route_global_t*
tbl_route_global_init_route_global()
{
    _g_p_tbl_route_global = XCALLOC(MEM_TBL_ROUTE_GLOBAL, sizeof(tbl_route_global_t));
    return _g_p_tbl_route_global;
}


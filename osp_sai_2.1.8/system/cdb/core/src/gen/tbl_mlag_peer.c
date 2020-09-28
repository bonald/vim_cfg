
#include "proto.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "cdb_data_cmp.h"

tbl_mlag_peer_t *_g_p_tbl_mlag_peer = NULL;

int32
tbl_mlag_peer_set_mlag_peer_field_sync(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_field_id_t field_id, uint32 sync)
{
    tbl_mlag_peer_t *p_db_peer = _g_p_tbl_mlag_peer;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MLAG_PEER_FLD_PEER_ADDR:
        sal_memcpy(&p_db_peer->peer_addr, &p_peer->peer_addr, sizeof(p_peer->peer_addr));
        break;

    case TBL_MLAG_PEER_FLD_SET_TIMERS:
        p_db_peer->set_timers = p_peer->set_timers;
        break;

    case TBL_MLAG_PEER_FLD_HOLDTIME:
        p_db_peer->holdtime = p_peer->holdtime;
        break;

    case TBL_MLAG_PEER_FLD_KEEPALIVE:
        p_db_peer->keepalive = p_peer->keepalive;
        break;

    case TBL_MLAG_PEER_FLD_SERVER_SOCK:
        p_db_peer->server_sock = p_peer->server_sock;
        break;

    case TBL_MLAG_PEER_FLD_CLIENT_SOCK:
        p_db_peer->client_sock = p_peer->client_sock;
        break;

    case TBL_MLAG_PEER_FLD_V_AUTO_START:
        p_db_peer->v_auto_start = p_peer->v_auto_start;
        break;

    case TBL_MLAG_PEER_FLD_V_CONNECT:
        p_db_peer->v_connect = p_peer->v_connect;
        break;

    case TBL_MLAG_PEER_FLD_V_HOLDTIME:
        p_db_peer->v_holdtime = p_peer->v_holdtime;
        break;

    case TBL_MLAG_PEER_FLD_V_KEEPALIVE:
        p_db_peer->v_keepalive = p_peer->v_keepalive;
        break;

    case TBL_MLAG_PEER_FLD_OPEN_IN:
        p_db_peer->open_in = p_peer->open_in;
        break;

    case TBL_MLAG_PEER_FLD_OPEN_OUT:
        p_db_peer->open_out = p_peer->open_out;
        break;

    case TBL_MLAG_PEER_FLD_KEEPALIVE_IN:
        p_db_peer->keepalive_in = p_peer->keepalive_in;
        break;

    case TBL_MLAG_PEER_FLD_KEEPALIVE_OUT:
        p_db_peer->keepalive_out = p_peer->keepalive_out;
        break;

    case TBL_MLAG_PEER_FLD_DISCONNECT_IN:
        p_db_peer->disconnect_in = p_peer->disconnect_in;
        break;

    case TBL_MLAG_PEER_FLD_DISCONNECT_OUT:
        p_db_peer->disconnect_out = p_peer->disconnect_out;
        break;

    case TBL_MLAG_PEER_FLD_FDBSYNC_IN:
        p_db_peer->fdbsync_in = p_peer->fdbsync_in;
        break;

    case TBL_MLAG_PEER_FLD_FDBSYNC_OUT:
        p_db_peer->fdbsync_out = p_peer->fdbsync_out;
        break;

    case TBL_MLAG_PEER_FLD_FAILOVER_IN:
        p_db_peer->failover_in = p_peer->failover_in;
        break;

    case TBL_MLAG_PEER_FLD_FAILOVER_OUT:
        p_db_peer->failover_out = p_peer->failover_out;
        break;

    case TBL_MLAG_PEER_FLD_CONF_IN:
        p_db_peer->conf_in = p_peer->conf_in;
        break;

    case TBL_MLAG_PEER_FLD_CONF_OUT:
        p_db_peer->conf_out = p_peer->conf_out;
        break;

    case TBL_MLAG_PEER_FLD_SYSPRI_IN:
        p_db_peer->syspri_in = p_peer->syspri_in;
        break;

    case TBL_MLAG_PEER_FLD_SYSPRI_OUT:
        p_db_peer->syspri_out = p_peer->syspri_out;
        break;

    case TBL_MLAG_PEER_FLD_PEER_FDB_IN:
        p_db_peer->peer_fdb_in = p_peer->peer_fdb_in;
        break;

    case TBL_MLAG_PEER_FLD_PEER_FDB_OUT:
        p_db_peer->peer_fdb_out = p_peer->peer_fdb_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_IN:
        p_db_peer->stp_in = p_peer->stp_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_OUT:
        p_db_peer->stp_out = p_peer->stp_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_GLOBAL_IN:
        p_db_peer->stp_global_in = p_peer->stp_global_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT:
        p_db_peer->stp_global_out = p_peer->stp_global_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_PORT_IN:
        p_db_peer->stp_port_in = p_peer->stp_port_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_PORT_OUT:
        p_db_peer->stp_port_out = p_peer->stp_port_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_PACKET_IN:
        p_db_peer->stp_packet_in = p_peer->stp_packet_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_PACKET_OUT:
        p_db_peer->stp_packet_out = p_peer->stp_packet_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_INSTANCE_IN:
        p_db_peer->stp_instance_in = p_peer->stp_instance_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT:
        p_db_peer->stp_instance_out = p_peer->stp_instance_out;
        break;

    case TBL_MLAG_PEER_FLD_STP_STATE_IN:
        p_db_peer->stp_state_in = p_peer->stp_state_in;
        break;

    case TBL_MLAG_PEER_FLD_STP_STATE_OUT:
        p_db_peer->stp_state_out = p_peer->stp_state_out;
        break;

    case TBL_MLAG_PEER_FLD_ESTABLISHED:
        p_db_peer->established = p_peer->established;
        break;

    case TBL_MLAG_PEER_FLD_DROPPED:
        p_db_peer->dropped = p_peer->dropped;
        break;

    case TBL_MLAG_PEER_FLD_TTL:
        p_db_peer->ttl = p_peer->ttl;
        break;

    case TBL_MLAG_PEER_FLD_UPTIME:
        sal_memcpy(&p_db_peer->uptime, &p_peer->uptime, sizeof(p_peer->uptime));
        break;

    case TBL_MLAG_PEER_FLD_LASTREAD:
        sal_memcpy(&p_db_peer->lastread, &p_peer->lastread, sizeof(p_peer->lastread));
        break;

    case TBL_MLAG_PEER_FLD_MPF_STATE:
        p_db_peer->mpf_state = p_peer->mpf_state;
        break;

    case TBL_MLAG_PEER_FLD_T_AUTO_START:
        p_db_peer->t_auto_start = p_peer->t_auto_start;
        break;

    case TBL_MLAG_PEER_FLD_T_HOLDTIME:
        p_db_peer->t_holdtime = p_peer->t_holdtime;
        break;

    case TBL_MLAG_PEER_FLD_T_KEEPALIVE:
        p_db_peer->t_keepalive = p_peer->t_keepalive;
        break;

    case TBL_MLAG_PEER_FLD_MAX:
        sal_memcpy(p_db_peer, p_peer, sizeof(tbl_mlag_peer_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MLAG_PEER, field_id, p_db_peer);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MLAG_PEER, field_id, p_db_peer);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mlag_peer_set_mlag_peer_field(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_field_id_t field_id)
{
    return tbl_mlag_peer_set_mlag_peer_field_sync(p_peer, field_id, TRUE);
}

tbl_mlag_peer_t*
tbl_mlag_peer_get_mlag_peer()
{
    return _g_p_tbl_mlag_peer;
}

int32
tbl_mlag_peer_key_str2val(char *str, tbl_mlag_peer_t *p_peer)
{
    return PM_E_NONE;
}

char*
tbl_mlag_peer_key_name_dump(tbl_mlag_peer_t *p_peer, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PEER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_mlag_peer_key_value_dump(tbl_mlag_peer_t *p_peer, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_mlag_peer_field_name_dump(tbl_mlag_peer_t *p_peer, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PEER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MLAG_PEER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mlag_peer_field_value_dump(tbl_mlag_peer_t *p_peer, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_ADDR, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_peer->peer_addr);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SET_TIMERS, field_id))
    {
        sal_sprintf(str, "%u", p_peer->set_timers);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_HOLDTIME, field_id))
    {
        sal_sprintf(str, "%u", p_peer->holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE, field_id))
    {
        sal_sprintf(str, "%u", p_peer->keepalive);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SERVER_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_peer->server_sock);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CLIENT_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_peer->client_sock);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_AUTO_START, field_id))
    {
        sal_sprintf(str, "%u", p_peer->v_auto_start);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_CONNECT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->v_connect);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_HOLDTIME, field_id))
    {
        sal_sprintf(str, "%u", p_peer->v_holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_KEEPALIVE, field_id))
    {
        sal_sprintf(str, "%u", p_peer->v_keepalive);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->open_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->open_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->keepalive_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->keepalive_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->disconnect_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->disconnect_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->fdbsync_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->fdbsync_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->failover_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->failover_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->conf_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->conf_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->syspri_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->syspri_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->peer_fdb_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->peer_fdb_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_global_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_global_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_port_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_port_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_packet_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_packet_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_instance_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_instance_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_IN, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_state_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_peer->stp_state_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_ESTABLISHED, field_id))
    {
        sal_sprintf(str, "%u", p_peer->established);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DROPPED, field_id))
    {
        sal_sprintf(str, "%u", p_peer->dropped);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_peer->ttl);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_peer->uptime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_LASTREAD, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_peer->lastread);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_MPF_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_peer->mpf_state);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_AUTO_START, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_peer->t_auto_start);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_HOLDTIME, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_peer->t_holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_KEEPALIVE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_peer->t_keepalive);
    }
    return str;
}

char**
tbl_mlag_peer_table_dump(tbl_mlag_peer_t *p_peer, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mlag_peer_key_name_dump(p_peer, buf));
    for(i=1; i<TBL_MLAG_PEER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mlag_peer_field_name_dump(p_peer, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mlag_peer_field_value_dump(p_peer, i, buf));
    }
    return str;
}

int32
tbl_mlag_peer_field_value_parser(char *str, int32 field_id, tbl_mlag_peer_t *p_peer)
{
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_ADDR, field_id))
    {
        cdb_addr_str2val(&p_peer->peer_addr, str, 0);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SET_TIMERS, field_id))
    {
        int32 ret;
        p_peer->set_timers = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_HOLDTIME, field_id))
    {
        int32 ret;
        p_peer->holdtime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE, field_id))
    {
        int32 ret;
        p_peer->keepalive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SERVER_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CLIENT_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_AUTO_START, field_id))
    {
        int32 ret;
        p_peer->v_auto_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_CONNECT, field_id))
    {
        int32 ret;
        p_peer->v_connect = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_HOLDTIME, field_id))
    {
        int32 ret;
        p_peer->v_holdtime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_KEEPALIVE, field_id))
    {
        int32 ret;
        p_peer->v_keepalive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_IN, field_id))
    {
        int32 ret;
        p_peer->open_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_OUT, field_id))
    {
        int32 ret;
        p_peer->open_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_IN, field_id))
    {
        int32 ret;
        p_peer->keepalive_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_OUT, field_id))
    {
        int32 ret;
        p_peer->keepalive_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_IN, field_id))
    {
        int32 ret;
        p_peer->disconnect_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_OUT, field_id))
    {
        int32 ret;
        p_peer->disconnect_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_IN, field_id))
    {
        int32 ret;
        p_peer->fdbsync_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_OUT, field_id))
    {
        int32 ret;
        p_peer->fdbsync_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_IN, field_id))
    {
        int32 ret;
        p_peer->failover_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_OUT, field_id))
    {
        int32 ret;
        p_peer->failover_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_IN, field_id))
    {
        int32 ret;
        p_peer->conf_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_OUT, field_id))
    {
        int32 ret;
        p_peer->conf_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_IN, field_id))
    {
        int32 ret;
        p_peer->syspri_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_OUT, field_id))
    {
        int32 ret;
        p_peer->syspri_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_IN, field_id))
    {
        int32 ret;
        p_peer->peer_fdb_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_OUT, field_id))
    {
        int32 ret;
        p_peer->peer_fdb_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_IN, field_id))
    {
        int32 ret;
        p_peer->stp_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_IN, field_id))
    {
        int32 ret;
        p_peer->stp_global_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_global_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_IN, field_id))
    {
        int32 ret;
        p_peer->stp_port_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_port_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_IN, field_id))
    {
        int32 ret;
        p_peer->stp_packet_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_packet_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_IN, field_id))
    {
        int32 ret;
        p_peer->stp_instance_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_instance_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_IN, field_id))
    {
        int32 ret;
        p_peer->stp_state_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_OUT, field_id))
    {
        int32 ret;
        p_peer->stp_state_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_ESTABLISHED, field_id))
    {
        int32 ret;
        p_peer->established = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DROPPED, field_id))
    {
        int32 ret;
        p_peer->dropped = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_TTL, field_id))
    {
        int32 ret;
        p_peer->ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_peer->uptime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_LASTREAD, field_id))
    {
        cdb_sal_time_str2val(str, &p_peer->lastread);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_MPF_STATE, field_id))
    {
        int32 ret;
        p_peer->mpf_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_AUTO_START, field_id))
    {
        ctc_task_str2val(str, p_peer->t_auto_start);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_HOLDTIME, field_id))
    {
        ctc_task_str2val(str, p_peer->t_holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_KEEPALIVE, field_id))
    {
        ctc_task_str2val(str, p_peer->t_keepalive);
    }
    return PM_E_NONE;
}

int32
tbl_mlag_peer_table_parser(char** array, char* key_value,tbl_mlag_peer_t *p_peer)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PEER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mlag_peer_key_str2val(key_value, p_peer));

    for(i=1; i<TBL_MLAG_PEER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mlag_peer_field_value_parser( array[j++], i, p_peer));
    }

    return PM_E_NONE;
}

int32
tbl_mlag_peer_dump_one(tbl_mlag_peer_t *p_peer, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PEER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_PEER_ADDR].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_peer->peer_addr));
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SET_TIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_SET_TIMERS].name,
            p_peer->set_timers);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_HOLDTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_HOLDTIME].name,
            p_peer->holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_KEEPALIVE].name,
            p_peer->keepalive);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SERVER_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_MLAG_PEER_FLD_SERVER_SOCK].name,
            p_peer->server_sock);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CLIENT_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_MLAG_PEER_FLD_CLIENT_SOCK].name,
            p_peer->client_sock);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_AUTO_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_V_AUTO_START].name,
            p_peer->v_auto_start);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_CONNECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_V_CONNECT].name,
            p_peer->v_connect);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_HOLDTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_V_HOLDTIME].name,
            p_peer->v_holdtime);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_V_KEEPALIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_V_KEEPALIVE].name,
            p_peer->v_keepalive);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_OPEN_IN].name,
            p_peer->open_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_OPEN_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_OPEN_OUT].name,
            p_peer->open_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_KEEPALIVE_IN].name,
            p_peer->keepalive_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_KEEPALIVE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_KEEPALIVE_OUT].name,
            p_peer->keepalive_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_DISCONNECT_IN].name,
            p_peer->disconnect_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DISCONNECT_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_DISCONNECT_OUT].name,
            p_peer->disconnect_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_FDBSYNC_IN].name,
            p_peer->fdbsync_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FDBSYNC_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_FDBSYNC_OUT].name,
            p_peer->fdbsync_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_FAILOVER_IN].name,
            p_peer->failover_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_FAILOVER_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_FAILOVER_OUT].name,
            p_peer->failover_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_CONF_IN].name,
            p_peer->conf_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_CONF_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_CONF_OUT].name,
            p_peer->conf_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_SYSPRI_IN].name,
            p_peer->syspri_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_SYSPRI_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_SYSPRI_OUT].name,
            p_peer->syspri_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_PEER_FDB_IN].name,
            p_peer->peer_fdb_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_PEER_FDB_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_PEER_FDB_OUT].name,
            p_peer->peer_fdb_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_IN].name,
            p_peer->stp_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_OUT].name,
            p_peer->stp_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_GLOBAL_IN].name,
            p_peer->stp_global_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_GLOBAL_OUT].name,
            p_peer->stp_global_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_PORT_IN].name,
            p_peer->stp_port_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PORT_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_PORT_OUT].name,
            p_peer->stp_port_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_PACKET_IN].name,
            p_peer->stp_packet_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_PACKET_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_PACKET_OUT].name,
            p_peer->stp_packet_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_INSTANCE_IN].name,
            p_peer->stp_instance_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_INSTANCE_OUT].name,
            p_peer->stp_instance_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_STATE_IN].name,
            p_peer->stp_state_in);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_STP_STATE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_STP_STATE_OUT].name,
            p_peer->stp_state_out);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_ESTABLISHED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_ESTABLISHED].name,
            p_peer->established);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_DROPPED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_DROPPED].name,
            p_peer->dropped);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_TTL].name,
            p_peer->ttl);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_UPTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_UPTIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_peer->uptime));
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_LASTREAD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_LASTREAD].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_peer->lastread));
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_MPF_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PEER_FLD_MPF_STATE].name,
            p_peer->mpf_state);
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_AUTO_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_T_AUTO_START].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_peer->t_auto_start));
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_HOLDTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_T_HOLDTIME].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_peer->t_holdtime));
    }
    if (FLD_MATCH(TBL_MLAG_PEER_FLD_T_KEEPALIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MLAG_PEER_FLD_T_KEEPALIVE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_peer->t_keepalive));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mlag_peer_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mlag_peer_t *p_db_peer = _g_p_tbl_mlag_peer;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_peer)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_peer, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mlag_peer_t*
tbl_mlag_peer_init_mlag_peer()
{
    _g_p_tbl_mlag_peer = XCALLOC(MEM_TBL_MLAG_PEER, sizeof(tbl_mlag_peer_t));
    return _g_p_tbl_mlag_peer;
}


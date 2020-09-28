
#include "proto.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "cdb_data_cmp.h"

tbl_vrrp_session_master_t *_g_p_tbl_vrrp_session_master = NULL;

int32
tbl_vrrp_session_add_vrrp_session_sync(tbl_vrrp_session_t *p_vrrp_sess, uint32 sync)
{
    tbl_vrrp_session_master_t *p_master = _g_p_tbl_vrrp_session_master;
    tbl_vrrp_session_t *p_db_vrrp_sess = NULL;

    /* 1. lookup entry exist */
    if (tbl_vrrp_session_get_vrrp_session(&p_vrrp_sess->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vrrp_sess = XCALLOC(MEM_TBL_VRRP_SESSION, sizeof(tbl_vrrp_session_t));
    if (NULL == p_db_vrrp_sess)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vrrp_sess, p_vrrp_sess, sizeof(tbl_vrrp_session_t));

    /* 4. add to db */
    p_master->session_array[p_vrrp_sess->key.vrid] = p_db_vrrp_sess;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VRRP_SESSION, p_db_vrrp_sess);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VRRP_SESSION, p_db_vrrp_sess);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_session_del_vrrp_session_sync(tbl_vrrp_session_key_t *p_vrrp_sess_key, uint32 sync)
{
    tbl_vrrp_session_master_t *p_master = _g_p_tbl_vrrp_session_master;
    tbl_vrrp_session_t *p_db_vrrp_sess = NULL;

    /* 1. lookup entry exist */
    p_db_vrrp_sess = tbl_vrrp_session_get_vrrp_session(p_vrrp_sess_key);
    if (NULL == p_db_vrrp_sess)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VRRP_SESSION, p_db_vrrp_sess);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VRRP_SESSION, p_db_vrrp_sess);
        #endif 
    }

    /* 3. delete from db */
    p_master->session_array[p_vrrp_sess_key->vrid] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VRRP_SESSION, p_db_vrrp_sess);

    return PM_E_NONE;
}

int32
tbl_vrrp_session_set_vrrp_session_field_sync(tbl_vrrp_session_t *p_vrrp_sess, tbl_vrrp_session_field_id_t field_id, uint32 sync)
{
    tbl_vrrp_session_t *p_db_vrrp_sess = NULL;

    /* 1. lookup entry exist */
    p_db_vrrp_sess = tbl_vrrp_session_get_vrrp_session(&p_vrrp_sess->key);
    if (NULL == p_db_vrrp_sess)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VRRP_SESSION_FLD_ENABLE:
        p_db_vrrp_sess->enable = p_vrrp_sess->enable;
        break;

    case TBL_VRRP_SESSION_FLD_STATE:
        p_db_vrrp_sess->state = p_vrrp_sess->state;
        break;

    case TBL_VRRP_SESSION_FLD_VIP:
        sal_memcpy(&p_db_vrrp_sess->vip, &p_vrrp_sess->vip, sizeof(p_vrrp_sess->vip));
        break;

    case TBL_VRRP_SESSION_FLD_OWNER:
        p_db_vrrp_sess->owner = p_vrrp_sess->owner;
        break;

    case TBL_VRRP_SESSION_FLD_IFNAME:
        sal_memcpy(p_db_vrrp_sess->ifname, p_vrrp_sess->ifname, sizeof(p_vrrp_sess->ifname));
        break;

    case TBL_VRRP_SESSION_FLD_IFINDEX:
        p_db_vrrp_sess->ifindex = p_vrrp_sess->ifindex;
        break;

    case TBL_VRRP_SESSION_FLD_PRIORITY:
        p_db_vrrp_sess->priority = p_vrrp_sess->priority;
        break;

    case TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY:
        p_db_vrrp_sess->config_priority = p_vrrp_sess->config_priority;
        break;

    case TBL_VRRP_SESSION_FLD_ADVT_INTERVAL:
        p_db_vrrp_sess->advt_interval = p_vrrp_sess->advt_interval;
        break;

    case TBL_VRRP_SESSION_FLD_TIME_MODE:
        p_db_vrrp_sess->time_mode = p_vrrp_sess->time_mode;
        break;

    case TBL_VRRP_SESSION_FLD_PREEMPT_DELAY:
        p_db_vrrp_sess->preempt_delay = p_vrrp_sess->preempt_delay;
        break;

    case TBL_VRRP_SESSION_FLD_PREEMPT_MODE:
        p_db_vrrp_sess->preempt_mode = p_vrrp_sess->preempt_mode;
        break;

    case TBL_VRRP_SESSION_FLD_VMAC:
        sal_memcpy(p_db_vrrp_sess->vmac, p_vrrp_sess->vmac, sizeof(p_vrrp_sess->vmac));
        break;

    case TBL_VRRP_SESSION_FLD_IP_PROTOCOL:
        p_db_vrrp_sess->ip_protocol = p_vrrp_sess->ip_protocol;
        break;

    case TBL_VRRP_SESSION_FLD_TIMER:
        p_db_vrrp_sess->timer = p_vrrp_sess->timer;
        break;

    case TBL_VRRP_SESSION_FLD_OLD_MAC:
        sal_memcpy(p_db_vrrp_sess->old_mac, p_vrrp_sess->old_mac, sizeof(p_vrrp_sess->old_mac));
        break;

    case TBL_VRRP_SESSION_FLD_VIP_STATUS:
        p_db_vrrp_sess->vip_status = p_vrrp_sess->vip_status;
        break;

    case TBL_VRRP_SESSION_FLD_IFP_STATUS:
        p_db_vrrp_sess->ifp_status = p_vrrp_sess->ifp_status;
        break;

    case TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG:
        p_db_vrrp_sess->shutdown_flag = p_vrrp_sess->shutdown_flag;
        break;

    case TBL_VRRP_SESSION_FLD_SKEW_TIME:
        p_db_vrrp_sess->skew_time = p_vrrp_sess->skew_time;
        break;

    case TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL:
        p_db_vrrp_sess->master_down_interval = p_vrrp_sess->master_down_interval;
        break;

    case TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS:
        p_db_vrrp_sess->num_ip_addrs = p_vrrp_sess->num_ip_addrs;
        break;

    case TBL_VRRP_SESSION_FLD_TRACKOBJ_ID:
        p_db_vrrp_sess->trackobj_id = p_vrrp_sess->trackobj_id;
        break;

    case TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS:
        p_db_vrrp_sess->trackobj_status = p_vrrp_sess->trackobj_status;
        break;

    case TBL_VRRP_SESSION_FLD_PRIORITY_DELTA:
        p_db_vrrp_sess->priority_delta = p_vrrp_sess->priority_delta;
        break;

    case TBL_VRRP_SESSION_FLD_VRRP_UPTIME:
        sal_memcpy(&p_db_vrrp_sess->vrrp_uptime, &p_vrrp_sess->vrrp_uptime, sizeof(p_vrrp_sess->vrrp_uptime));
        break;

    case TBL_VRRP_SESSION_FLD_ADMIN_STATE:
        p_db_vrrp_sess->admin_state = p_vrrp_sess->admin_state;
        break;

    case TBL_VRRP_SESSION_FLD_ROWSTATUS:
        p_db_vrrp_sess->rowstatus = p_vrrp_sess->rowstatus;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER:
        p_db_vrrp_sess->stats_become_master = p_vrrp_sess->stats_become_master;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD:
        p_db_vrrp_sess->stats_advt_rcvd = p_vrrp_sess->stats_advt_rcvd;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS:
        p_db_vrrp_sess->stats_advt_interval_errors = p_vrrp_sess->stats_advt_interval_errors;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES:
        p_db_vrrp_sess->stats_auth_failures = p_vrrp_sess->stats_auth_failures;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS:
        p_db_vrrp_sess->stats_ip_ttl_errors = p_vrrp_sess->stats_ip_ttl_errors;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD:
        p_db_vrrp_sess->stats_priority_zero_pkts_rcvd = p_vrrp_sess->stats_priority_zero_pkts_rcvd;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT:
        p_db_vrrp_sess->stats_priority_zero_pkts_sent = p_vrrp_sess->stats_priority_zero_pkts_sent;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD:
        p_db_vrrp_sess->stats_invalid_type_pkts_rcvd = p_vrrp_sess->stats_invalid_type_pkts_rcvd;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS:
        p_db_vrrp_sess->stats_addr_list_errors = p_vrrp_sess->stats_addr_list_errors;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE:
        p_db_vrrp_sess->stats_invalid_auth_type = p_vrrp_sess->stats_invalid_auth_type;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH:
        p_db_vrrp_sess->stats_auth_type_mismatch = p_vrrp_sess->stats_auth_type_mismatch;
        break;

    case TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS:
        p_db_vrrp_sess->stats_pkt_len_errors = p_vrrp_sess->stats_pkt_len_errors;
        break;

    case TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS:
        p_db_vrrp_sess->master_ip_status = p_vrrp_sess->master_ip_status;
        break;

    case TBL_VRRP_SESSION_FLD_MASTER_IP:
        sal_memcpy(&p_db_vrrp_sess->master_ip, &p_vrrp_sess->master_ip, sizeof(p_vrrp_sess->master_ip));
        break;

    case TBL_VRRP_SESSION_FLD_MASTER_PRIORITY:
        p_db_vrrp_sess->master_priority = p_vrrp_sess->master_priority;
        break;

    case TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL:
        p_db_vrrp_sess->master_advt_interval = p_vrrp_sess->master_advt_interval;
        break;

    case TBL_VRRP_SESSION_FLD_PRIMARY_IP:
        sal_memcpy(&p_db_vrrp_sess->primary_ip, &p_vrrp_sess->primary_ip, sizeof(p_vrrp_sess->primary_ip));
        break;

    case TBL_VRRP_SESSION_FLD_LEARN_MASTER:
        p_db_vrrp_sess->learn_master = p_vrrp_sess->learn_master;
        break;

    case TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP:
        p_db_vrrp_sess->secondaryip_same_vip = p_vrrp_sess->secondaryip_same_vip;
        break;

    case TBL_VRRP_SESSION_FLD_MAX:
        sal_memcpy(p_db_vrrp_sess, p_vrrp_sess, sizeof(tbl_vrrp_session_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VRRP_SESSION, field_id, p_db_vrrp_sess);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VRRP_SESSION, field_id, p_db_vrrp_sess);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_session_add_vrrp_session(tbl_vrrp_session_t *p_vrrp_sess)
{
    return tbl_vrrp_session_add_vrrp_session_sync(p_vrrp_sess, TRUE);
}

int32
tbl_vrrp_session_del_vrrp_session(tbl_vrrp_session_key_t *p_vrrp_sess_key)
{
    return tbl_vrrp_session_del_vrrp_session_sync(p_vrrp_sess_key, TRUE);
}

int32
tbl_vrrp_session_set_vrrp_session_field(tbl_vrrp_session_t *p_vrrp_sess, tbl_vrrp_session_field_id_t field_id)
{
    return tbl_vrrp_session_set_vrrp_session_field_sync(p_vrrp_sess, field_id, TRUE);
}

tbl_vrrp_session_t*
tbl_vrrp_session_get_vrrp_session(tbl_vrrp_session_key_t *p_vrrp_sess_key)
{
    tbl_vrrp_session_master_t *p_master = _g_p_tbl_vrrp_session_master;

    if (GLB_VRRP_MAX_VRID+1 <= p_vrrp_sess_key->vrid)
    {
        return NULL;
    }
    return (p_master->session_array[p_vrrp_sess_key->vrid]);
}

char*
tbl_vrrp_session_key_val2str(tbl_vrrp_session_t *p_vrrp_sess, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vrrp_sess->key.vrid);
    return str;
}

int32
tbl_vrrp_session_key_str2val(char *str, tbl_vrrp_session_t *p_vrrp_sess)
{
    int32 ret = 0;

    p_vrrp_sess->key.vrid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vrrp_session_key_name_dump(tbl_vrrp_session_t *p_vrrp_sess, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_SESSION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vrrp_sess->key.vrid);
    return str;
}

char*
tbl_vrrp_session_key_value_dump(tbl_vrrp_session_t *p_vrrp_sess, char *str)
{
    sal_sprintf(str, "%u", p_vrrp_sess->key.vrid);
    return str;
}

char*
tbl_vrrp_session_field_name_dump(tbl_vrrp_session_t *p_vrrp_sess, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VRRP_SESSION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vrrp_session_field_value_dump(tbl_vrrp_session_t *p_vrrp_sess, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->key.vrid);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->enable);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->state);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_sess->vip);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OWNER, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->owner);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_vrrp_sess->ifname);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->ifindex);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->config_priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADVT_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->advt_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIME_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->time_mode);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_DELAY, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->preempt_delay);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->preempt_mode);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VMAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_vrrp_sess->vmac);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->ip_protocol);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIMER, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->timer);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OLD_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_vrrp_sess->old_mac);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->vip_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFP_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->ifp_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->shutdown_flag);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SKEW_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->skew_time);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->master_down_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->num_ip_addrs);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_ID, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->trackobj_id);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->trackobj_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY_DELTA, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->priority_delta);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VRRP_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_sess->vrrp_uptime);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADMIN_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->admin_state);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ROWSTATUS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->rowstatus);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_become_master);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_advt_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_advt_interval_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_auth_failures);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_ip_ttl_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_priority_zero_pkts_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_priority_zero_pkts_sent);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_invalid_type_pkts_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_addr_list_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_invalid_auth_type);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_auth_type_mismatch);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->stats_pkt_len_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->master_ip_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_sess->master_ip);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->master_priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->master_advt_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIMARY_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_sess->primary_ip);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_LEARN_MASTER, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->learn_master);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_sess->secondaryip_same_vip);
    }
    return str;
}

char**
tbl_vrrp_session_table_dump(tbl_vrrp_session_t *p_vrrp_sess, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vrrp_session_key_name_dump(p_vrrp_sess, buf));
    for(i=1; i<TBL_VRRP_SESSION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_session_field_name_dump(p_vrrp_sess, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_session_field_value_dump(p_vrrp_sess, i, buf));
    }
    return str;
}

int32
tbl_vrrp_session_field_value_parser(char *str, int32 field_id, tbl_vrrp_session_t *p_vrrp_sess)
{
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_KEY, field_id))
    {
        int32 ret;
        p_vrrp_sess->key.vrid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_vrrp_sess->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATE, field_id))
    {
        int32 ret;
        p_vrrp_sess->state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP, field_id))
    {
        cdb_addr_str2val(&p_vrrp_sess->vip, str, 0);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OWNER, field_id))
    {
        int32 ret;
        p_vrrp_sess->owner = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_vrrp_sess->ifname, str);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_vrrp_sess->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_vrrp_sess->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY, field_id))
    {
        int32 ret;
        p_vrrp_sess->config_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADVT_INTERVAL, field_id))
    {
        int32 ret;
        p_vrrp_sess->advt_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIME_MODE, field_id))
    {
        int32 ret;
        p_vrrp_sess->time_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_DELAY, field_id))
    {
        int32 ret;
        p_vrrp_sess->preempt_delay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_MODE, field_id))
    {
        int32 ret;
        p_vrrp_sess->preempt_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VMAC, field_id))
    {
        cdb_mac_addr_str2val(p_vrrp_sess->vmac, str);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IP_PROTOCOL, field_id))
    {
        int32 ret;
        p_vrrp_sess->ip_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIMER, field_id))
    {
        int32 ret;
        p_vrrp_sess->timer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OLD_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_vrrp_sess->old_mac, str);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP_STATUS, field_id))
    {
        int32 ret;
        p_vrrp_sess->vip_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFP_STATUS, field_id))
    {
        int32 ret;
        p_vrrp_sess->ifp_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG, field_id))
    {
        int32 ret;
        p_vrrp_sess->shutdown_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SKEW_TIME, field_id))
    {
        int32 ret;
        p_vrrp_sess->skew_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL, field_id))
    {
        int32 ret;
        p_vrrp_sess->master_down_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS, field_id))
    {
        int32 ret;
        p_vrrp_sess->num_ip_addrs = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_ID, field_id))
    {
        int32 ret;
        p_vrrp_sess->trackobj_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS, field_id))
    {
        int32 ret;
        p_vrrp_sess->trackobj_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY_DELTA, field_id))
    {
        int32 ret;
        p_vrrp_sess->priority_delta = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VRRP_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_vrrp_sess->vrrp_uptime);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADMIN_STATE, field_id))
    {
        int32 ret;
        p_vrrp_sess->admin_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ROWSTATUS, field_id))
    {
        int32 ret;
        p_vrrp_sess->rowstatus = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_become_master = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_advt_rcvd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_advt_interval_errors = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_auth_failures = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_ip_ttl_errors = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_priority_zero_pkts_rcvd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_priority_zero_pkts_sent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_invalid_type_pkts_rcvd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_addr_list_errors = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_invalid_auth_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_auth_type_mismatch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS, field_id))
    {
        int32 ret;
        p_vrrp_sess->stats_pkt_len_errors = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS, field_id))
    {
        int32 ret;
        p_vrrp_sess->master_ip_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP, field_id))
    {
        cdb_addr_str2val(&p_vrrp_sess->master_ip, str, 0);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_PRIORITY, field_id))
    {
        int32 ret;
        p_vrrp_sess->master_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL, field_id))
    {
        int32 ret;
        p_vrrp_sess->master_advt_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIMARY_IP, field_id))
    {
        cdb_addr_str2val(&p_vrrp_sess->primary_ip, str, 0);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_LEARN_MASTER, field_id))
    {
        int32 ret;
        p_vrrp_sess->learn_master = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP, field_id))
    {
        int32 ret;
        p_vrrp_sess->secondaryip_same_vip = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vrrp_session_table_parser(char** array, char* key_value,tbl_vrrp_session_t *p_vrrp_sess)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vrrp_session_key_str2val(key_value, p_vrrp_sess));

    for(i=1; i<TBL_VRRP_SESSION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vrrp_session_field_value_parser( array[j++], i, p_vrrp_sess));
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_session_dump_one(tbl_vrrp_session_t *p_vrrp_sess, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_SESSION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_KEY].name,
            p_vrrp_sess->key.vrid);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_ENABLE].name,
            p_vrrp_sess->enable);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATE].name,
            p_vrrp_sess->state);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_VIP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_sess->vip));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_OWNER].name,
            p_vrrp_sess->owner);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_IFNAME].name,
            p_vrrp_sess->ifname);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_IFINDEX].name,
            p_vrrp_sess->ifindex);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_PRIORITY].name,
            p_vrrp_sess->priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_CONFIG_PRIORITY].name,
            p_vrrp_sess->config_priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADVT_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_ADVT_INTERVAL].name,
            p_vrrp_sess->advt_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIME_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_TIME_MODE].name,
            p_vrrp_sess->time_mode);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_PREEMPT_DELAY].name,
            p_vrrp_sess->preempt_delay);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PREEMPT_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_PREEMPT_MODE].name,
            p_vrrp_sess->preempt_mode);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_VMAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_sess->vmac));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_IP_PROTOCOL].name,
            p_vrrp_sess->ip_protocol);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_TIMER].name,
            p_vrrp_sess->timer);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_OLD_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_OLD_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_sess->old_mac));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VIP_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_VIP_STATUS].name,
            p_vrrp_sess->vip_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_IFP_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_IFP_STATUS].name,
            p_vrrp_sess->ifp_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_SHUTDOWN_FLAG].name,
            p_vrrp_sess->shutdown_flag);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SKEW_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_SKEW_TIME].name,
            p_vrrp_sess->skew_time);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_MASTER_DOWN_INTERVAL].name,
            p_vrrp_sess->master_down_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_NUM_IP_ADDRS].name,
            p_vrrp_sess->num_ip_addrs);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_TRACKOBJ_ID].name,
            p_vrrp_sess->trackobj_id);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_TRACKOBJ_STATUS].name,
            p_vrrp_sess->trackobj_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIORITY_DELTA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_PRIORITY_DELTA].name,
            p_vrrp_sess->priority_delta);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_VRRP_UPTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_VRRP_UPTIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_sess->vrrp_uptime));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ADMIN_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_ADMIN_STATE].name,
            p_vrrp_sess->admin_state);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_ROWSTATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_ROWSTATUS].name,
            p_vrrp_sess->rowstatus);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_BECOME_MASTER].name,
            p_vrrp_sess->stats_become_master);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_ADVT_RCVD].name,
            p_vrrp_sess->stats_advt_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_ADVT_INTERVAL_ERRORS].name,
            p_vrrp_sess->stats_advt_interval_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_AUTH_FAILURES].name,
            p_vrrp_sess->stats_auth_failures);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_IP_TTL_ERRORS].name,
            p_vrrp_sess->stats_ip_ttl_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_RCVD].name,
            p_vrrp_sess->stats_priority_zero_pkts_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_PRIORITY_ZERO_PKTS_SENT].name,
            p_vrrp_sess->stats_priority_zero_pkts_sent);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_INVALID_TYPE_PKTS_RCVD].name,
            p_vrrp_sess->stats_invalid_type_pkts_rcvd);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_ADDR_LIST_ERRORS].name,
            p_vrrp_sess->stats_addr_list_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_INVALID_AUTH_TYPE].name,
            p_vrrp_sess->stats_invalid_auth_type);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_AUTH_TYPE_MISMATCH].name,
            p_vrrp_sess->stats_auth_type_mismatch);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_STATS_PKT_LEN_ERRORS].name,
            p_vrrp_sess->stats_pkt_len_errors);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_MASTER_IP_STATUS].name,
            p_vrrp_sess->master_ip_status);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_MASTER_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_sess->master_ip));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_MASTER_PRIORITY].name,
            p_vrrp_sess->master_priority);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_MASTER_ADVT_INTERVAL].name,
            p_vrrp_sess->master_advt_interval);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_PRIMARY_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_SESSION_FLD_PRIMARY_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_sess->primary_ip));
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_LEARN_MASTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_LEARN_MASTER].name,
            p_vrrp_sess->learn_master);
    }
    if (FLD_MATCH(TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_SESSION_FLD_SECONDARYIP_SAME_VIP].name,
            p_vrrp_sess->secondaryip_same_vip);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vrrp_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vrrp_session_master_t *p_master = _g_p_tbl_vrrp_session_master;
    tbl_vrrp_session_t *p_db_vrrp_sess = NULL;
    uint32 vrid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vrid = 0; vrid < GLB_VRRP_MAX_VRID+1; vrid++)
    {
        p_db_vrrp_sess = p_master->session_array[vrid];
        if (NULL == p_db_vrrp_sess)
        {
            continue;
        }
        rc |= fn(p_db_vrrp_sess, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vrrp_session_master_t*
tbl_vrrp_session_get_master()
{
    return _g_p_tbl_vrrp_session_master;
}

tbl_vrrp_session_master_t*
tbl_vrrp_session_init_vrrp_session()
{
    _g_p_tbl_vrrp_session_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vrrp_session_master_t));
    return _g_p_tbl_vrrp_session_master;
}


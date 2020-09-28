
#include "proto.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "cdb_data_cmp.h"

tbl_ipsla_entry_master_t *_g_p_tbl_ipsla_entry_master = NULL;

int32
tbl_ipsla_entry_add_ipsla_entry_sync(tbl_ipsla_entry_t *p_ipsla_entry, uint32 sync)
{
    tbl_ipsla_entry_master_t *p_master = _g_p_tbl_ipsla_entry_master;
    tbl_ipsla_entry_t *p_db_ipsla_entry = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipsla_entry_get_ipsla_entry(&p_ipsla_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipsla_entry = XCALLOC(MEM_TBL_IPSLA_ENTRY, sizeof(tbl_ipsla_entry_t));
    if (NULL == p_db_ipsla_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipsla_entry, p_ipsla_entry, sizeof(tbl_ipsla_entry_t));

    /* 4. add to db */
    p_master->ipslaentry_array[p_ipsla_entry->key.entry_id] = p_db_ipsla_entry;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPSLA_ENTRY, p_db_ipsla_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPSLA_ENTRY, p_db_ipsla_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_entry_del_ipsla_entry_sync(tbl_ipsla_entry_key_t *p_ipsla_entry_key, uint32 sync)
{
    tbl_ipsla_entry_master_t *p_master = _g_p_tbl_ipsla_entry_master;
    tbl_ipsla_entry_t *p_db_ipsla_entry = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_entry = tbl_ipsla_entry_get_ipsla_entry(p_ipsla_entry_key);
    if (NULL == p_db_ipsla_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPSLA_ENTRY, p_db_ipsla_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPSLA_ENTRY, p_db_ipsla_entry);
        #endif 
    }

    /* 3. delete from db */
    p_master->ipslaentry_array[p_ipsla_entry_key->entry_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_IPSLA_ENTRY, p_db_ipsla_entry);

    return PM_E_NONE;
}

int32
tbl_ipsla_entry_set_ipsla_entry_field_sync(tbl_ipsla_entry_t *p_ipsla_entry, tbl_ipsla_entry_field_id_t field_id, uint32 sync)
{
    tbl_ipsla_entry_t *p_db_ipsla_entry = NULL;

    /* 1. lookup entry exist */
    p_db_ipsla_entry = tbl_ipsla_entry_get_ipsla_entry(&p_ipsla_entry->key);
    if (NULL == p_db_ipsla_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSLA_ENTRY_FLD_TYPE:
        p_db_ipsla_entry->type = p_ipsla_entry->type;
        break;

    case TBL_IPSLA_ENTRY_FLD_ADMIN_STATE:
        p_db_ipsla_entry->admin_state = p_ipsla_entry->admin_state;
        break;

    case TBL_IPSLA_ENTRY_FLD_DESC:
        sal_memcpy(p_db_ipsla_entry->desc, p_ipsla_entry->desc, sizeof(p_ipsla_entry->desc));
        break;

    case TBL_IPSLA_ENTRY_FLD_FREQUENCY:
        p_db_ipsla_entry->frequency = p_ipsla_entry->frequency;
        break;

    case TBL_IPSLA_ENTRY_FLD_TIMEOUT:
        p_db_ipsla_entry->timeout = p_ipsla_entry->timeout;
        break;

    case TBL_IPSLA_ENTRY_FLD_THRESHOLD:
        p_db_ipsla_entry->threshold = p_ipsla_entry->threshold;
        break;

    case TBL_IPSLA_ENTRY_FLD_INTERVAL:
        p_db_ipsla_entry->interval = p_ipsla_entry->interval;
        break;

    case TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST:
        p_db_ipsla_entry->packets_per_test = p_ipsla_entry->packets_per_test;
        break;

    case TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT:
        p_db_ipsla_entry->fail_percent = p_ipsla_entry->fail_percent;
        break;

    case TBL_IPSLA_ENTRY_FLD_TTL:
        p_db_ipsla_entry->ttl = p_ipsla_entry->ttl;
        break;

    case TBL_IPSLA_ENTRY_FLD_TOS:
        p_db_ipsla_entry->tos = p_ipsla_entry->tos;
        break;

    case TBL_IPSLA_ENTRY_FLD_DATA_SIZE:
        p_db_ipsla_entry->data_size = p_ipsla_entry->data_size;
        break;

    case TBL_IPSLA_ENTRY_FLD_DATA_PATTERN:
        p_db_ipsla_entry->data_pattern = p_ipsla_entry->data_pattern;
        break;

    case TBL_IPSLA_ENTRY_FLD_INADDR:
        sal_memcpy(&p_db_ipsla_entry->inaddr, &p_ipsla_entry->inaddr, sizeof(p_ipsla_entry->inaddr));
        break;

    case TBL_IPSLA_ENTRY_FLD_SRC_INADDR:
        sal_memcpy(&p_db_ipsla_entry->src_inaddr, &p_ipsla_entry->src_inaddr, sizeof(p_ipsla_entry->src_inaddr));
        break;

    case TBL_IPSLA_ENTRY_FLD_SRC_IFNAME:
        sal_memcpy(p_db_ipsla_entry->src_ifname, p_ipsla_entry->src_ifname, sizeof(p_ipsla_entry->src_ifname));
        break;

    case TBL_IPSLA_ENTRY_FLD_SOCK:
        p_db_ipsla_entry->sock = p_ipsla_entry->sock;
        break;

    case TBL_IPSLA_ENTRY_FLD_SEND_SOCK:
        p_db_ipsla_entry->send_sock = p_ipsla_entry->send_sock;
        break;

    case TBL_IPSLA_ENTRY_FLD_SOCK_FLAG:
        p_db_ipsla_entry->sock_flag = p_ipsla_entry->sock_flag;
        break;

    case TBL_IPSLA_ENTRY_FLD_BUF:
        sal_memcpy(p_db_ipsla_entry->buf, p_ipsla_entry->buf, sizeof(p_ipsla_entry->buf));
        break;

    case TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH:
        p_db_ipsla_entry->packet_length = p_ipsla_entry->packet_length;
        break;

    case TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ:
        p_db_ipsla_entry->t_ipsla_read = p_ipsla_entry->t_ipsla_read;
        break;

    case TBL_IPSLA_ENTRY_FLD_TESTID:
        p_db_ipsla_entry->testid = p_ipsla_entry->testid;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY:
        p_db_ipsla_entry->running_frequency = p_ipsla_entry->running_frequency;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT:
        p_db_ipsla_entry->running_timeout = p_ipsla_entry->running_timeout;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL:
        p_db_ipsla_entry->running_interval = p_ipsla_entry->running_interval;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT:
        p_db_ipsla_entry->running_probe_cnt = p_ipsla_entry->running_probe_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_SEQUENCE:
        p_db_ipsla_entry->sequence = p_ipsla_entry->sequence;
        break;

    case TBL_IPSLA_ENTRY_FLD_RTT:
        p_db_ipsla_entry->rtt = p_ipsla_entry->rtt;
        break;

    case TBL_IPSLA_ENTRY_FLD_SEND_TIME:
        p_db_ipsla_entry->send_time = p_ipsla_entry->send_time;
        break;

    case TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME:
        p_db_ipsla_entry->last_send_time = p_ipsla_entry->last_send_time;
        break;

    case TBL_IPSLA_ENTRY_FLD_SEND_CNT:
        p_db_ipsla_entry->send_cnt = p_ipsla_entry->send_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT:
        p_db_ipsla_entry->receive_cnt = p_ipsla_entry->receive_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT:
        p_db_ipsla_entry->overthreshold_cnt = p_ipsla_entry->overthreshold_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT:
        p_db_ipsla_entry->sendfail_cnt = p_ipsla_entry->sendfail_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN:
        p_db_ipsla_entry->running_rtt_min = p_ipsla_entry->running_rtt_min;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX:
        p_db_ipsla_entry->running_rtt_max = p_ipsla_entry->running_rtt_max;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM:
        p_db_ipsla_entry->running_rtt_sum = p_ipsla_entry->running_rtt_sum;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS:
        p_db_ipsla_entry->running_success = p_ipsla_entry->running_success;
        break;

    case TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT:
        p_db_ipsla_entry->running_fail_percent = p_ipsla_entry->running_fail_percent;
        break;

    case TBL_IPSLA_ENTRY_FLD_RETURNCODE:
        p_db_ipsla_entry->returncode = p_ipsla_entry->returncode;
        break;

    case TBL_IPSLA_ENTRY_FLD_ECHO_REPLY:
        p_db_ipsla_entry->echo_reply = p_ipsla_entry->echo_reply;
        break;

    case TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE:
        p_db_ipsla_entry->total_receive = p_ipsla_entry->total_receive;
        break;

    case TBL_IPSLA_ENTRY_FLD_TOTAL_SEND:
        p_db_ipsla_entry->total_send = p_ipsla_entry->total_send;
        break;

    case TBL_IPSLA_ENTRY_FLD_RTT_SUM:
        p_db_ipsla_entry->rtt_sum = p_ipsla_entry->rtt_sum;
        break;

    case TBL_IPSLA_ENTRY_FLD_RTT_MAX:
        p_db_ipsla_entry->rtt_max = p_ipsla_entry->rtt_max;
        break;

    case TBL_IPSLA_ENTRY_FLD_RTT_MIN:
        p_db_ipsla_entry->rtt_min = p_ipsla_entry->rtt_min;
        break;

    case TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT:
        p_db_ipsla_entry->probe_result_config_cnt = p_ipsla_entry->probe_result_config_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX:
        p_db_ipsla_entry->probe_result_index = p_ipsla_entry->probe_result_index;
        break;

    case TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT:
        p_db_ipsla_entry->test_result_config_cnt = p_ipsla_entry->test_result_config_cnt;
        break;

    case TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX:
        p_db_ipsla_entry->test_result_index = p_ipsla_entry->test_result_index;
        break;

    case TBL_IPSLA_ENTRY_FLD_MAX:
        sal_memcpy(p_db_ipsla_entry, p_ipsla_entry, sizeof(tbl_ipsla_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSLA_ENTRY, field_id, p_db_ipsla_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPSLA_ENTRY, field_id, p_db_ipsla_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_entry_add_ipsla_entry(tbl_ipsla_entry_t *p_ipsla_entry)
{
    return tbl_ipsla_entry_add_ipsla_entry_sync(p_ipsla_entry, TRUE);
}

int32
tbl_ipsla_entry_del_ipsla_entry(tbl_ipsla_entry_key_t *p_ipsla_entry_key)
{
    return tbl_ipsla_entry_del_ipsla_entry_sync(p_ipsla_entry_key, TRUE);
}

int32
tbl_ipsla_entry_set_ipsla_entry_field(tbl_ipsla_entry_t *p_ipsla_entry, tbl_ipsla_entry_field_id_t field_id)
{
    return tbl_ipsla_entry_set_ipsla_entry_field_sync(p_ipsla_entry, field_id, TRUE);
}

tbl_ipsla_entry_t*
tbl_ipsla_entry_get_ipsla_entry(tbl_ipsla_entry_key_t *p_ipsla_entry_key)
{
    tbl_ipsla_entry_master_t *p_master = _g_p_tbl_ipsla_entry_master;

    if (GLB_IPSLA_ENTRY_MAX+1 <= p_ipsla_entry_key->entry_id)
    {
        return NULL;
    }
    return (p_master->ipslaentry_array[p_ipsla_entry_key->entry_id]);
}

char*
tbl_ipsla_entry_key_val2str(tbl_ipsla_entry_t *p_ipsla_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ipsla_entry->key.entry_id);
    return str;
}

int32
tbl_ipsla_entry_key_str2val(char *str, tbl_ipsla_entry_t *p_ipsla_entry)
{
    int32 ret = 0;

    p_ipsla_entry->key.entry_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ipsla_entry_key_name_dump(tbl_ipsla_entry_t *p_ipsla_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_ENTRY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ipsla_entry->key.entry_id);
    return str;
}

char*
tbl_ipsla_entry_key_value_dump(tbl_ipsla_entry_t *p_ipsla_entry, char *str)
{
    sal_sprintf(str, "%u", p_ipsla_entry->key.entry_id);
    return str;
}

char*
tbl_ipsla_entry_field_name_dump(tbl_ipsla_entry_t *p_ipsla_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPSLA_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipsla_entry_field_value_dump(tbl_ipsla_entry_t *p_ipsla_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->key.entry_id);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->type);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ADMIN_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->admin_state);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_ipsla_entry->desc);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FREQUENCY, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->frequency);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->threshold);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->interval);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->packets_per_test);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->ttl);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOS, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->tos);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->data_size);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_PATTERN, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->data_pattern);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_entry->inaddr);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_INADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_ipsla_entry->src_inaddr);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_ipsla_entry->src_ifname);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_ipsla_entry->sock);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_ipsla_entry->send_sock);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK_FLAG, field_id))
    {
        sal_sprintf(str, "%d", p_ipsla_entry->sock_flag);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_ipsla_entry->buf, sizeof(p_ipsla_entry->buf));
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->packet_length);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ipsla_entry->t_ipsla_read);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TESTID, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_frequency);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_interval);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_probe_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEQUENCE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->sequence);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->rtt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_ipsla_entry->send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_ipsla_entry->last_send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->send_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->receive_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->overthreshold_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->sendfail_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_success);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->running_fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RETURNCODE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->returncode);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ECHO_REPLY, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->echo_reply);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->total_receive);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_SEND, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->total_send);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_SUM, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_ipsla_entry->rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->probe_result_config_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->probe_result_index);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->test_result_config_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_entry->test_result_index);
    }
    return str;
}

char**
tbl_ipsla_entry_table_dump(tbl_ipsla_entry_t *p_ipsla_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipsla_entry_key_name_dump(p_ipsla_entry, buf));
    for(i=1; i<TBL_IPSLA_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_entry_field_name_dump(p_ipsla_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_entry_field_value_dump(p_ipsla_entry, i, buf));
    }
    return str;
}

int32
tbl_ipsla_entry_field_value_parser(char *str, int32 field_id, tbl_ipsla_entry_t *p_ipsla_entry)
{
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_KEY, field_id))
    {
        int32 ret;
        p_ipsla_entry->key.entry_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TYPE, field_id))
    {
        int32 ret;
        p_ipsla_entry->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ADMIN_STATE, field_id))
    {
        int32 ret;
        p_ipsla_entry->admin_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DESC, field_id))
    {
        sal_strcpy(p_ipsla_entry->desc, str);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FREQUENCY, field_id))
    {
        int32 ret;
        p_ipsla_entry->frequency = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_ipsla_entry->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_THRESHOLD, field_id))
    {
        int32 ret;
        p_ipsla_entry->threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INTERVAL, field_id))
    {
        int32 ret;
        p_ipsla_entry->interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST, field_id))
    {
        int32 ret;
        p_ipsla_entry->packets_per_test = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT, field_id))
    {
        int32 ret;
        p_ipsla_entry->fail_percent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TTL, field_id))
    {
        int32 ret;
        p_ipsla_entry->ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOS, field_id))
    {
        int32 ret;
        p_ipsla_entry->tos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_SIZE, field_id))
    {
        int32 ret;
        p_ipsla_entry->data_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_PATTERN, field_id))
    {
        int32 ret;
        p_ipsla_entry->data_pattern = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_ipsla_entry->inaddr, str, 0);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_INADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_ipsla_entry->src_inaddr, str, 0);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_IFNAME, field_id))
    {
        sal_strcpy(p_ipsla_entry->src_ifname, str);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK_FLAG, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_ipsla_entry->buf);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH, field_id))
    {
        int32 ret;
        p_ipsla_entry->packet_length = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ, field_id))
    {
        ctc_task_str2val(str, p_ipsla_entry->t_ipsla_read);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TESTID, field_id))
    {
        int32 ret;
        p_ipsla_entry->testid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_frequency = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_probe_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEQUENCE, field_id))
    {
        int32 ret;
        p_ipsla_entry->sequence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT, field_id))
    {
        int32 ret;
        p_ipsla_entry->rtt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_TIME, field_id))
    {
        int32 ret;
        p_ipsla_entry->send_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME, field_id))
    {
        int32 ret;
        p_ipsla_entry->last_send_time = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->send_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->receive_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->overthreshold_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->sendfail_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_rtt_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_rtt_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_rtt_sum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_success = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT, field_id))
    {
        int32 ret;
        p_ipsla_entry->running_fail_percent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RETURNCODE, field_id))
    {
        int32 ret;
        p_ipsla_entry->returncode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ECHO_REPLY, field_id))
    {
        int32 ret;
        p_ipsla_entry->echo_reply = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE, field_id))
    {
        int32 ret;
        p_ipsla_entry->total_receive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_SEND, field_id))
    {
        int32 ret;
        p_ipsla_entry->total_send = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_SUM, field_id))
    {
        int32 ret;
        p_ipsla_entry->rtt_sum = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MAX, field_id))
    {
        int32 ret;
        p_ipsla_entry->rtt_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MIN, field_id))
    {
        int32 ret;
        p_ipsla_entry->rtt_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->probe_result_config_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX, field_id))
    {
        int32 ret;
        p_ipsla_entry->probe_result_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT, field_id))
    {
        int32 ret;
        p_ipsla_entry->test_result_config_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX, field_id))
    {
        int32 ret;
        p_ipsla_entry->test_result_index = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipsla_entry_table_parser(char** array, char* key_value,tbl_ipsla_entry_t *p_ipsla_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipsla_entry_key_str2val(key_value, p_ipsla_entry));

    for(i=1; i<TBL_IPSLA_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipsla_entry_field_value_parser( array[j++], i, p_ipsla_entry));
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_entry_dump_one(tbl_ipsla_entry_t *p_ipsla_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_KEY].name,
            p_ipsla_entry->key.entry_id);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TYPE].name,
            p_ipsla_entry->type);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ADMIN_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_ADMIN_STATE].name,
            p_ipsla_entry->admin_state);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DESC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_DESC].name,
            p_ipsla_entry->desc);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FREQUENCY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_FREQUENCY].name,
            p_ipsla_entry->frequency);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TIMEOUT].name,
            p_ipsla_entry->timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_THRESHOLD].name,
            p_ipsla_entry->threshold);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_INTERVAL].name,
            p_ipsla_entry->interval);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST].name,
            p_ipsla_entry->packets_per_test);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT].name,
            p_ipsla_entry->fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TTL].name,
            p_ipsla_entry->ttl);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TOS].name,
            p_ipsla_entry->tos);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_DATA_SIZE].name,
            p_ipsla_entry->data_size);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_DATA_PATTERN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_DATA_PATTERN].name,
            p_ipsla_entry->data_pattern);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_INADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_INADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_entry->inaddr));
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_INADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SRC_INADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_ipsla_entry->src_inaddr));
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SRC_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SRC_IFNAME].name,
            p_ipsla_entry->src_ifname);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SOCK].name,
            p_ipsla_entry->sock);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SEND_SOCK].name,
            p_ipsla_entry->send_sock);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SOCK_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SOCK_FLAG].name,
            p_ipsla_entry->sock_flag);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_ipsla_entry->buf, sizeof(p_ipsla_entry->buf)));
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH].name,
            p_ipsla_entry->packet_length);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ipsla_entry->t_ipsla_read));
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TESTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TESTID].name,
            p_ipsla_entry->testid);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY].name,
            p_ipsla_entry->running_frequency);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT].name,
            p_ipsla_entry->running_timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL].name,
            p_ipsla_entry->running_interval);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT].name,
            p_ipsla_entry->running_probe_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEQUENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SEQUENCE].name,
            p_ipsla_entry->sequence);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RTT].name,
            p_ipsla_entry->rtt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SEND_TIME].name,
            p_ipsla_entry->send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME].name,
            p_ipsla_entry->last_send_time);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SEND_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SEND_CNT].name,
            p_ipsla_entry->send_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT].name,
            p_ipsla_entry->receive_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT].name,
            p_ipsla_entry->overthreshold_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT].name,
            p_ipsla_entry->sendfail_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN].name,
            p_ipsla_entry->running_rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX].name,
            p_ipsla_entry->running_rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM].name,
            p_ipsla_entry->running_rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS].name,
            p_ipsla_entry->running_success);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT].name,
            p_ipsla_entry->running_fail_percent);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RETURNCODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RETURNCODE].name,
            p_ipsla_entry->returncode);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_ECHO_REPLY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_ECHO_REPLY].name,
            p_ipsla_entry->echo_reply);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE].name,
            p_ipsla_entry->total_receive);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TOTAL_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TOTAL_SEND].name,
            p_ipsla_entry->total_send);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_SUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RTT_SUM].name,
            p_ipsla_entry->rtt_sum);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RTT_MAX].name,
            p_ipsla_entry->rtt_max);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_RTT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_RTT_MIN].name,
            p_ipsla_entry->rtt_min);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT].name,
            p_ipsla_entry->probe_result_config_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX].name,
            p_ipsla_entry->probe_result_index);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT].name,
            p_ipsla_entry->test_result_config_cnt);
    }
    if (FLD_MATCH(TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX].name,
            p_ipsla_entry->test_result_index);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsla_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsla_entry_master_t *p_master = _g_p_tbl_ipsla_entry_master;
    tbl_ipsla_entry_t *p_db_ipsla_entry = NULL;
    uint32 entry_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (entry_id = 0; entry_id < GLB_IPSLA_ENTRY_MAX+1; entry_id++)
    {
        p_db_ipsla_entry = p_master->ipslaentry_array[entry_id];
        if (NULL == p_db_ipsla_entry)
        {
            continue;
        }
        rc |= fn(p_db_ipsla_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipsla_entry_master_t*
tbl_ipsla_entry_get_master()
{
    return _g_p_tbl_ipsla_entry_master;
}

tbl_ipsla_entry_master_t*
tbl_ipsla_entry_init_ipsla_entry()
{
    _g_p_tbl_ipsla_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipsla_entry_master_t));
    return _g_p_tbl_ipsla_entry_master;
}


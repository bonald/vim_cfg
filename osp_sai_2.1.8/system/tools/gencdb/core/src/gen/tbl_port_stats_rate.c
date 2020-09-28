
#include "proto.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "cdb_data_cmp.h"

tbl_port_stats_rate_master_t *_g_p_tbl_port_stats_rate_master = NULL;

static uint32
_tbl_port_stats_rate_hash_make(tbl_port_stats_rate_t *p_port_stats_rate)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_port_stats_rate->key;
    for (index = 0; index < sizeof(p_port_stats_rate->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_port_stats_rate_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_port_stats_rate_t *p_port_stats_rate1 = (tbl_port_stats_rate_t*)p_arg1;
    tbl_port_stats_rate_t *p_port_stats_rate2 = (tbl_port_stats_rate_t*)p_arg2;

    if (0 == sal_memcmp(&p_port_stats_rate1->key, &p_port_stats_rate2->key, sizeof(tbl_port_stats_rate_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_port_stats_rate_add_port_stats_rate_sync(tbl_port_stats_rate_t *p_port_stats_rate, uint32 sync)
{
    tbl_port_stats_rate_master_t *p_master = _g_p_tbl_port_stats_rate_master;
    tbl_port_stats_rate_t *p_db_port_stats_rate = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_port_stats_rate_get_port_stats_rate(&p_port_stats_rate->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port_stats_rate = XCALLOC(MEM_TBL_PORT_STATS_RATE, sizeof(tbl_port_stats_rate_t));
    if (NULL == p_db_port_stats_rate)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port_stats_rate, p_port_stats_rate, sizeof(tbl_port_stats_rate_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->port_stats_rate_hash, (void*)p_db_port_stats_rate, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->port_stats_rate_list, p_db_port_stats_rate);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PORT_STATS_RATE, p_db_port_stats_rate);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PORT_STATS_RATE, p_db_port_stats_rate);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_rate_del_port_stats_rate_sync(tbl_port_stats_rate_key_t *p_port_stats_rate_key, uint32 sync)
{
    tbl_port_stats_rate_master_t *p_master = _g_p_tbl_port_stats_rate_master;
    tbl_port_stats_rate_t *p_db_port_stats_rate = NULL;

    /* 1. lookup entry exist */
    p_db_port_stats_rate = tbl_port_stats_rate_get_port_stats_rate(p_port_stats_rate_key);
    if (NULL == p_db_port_stats_rate)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PORT_STATS_RATE, p_db_port_stats_rate);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PORT_STATS_RATE, p_db_port_stats_rate);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->port_stats_rate_hash, (void*)p_db_port_stats_rate);
    ctclib_slistnode_delete(p_master->port_stats_rate_list, p_db_port_stats_rate);

    /* 4. free db entry */
    XFREE(MEM_TBL_PORT_STATS_RATE, p_db_port_stats_rate);

    return PM_E_NONE;
}

int32
tbl_port_stats_rate_set_port_stats_rate_field_sync(tbl_port_stats_rate_t *p_port_stats_rate, tbl_port_stats_rate_field_id_t field_id, uint32 sync)
{
    tbl_port_stats_rate_t *p_db_port_stats_rate = NULL;

    /* 1. lookup entry exist */
    p_db_port_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&p_port_stats_rate->key);
    if (NULL == p_db_port_stats_rate)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PORT_STATS_RATE_FLD_PORT_ID:
        p_db_port_stats_rate->port_id = p_port_stats_rate->port_id;
        break;

    case TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE:
        p_db_port_stats_rate->octets_rcv_rate = p_port_stats_rate->octets_rcv_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE:
        p_db_port_stats_rate->pkts_rcv_rate = p_port_stats_rate->pkts_rcv_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE:
        p_db_port_stats_rate->octets_send_rate = p_port_stats_rate->octets_send_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE:
        p_db_port_stats_rate->pkts_send_rate = p_port_stats_rate->pkts_send_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL:
        p_db_port_stats_rate->load_interval = p_port_stats_rate->load_interval;
        break;

    case TBL_PORT_STATS_RATE_FLD_INPUT_RATE:
        p_db_port_stats_rate->input_rate = p_port_stats_rate->input_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE:
        p_db_port_stats_rate->input_rate_state = p_port_stats_rate->input_rate_state;
        break;

    case TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD:
        p_db_port_stats_rate->input_rate_threshold = p_port_stats_rate->input_rate_threshold;
        break;

    case TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME:
        p_db_port_stats_rate->input_rate_resume = p_port_stats_rate->input_rate_resume;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE:
        p_db_port_stats_rate->output_rate = p_port_stats_rate->output_rate;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE:
        p_db_port_stats_rate->output_rate_state = p_port_stats_rate->output_rate_state;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD:
        p_db_port_stats_rate->output_rate_threshold = p_port_stats_rate->output_rate_threshold;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME:
        p_db_port_stats_rate->output_rate_resume = p_port_stats_rate->output_rate_resume;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD:
        p_db_port_stats_rate->output_discard_threshold = p_port_stats_rate->output_discard_threshold;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL:
        p_db_port_stats_rate->output_discard_interval = p_port_stats_rate->output_discard_interval;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS:
        p_db_port_stats_rate->output_discard_packets = p_port_stats_rate->output_discard_packets;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS:
        p_db_port_stats_rate->output_discard_total_packets = p_port_stats_rate->output_discard_total_packets;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE:
        p_db_port_stats_rate->output_discard_state = p_port_stats_rate->output_discard_state;
        break;

    case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL:
        p_db_port_stats_rate->output_discard_running_interval = p_port_stats_rate->output_discard_running_interval;
        break;

    case TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER:
        p_db_port_stats_rate->t_output_discard_timer = p_port_stats_rate->t_output_discard_timer;
        break;

    case TBL_PORT_STATS_RATE_FLD_MAX:
        sal_memcpy(p_db_port_stats_rate, p_port_stats_rate, sizeof(tbl_port_stats_rate_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PORT_STATS_RATE, field_id, p_db_port_stats_rate);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PORT_STATS_RATE, field_id, p_db_port_stats_rate);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_rate_add_port_stats_rate(tbl_port_stats_rate_t *p_port_stats_rate)
{
    return tbl_port_stats_rate_add_port_stats_rate_sync(p_port_stats_rate, TRUE);
}

int32
tbl_port_stats_rate_del_port_stats_rate(tbl_port_stats_rate_key_t *p_port_stats_rate_key)
{
    return tbl_port_stats_rate_del_port_stats_rate_sync(p_port_stats_rate_key, TRUE);
}

int32
tbl_port_stats_rate_set_port_stats_rate_field(tbl_port_stats_rate_t *p_port_stats_rate, tbl_port_stats_rate_field_id_t field_id)
{
    return tbl_port_stats_rate_set_port_stats_rate_field_sync(p_port_stats_rate, field_id, TRUE);
}

tbl_port_stats_rate_t*
tbl_port_stats_rate_get_port_stats_rate(tbl_port_stats_rate_key_t *p_port_stats_rate_key)
{
    tbl_port_stats_rate_master_t *p_master = _g_p_tbl_port_stats_rate_master;
    tbl_port_stats_rate_t lkp;

    sal_memcpy(&lkp.key, p_port_stats_rate_key, sizeof(tbl_port_stats_rate_key_t));
    return ctclib_hash_lookup(p_master->port_stats_rate_hash, &lkp);
}

char*
tbl_port_stats_rate_key_val2str(tbl_port_stats_rate_t *p_port_stats_rate, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_port_stats_rate->key.ifindex);
    return str;
}

int32
tbl_port_stats_rate_key_str2val(char *str, tbl_port_stats_rate_t *p_port_stats_rate)
{
    int32 ret = 0;

    p_port_stats_rate->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_port_stats_rate_key_name_dump(tbl_port_stats_rate_t *p_port_stats_rate, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS_RATE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_port_stats_rate->key.ifindex);
    return str;
}

char*
tbl_port_stats_rate_key_value_dump(tbl_port_stats_rate_t *p_port_stats_rate, char *str)
{
    sal_sprintf(str, "%u", p_port_stats_rate->key.ifindex);
    return str;
}

char*
tbl_port_stats_rate_field_name_dump(tbl_port_stats_rate_t *p_port_stats_rate, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS_RATE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PORT_STATS_RATE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_port_stats_rate_field_value_dump(tbl_port_stats_rate_t *p_port_stats_rate, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->key.ifindex);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PORT_ID, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->port_id);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->octets_rcv_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->pkts_rcv_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->octets_send_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->pkts_send_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->load_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->input_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->input_rate_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->input_rate_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->input_rate_resume);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_rate_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_rate_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_rate_resume);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->output_discard_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_discard_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->output_discard_packets);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_port_stats_rate->output_discard_total_packets);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_discard_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_port_stats_rate->output_discard_running_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port_stats_rate->t_output_discard_timer);
    }
    return str;
}

char**
tbl_port_stats_rate_table_dump(tbl_port_stats_rate_t *p_port_stats_rate, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_port_stats_rate_key_name_dump(p_port_stats_rate, buf));
    for(i=1; i<TBL_PORT_STATS_RATE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_port_stats_rate_field_name_dump(p_port_stats_rate, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_port_stats_rate_field_value_dump(p_port_stats_rate, i, buf));
    }
    return str;
}

int32
tbl_port_stats_rate_field_value_parser(char *str, int32 field_id, tbl_port_stats_rate_t *p_port_stats_rate)
{
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_KEY, field_id))
    {
        int32 ret;
        p_port_stats_rate->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PORT_ID, field_id))
    {
        int32 ret;
        p_port_stats_rate->port_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->octets_rcv_rate = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->pkts_rcv_rate = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->octets_send_rate = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->pkts_send_rate = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL, field_id))
    {
        int32 ret;
        p_port_stats_rate->load_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->input_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->input_rate_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD, field_id))
    {
        int32 ret;
        p_port_stats_rate->input_rate_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME, field_id))
    {
        int32 ret;
        p_port_stats_rate->input_rate_resume = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_rate_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_rate_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_rate_resume = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_threshold = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_packets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_total_packets = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL, field_id))
    {
        int32 ret;
        p_port_stats_rate->output_discard_running_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port_stats_rate->t_output_discard_timer);
    }
    return PM_E_NONE;
}

int32
tbl_port_stats_rate_table_parser(char** array, char* key_value,tbl_port_stats_rate_t *p_port_stats_rate)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS_RATE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_port_stats_rate_key_str2val(key_value, p_port_stats_rate));

    for(i=1; i<TBL_PORT_STATS_RATE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_port_stats_rate_field_value_parser( array[j++], i, p_port_stats_rate));
    }

    return PM_E_NONE;
}

int32
tbl_port_stats_rate_dump_one(tbl_port_stats_rate_t *p_port_stats_rate, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT_STATS_RATE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_KEY].name,
            p_port_stats_rate->key.ifindex);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PORT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_PORT_ID].name,
            p_port_stats_rate->port_id);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE].name,
            p_port_stats_rate->octets_rcv_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE].name,
            p_port_stats_rate->pkts_rcv_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE].name,
            p_port_stats_rate->octets_send_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE].name,
            p_port_stats_rate->pkts_send_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL].name,
            p_port_stats_rate->load_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_INPUT_RATE].name,
            p_port_stats_rate->input_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE].name,
            p_port_stats_rate->input_rate_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD].name,
            p_port_stats_rate->input_rate_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME].name,
            p_port_stats_rate->input_rate_resume);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE].name,
            p_port_stats_rate->output_rate);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE].name,
            p_port_stats_rate->output_rate_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD].name,
            p_port_stats_rate->output_rate_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME].name,
            p_port_stats_rate->output_rate_resume);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD].name,
            p_port_stats_rate->output_discard_threshold);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL].name,
            p_port_stats_rate->output_discard_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS].name,
            p_port_stats_rate->output_discard_packets);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS].name,
            p_port_stats_rate->output_discard_total_packets);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE].name,
            p_port_stats_rate->output_discard_state);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL].name,
            p_port_stats_rate->output_discard_running_interval);
    }
    if (FLD_MATCH(TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port_stats_rate->t_output_discard_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_port_stats_rate_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_port_stats_rate_master_t *p_master = _g_p_tbl_port_stats_rate_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_port_stats_rate_t *p_db_port_stats_rate = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_stats_rate_list, p_db_port_stats_rate, listnode, next)
    {
        rc |= fn(p_db_port_stats_rate, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_port_stats_rate_master_t*
tbl_port_stats_rate_get_master()
{
    return _g_p_tbl_port_stats_rate_master;
}

tbl_port_stats_rate_master_t*
tbl_port_stats_rate_init_port_stats_rate()
{
    _g_p_tbl_port_stats_rate_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_port_stats_rate_master_t));
    _g_p_tbl_port_stats_rate_master->port_stats_rate_hash = ctclib_hash_create(_tbl_port_stats_rate_hash_make, _tbl_port_stats_rate_hash_cmp);
    _g_p_tbl_port_stats_rate_master->port_stats_rate_list = ctclib_slist_create(tbl_port_stats_rate_cmp, NULL);
    return _g_p_tbl_port_stats_rate_master;
}


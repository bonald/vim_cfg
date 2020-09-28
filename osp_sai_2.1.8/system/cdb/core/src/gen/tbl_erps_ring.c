
#include "proto.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "cdb_data_cmp.h"

tbl_erps_ring_master_t *_g_p_tbl_erps_ring_master = NULL;

#include "erps_define.h"

static uint32
_tbl_erps_ring_hash_make(tbl_erps_ring_t *p_erps_ring)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_erps_ring->key;
    for (index = 0; index < sizeof(p_erps_ring->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_erps_ring_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_erps_ring_t *p_erps_ring1 = (tbl_erps_ring_t*)p_arg1;
    tbl_erps_ring_t *p_erps_ring2 = (tbl_erps_ring_t*)p_arg2;

    if (0 == sal_memcmp(&p_erps_ring1->key, &p_erps_ring2->key, sizeof(tbl_erps_ring_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_erps_ring_add_erps_ring_sync(tbl_erps_ring_t *p_erps_ring, uint32 sync)
{
    tbl_erps_ring_master_t *p_master = _g_p_tbl_erps_ring_master;
    tbl_erps_ring_t *p_db_erps_ring = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_erps_ring_get_erps_ring(&p_erps_ring->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_erps_ring = XCALLOC(MEM_TBL_ERPS_RING, sizeof(tbl_erps_ring_t));
    if (NULL == p_db_erps_ring)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_erps_ring, p_erps_ring, sizeof(tbl_erps_ring_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->erps_ring_hash, (void*)p_db_erps_ring, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->erps_ring_list, p_db_erps_ring);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ERPS_RING, p_db_erps_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ERPS_RING, p_db_erps_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_erps_ring_del_erps_ring_sync(tbl_erps_ring_key_t *p_erps_ring_key, uint32 sync)
{
    tbl_erps_ring_master_t *p_master = _g_p_tbl_erps_ring_master;
    tbl_erps_ring_t *p_db_erps_ring = NULL;

    /* 1. lookup entry exist */
    p_db_erps_ring = tbl_erps_ring_get_erps_ring(p_erps_ring_key);
    if (NULL == p_db_erps_ring)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ERPS_RING, p_db_erps_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ERPS_RING, p_db_erps_ring);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->erps_ring_hash, (void*)p_db_erps_ring);
    ctclib_slistnode_delete(p_master->erps_ring_list, p_db_erps_ring);

    /* 4. free db entry */
    XFREE(MEM_TBL_ERPS_RING, p_db_erps_ring);

    return PM_E_NONE;
}

int32
tbl_erps_ring_set_erps_ring_field_sync(tbl_erps_ring_t *p_erps_ring, tbl_erps_ring_field_id_t field_id, uint32 sync)
{
    tbl_erps_ring_t *p_db_erps_ring = NULL;

    /* 1. lookup entry exist */
    p_db_erps_ring = tbl_erps_ring_get_erps_ring(&p_erps_ring->key);
    if (NULL == p_db_erps_ring)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ERPS_RING_FLD_ERPS_RING_STATE:
        p_db_erps_ring->erps_ring_state = p_erps_ring->erps_ring_state;
        break;

    case TBL_ERPS_RING_FLD_ERPS_RING_LEVEL:
        p_db_erps_ring->erps_ring_level = p_erps_ring->erps_ring_level;
        break;

    case TBL_ERPS_RING_FLD_INSTANCE_ID_NUM:
        p_db_erps_ring->instance_id_num = p_erps_ring->instance_id_num;
        break;

    case TBL_ERPS_RING_FLD_INSTANCE_ID:
        sal_memcpy(p_db_erps_ring->instance_id, p_erps_ring->instance_id, sizeof(p_erps_ring->instance_id));
        break;

    case TBL_ERPS_RING_FLD_NODE_ROLE:
        p_db_erps_ring->node_role = p_erps_ring->node_role;
        break;

    case TBL_ERPS_RING_FLD_EDGE_NODE_ROLE:
        p_db_erps_ring->edge_node_role = p_erps_ring->edge_node_role;
        break;

    case TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE:
        p_db_erps_ring->erps_srpt_enable = p_erps_ring->erps_srpt_enable;
        break;

    case TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX:
        p_db_erps_ring->primary_port_ifindex = p_erps_ring->primary_port_ifindex;
        break;

    case TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX:
        p_db_erps_ring->secondary_port_ifindex = p_erps_ring->secondary_port_ifindex;
        break;

    case TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX:
        p_db_erps_ring->edge_port_ifindex = p_erps_ring->edge_port_ifindex;
        break;

    case TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX:
        p_db_erps_ring->common_port_ifindex = p_erps_ring->common_port_ifindex;
        break;

    case TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX:
        p_db_erps_ring->blocked_port_ifindex = p_erps_ring->blocked_port_ifindex;
        break;

    case TBL_ERPS_RING_FLD_BLOCKED:
        p_db_erps_ring->blocked = p_erps_ring->blocked;
        break;

    case TBL_ERPS_RING_FLD_HELLO_TIMER:
        p_db_erps_ring->hello_timer = p_erps_ring->hello_timer;
        break;

    case TBL_ERPS_RING_FLD_FAILURE_TIMER:
        p_db_erps_ring->failure_timer = p_erps_ring->failure_timer;
        break;

    case TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER:
        p_db_erps_ring->pre_forwarding_timer = p_erps_ring->pre_forwarding_timer;
        break;

    case TBL_ERPS_RING_FLD_FLUSH_TIMER:
        p_db_erps_ring->flush_timer = p_erps_ring->flush_timer;
        break;

    case TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER:
        p_db_erps_ring->edge_hello_timer = p_erps_ring->edge_hello_timer;
        break;

    case TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER:
        p_db_erps_ring->edge_failure_timer = p_erps_ring->edge_failure_timer;
        break;

    case TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER:
        p_db_erps_ring->edge_block_timer = p_erps_ring->edge_block_timer;
        break;

    case TBL_ERPS_RING_FLD_HELLO_SEQ:
        p_db_erps_ring->hello_seq = p_erps_ring->hello_seq;
        break;

    case TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ:
        p_db_erps_ring->expect_hello_seq = p_erps_ring->expect_hello_seq;
        break;

    case TBL_ERPS_RING_FLD_RCV_HELLO_CNT:
        p_db_erps_ring->rcv_hello_cnt = p_erps_ring->rcv_hello_cnt;
        break;

    case TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT:
        p_db_erps_ring->rcv_ring_up_flush_fdb_cnt = p_erps_ring->rcv_ring_up_flush_fdb_cnt;
        break;

    case TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT:
        p_db_erps_ring->rcv_ring_down_flush_fdb_cnt = p_erps_ring->rcv_ring_down_flush_fdb_cnt;
        break;

    case TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT:
        p_db_erps_ring->rcv_link_down_cnt = p_erps_ring->rcv_link_down_cnt;
        break;

    case TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT:
        p_db_erps_ring->rcv_edge_hello_cnt = p_erps_ring->rcv_edge_hello_cnt;
        break;

    case TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT:
        p_db_erps_ring->rcv_major_fault_cnt = p_erps_ring->rcv_major_fault_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_HELLO_CNT:
        p_db_erps_ring->send_hello_cnt = p_erps_ring->send_hello_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT:
        p_db_erps_ring->send_ring_up_flush_fdb_cnt = p_erps_ring->send_ring_up_flush_fdb_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT:
        p_db_erps_ring->send_ring_down_flush_fdb_cnt = p_erps_ring->send_ring_down_flush_fdb_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT:
        p_db_erps_ring->send_link_down_cnt = p_erps_ring->send_link_down_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT:
        p_db_erps_ring->send_edge_hello_cnt = p_erps_ring->send_edge_hello_cnt;
        break;

    case TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT:
        p_db_erps_ring->send_major_fault_cnt = p_erps_ring->send_major_fault_cnt;
        break;

    case TBL_ERPS_RING_FLD_MAX:
        sal_memcpy(p_db_erps_ring, p_erps_ring, sizeof(tbl_erps_ring_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ERPS_RING, field_id, p_db_erps_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ERPS_RING, field_id, p_db_erps_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_erps_ring_add_erps_ring(tbl_erps_ring_t *p_erps_ring)
{
    return tbl_erps_ring_add_erps_ring_sync(p_erps_ring, TRUE);
}

int32
tbl_erps_ring_del_erps_ring(tbl_erps_ring_key_t *p_erps_ring_key)
{
    return tbl_erps_ring_del_erps_ring_sync(p_erps_ring_key, TRUE);
}

int32
tbl_erps_ring_set_erps_ring_field(tbl_erps_ring_t *p_erps_ring, tbl_erps_ring_field_id_t field_id)
{
    return tbl_erps_ring_set_erps_ring_field_sync(p_erps_ring, field_id, TRUE);
}

tbl_erps_ring_t*
tbl_erps_ring_get_erps_ring(tbl_erps_ring_key_t *p_erps_ring_key)
{
    tbl_erps_ring_master_t *p_master = _g_p_tbl_erps_ring_master;
    tbl_erps_ring_t lkp;

    sal_memcpy(&lkp.key, p_erps_ring_key, sizeof(tbl_erps_ring_key_t));
    return ctclib_hash_lookup(p_master->erps_ring_hash, &lkp);
}

char*
tbl_erps_ring_key_val2str(tbl_erps_ring_t *p_erps_ring, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_erps_ring_key_val2str(buf, MAX_CMD_STR_LEN, &p_erps_ring->key));
    return str;
}

int32
tbl_erps_ring_key_str2val(char *str, tbl_erps_ring_t *p_erps_ring)
{
    int32 ret = 0;

    ret = cdb_erps_ring_key_str2val(str, &p_erps_ring->key);
    return ret;
}

char*
tbl_erps_ring_key_name_dump(tbl_erps_ring_t *p_erps_ring, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_RING);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_erps_ring_key_val2str(buf, MAX_CMD_STR_LEN, &p_erps_ring->key));
    return str;
}

char*
tbl_erps_ring_key_value_dump(tbl_erps_ring_t *p_erps_ring, char *str)
{

    cdb_erps_ring_key_val2str(str, MAX_CMD_STR_LEN, &p_erps_ring->key);
    return str;
}

char*
tbl_erps_ring_field_name_dump(tbl_erps_ring_t *p_erps_ring, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ERPS_RING_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_erps_ring_field_value_dump(tbl_erps_ring_t *p_erps_ring, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ERPS_RING_FLD_KEY, field_id))
    {
        cdb_erps_ring_key_val2str(str, MAX_CMD_STR_LEN, &p_erps_ring->key);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(erps_state_strs, ERPS_STATE_MAX, p_erps_ring->erps_ring_state));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_LEVEL, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(erps_ring_level_strs, ERPS_RING_LEVEL_MAX, p_erps_ring->erps_ring_level));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->instance_id_num);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->instance_id, sizeof(p_erps_ring->instance_id)/4);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_NODE_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(erps_node_role_strs, ERPS_NODE_ROLE_MAX, p_erps_ring->node_role));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_NODE_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(erps_edge_node_role_strs, ERPS_EDGE_NODE_ROLE_MAX, p_erps_ring->edge_node_role));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->erps_srpt_enable);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->primary_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->secondary_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->edge_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->common_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->blocked_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->blocked);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->hello_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FAILURE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->failure_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->pre_forwarding_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FLUSH_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->flush_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->edge_hello_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->edge_failure_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_erps_ring->edge_block_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_SEQ, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->hello_seq);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->expect_hello_seq);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_HELLO_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_ring_up_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_ring_down_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_link_down_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_edge_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->rcv_major_fault_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_HELLO_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_ring_up_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_ring_down_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_link_down_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_edge_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_erps_ring->send_major_fault_cnt);
    }
    return str;
}

char**
tbl_erps_ring_table_dump(tbl_erps_ring_t *p_erps_ring, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_erps_ring_key_name_dump(p_erps_ring, buf));
    for(i=1; i<TBL_ERPS_RING_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_erps_ring_field_name_dump(p_erps_ring, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_erps_ring_field_value_dump(p_erps_ring, i, buf));
    }
    return str;
}

int32
tbl_erps_ring_field_value_parser(char *str, int32 field_id, tbl_erps_ring_t *p_erps_ring)
{
    if (FLD_MATCH(TBL_ERPS_RING_FLD_KEY, field_id))
    {
        cdb_erps_ring_key_str2val(str, &p_erps_ring->key);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_STATE, field_id))
    {
        cdb_enum_str2val(erps_state_strs, ERPS_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_LEVEL, field_id))
    {
        cdb_enum_str2val(erps_ring_level_strs, ERPS_RING_LEVEL_MAX, str);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID_NUM, field_id))
    {
        int32 ret;
        p_erps_ring->instance_id_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID, field_id))
    {
        cdb_uint32_array_str2val(str, p_erps_ring->instance_id);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_NODE_ROLE, field_id))
    {
        cdb_enum_str2val(erps_node_role_strs, ERPS_NODE_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_NODE_ROLE, field_id))
    {
        cdb_enum_str2val(erps_edge_node_role_strs, ERPS_EDGE_NODE_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE, field_id))
    {
        int32 ret;
        p_erps_ring->erps_srpt_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_erps_ring->primary_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_erps_ring->secondary_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_erps_ring->edge_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_erps_ring->common_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_erps_ring->blocked_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED, field_id))
    {
        int32 ret;
        p_erps_ring->blocked = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->hello_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FAILURE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->failure_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->pre_forwarding_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FLUSH_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->flush_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->edge_hello_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->edge_failure_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER, field_id))
    {
        ctc_task_str2val(str, p_erps_ring->edge_block_timer);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_SEQ, field_id))
    {
        int32 ret;
        p_erps_ring->hello_seq = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ, field_id))
    {
        int32 ret;
        p_erps_ring->expect_hello_seq = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_HELLO_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_hello_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_ring_up_flush_fdb_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_ring_down_flush_fdb_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_link_down_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_edge_hello_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->rcv_major_fault_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_HELLO_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_hello_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_ring_up_flush_fdb_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_ring_down_flush_fdb_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_link_down_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_edge_hello_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT, field_id))
    {
        int32 ret;
        p_erps_ring->send_major_fault_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_erps_ring_table_parser(char** array, char* key_value,tbl_erps_ring_t *p_erps_ring)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_erps_ring_key_str2val(key_value, p_erps_ring));

    for(i=1; i<TBL_ERPS_RING_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_erps_ring_field_value_parser( array[j++], i, p_erps_ring));
    }

    return PM_E_NONE;
}

int32
tbl_erps_ring_dump_one(tbl_erps_ring_t *p_erps_ring, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_RING);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ERPS_RING_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_KEY].name, 
            cdb_erps_ring_key_val2str(buf, MAX_CMD_STR_LEN, &p_erps_ring->key));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_ERPS_RING_STATE].name, 
            cdb_enum_val2str(erps_state_strs, ERPS_STATE_MAX, p_erps_ring->erps_ring_state));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_RING_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_ERPS_RING_LEVEL].name, 
            cdb_enum_val2str(erps_ring_level_strs, ERPS_RING_LEVEL_MAX, p_erps_ring->erps_ring_level));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_INSTANCE_ID_NUM].name,
            p_erps_ring->instance_id_num);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_INSTANCE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_INSTANCE_ID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->instance_id, sizeof(p_erps_ring->instance_id)/4));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_NODE_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_NODE_ROLE].name, 
            cdb_enum_val2str(erps_node_role_strs, ERPS_NODE_ROLE_MAX, p_erps_ring->node_role));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_NODE_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_EDGE_NODE_ROLE].name, 
            cdb_enum_val2str(erps_edge_node_role_strs, ERPS_EDGE_NODE_ROLE_MAX, p_erps_ring->edge_node_role));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE].name,
            p_erps_ring->erps_srpt_enable);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX].name,
            p_erps_ring->primary_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX].name,
            p_erps_ring->secondary_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX].name,
            p_erps_ring->edge_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX].name,
            p_erps_ring->common_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX].name,
            p_erps_ring->blocked_port_ifindex);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_BLOCKED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_BLOCKED].name,
            p_erps_ring->blocked);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_HELLO_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->hello_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FAILURE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_FAILURE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->failure_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->pre_forwarding_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_FLUSH_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_FLUSH_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->flush_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->edge_hello_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->edge_failure_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_erps_ring->edge_block_timer));
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_HELLO_SEQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_HELLO_SEQ].name,
            p_erps_ring->hello_seq);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ].name,
            p_erps_ring->expect_hello_seq);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_HELLO_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_HELLO_CNT].name,
            p_erps_ring->rcv_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT].name,
            p_erps_ring->rcv_ring_up_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT].name,
            p_erps_ring->rcv_ring_down_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT].name,
            p_erps_ring->rcv_link_down_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT].name,
            p_erps_ring->rcv_edge_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT].name,
            p_erps_ring->rcv_major_fault_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_HELLO_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_HELLO_CNT].name,
            p_erps_ring->send_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT].name,
            p_erps_ring->send_ring_up_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT].name,
            p_erps_ring->send_ring_down_flush_fdb_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT].name,
            p_erps_ring->send_link_down_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT].name,
            p_erps_ring->send_edge_hello_cnt);
    }
    if (FLD_MATCH(TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT].name,
            p_erps_ring->send_major_fault_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_erps_ring_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_erps_ring_master_t *p_master = _g_p_tbl_erps_ring_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_erps_ring_t *p_db_erps_ring = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->erps_ring_list, p_db_erps_ring, listnode, next)
    {
        rc |= fn(p_db_erps_ring, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_erps_ring_master_t*
tbl_erps_ring_get_master()
{
    return _g_p_tbl_erps_ring_master;
}

tbl_erps_ring_master_t*
tbl_erps_ring_init_erps_ring()
{
    _g_p_tbl_erps_ring_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_erps_ring_master_t));
    _g_p_tbl_erps_ring_master->erps_ring_hash = ctclib_hash_create(_tbl_erps_ring_hash_make, _tbl_erps_ring_hash_cmp);
    _g_p_tbl_erps_ring_master->erps_ring_list = ctclib_slist_create(tbl_erps_ring_cmp, NULL);
    return _g_p_tbl_erps_ring_master;
}


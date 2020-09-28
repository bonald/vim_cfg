
#include "proto.h"
#include "gen/tbl_aclqos_if_stats_define.h"
#include "gen/tbl_aclqos_if_stats.h"
#include "cdb_data_cmp.h"

tbl_aclqos_if_stats_master_t *_g_p_tbl_aclqos_if_stats_master = NULL;

static uint32
_tbl_aclqos_if_stats_hash_make(tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_aclqos_if_stats->key;
    for (index = 0; index < sizeof(p_aclqos_if_stats->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_aclqos_if_stats_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_aclqos_if_stats_t *p_aclqos_if_stats1 = (tbl_aclqos_if_stats_t*)p_arg1;
    tbl_aclqos_if_stats_t *p_aclqos_if_stats2 = (tbl_aclqos_if_stats_t*)p_arg2;

    if (0 == sal_memcmp(&p_aclqos_if_stats1->key, &p_aclqos_if_stats2->key, sizeof(tbl_aclqos_if_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_aclqos_if_stats_add_aclqos_if_stats_sync(tbl_aclqos_if_stats_t *p_aclqos_if_stats, uint32 sync)
{
    tbl_aclqos_if_stats_master_t *p_master = _g_p_tbl_aclqos_if_stats_master;
    tbl_aclqos_if_stats_t *p_db_aclqos_if_stats = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_aclqos_if_stats_get_aclqos_if_stats(&p_aclqos_if_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_aclqos_if_stats = XCALLOC(MEM_TBL_ACLQOS_IF_STATS, sizeof(tbl_aclqos_if_stats_t));
    if (NULL == p_db_aclqos_if_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_aclqos_if_stats, p_aclqos_if_stats, sizeof(tbl_aclqos_if_stats_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->if_hash, (void*)p_db_aclqos_if_stats, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_aclqos_if_stats);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACLQOS_IF_STATS, p_db_aclqos_if_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACLQOS_IF_STATS, p_db_aclqos_if_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_del_aclqos_if_stats_sync(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key, uint32 sync)
{
    tbl_aclqos_if_stats_master_t *p_master = _g_p_tbl_aclqos_if_stats_master;
    tbl_aclqos_if_stats_t *p_db_aclqos_if_stats = NULL;

    /* 1. lookup entry exist */
    p_db_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats(p_aclqos_if_stats_key);
    if (NULL == p_db_aclqos_if_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACLQOS_IF_STATS, p_db_aclqos_if_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACLQOS_IF_STATS, p_db_aclqos_if_stats);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->if_hash, (void*)p_db_aclqos_if_stats);
    ctclib_slistnode_delete(p_master->if_list, p_db_aclqos_if_stats);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACLQOS_IF_STATS, p_db_aclqos_if_stats);

    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_set_aclqos_if_stats_field_sync(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_aclqos_if_stats_field_id_t field_id, uint32 sync)
{
    tbl_aclqos_if_stats_t *p_db_aclqos_if_stats = NULL;

    /* 1. lookup entry exist */
    p_db_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats(&p_aclqos_if_stats->key);
    if (NULL == p_db_aclqos_if_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if_stats, p_aclqos_if_stats, flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_QUEUE)
        break;

    case TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT:
        sal_memcpy(p_db_aclqos_if_stats->queue_transimt_pkt, p_aclqos_if_stats->queue_transimt_pkt, sizeof(p_aclqos_if_stats->queue_transimt_pkt));
        break;

    case TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE:
        sal_memcpy(p_db_aclqos_if_stats->queue_transimt_byte, p_aclqos_if_stats->queue_transimt_byte, sizeof(p_aclqos_if_stats->queue_transimt_byte));
        break;

    case TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT:
        sal_memcpy(p_db_aclqos_if_stats->queue_drop_pkt, p_aclqos_if_stats->queue_drop_pkt, sizeof(p_aclqos_if_stats->queue_drop_pkt));
        break;

    case TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE:
        sal_memcpy(p_db_aclqos_if_stats->queue_drop_byte, p_aclqos_if_stats->queue_drop_byte, sizeof(p_aclqos_if_stats->queue_drop_byte));
        break;

    case TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET:
        p_db_aclqos_if_stats->green_packet = p_aclqos_if_stats->green_packet;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE:
        p_db_aclqos_if_stats->green_byte = p_aclqos_if_stats->green_byte;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET:
        p_db_aclqos_if_stats->yellow_packet = p_aclqos_if_stats->yellow_packet;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE:
        p_db_aclqos_if_stats->yellow_byte = p_aclqos_if_stats->yellow_byte;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_RED_PACKET:
        p_db_aclqos_if_stats->red_packet = p_aclqos_if_stats->red_packet;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_RED_BYTE:
        p_db_aclqos_if_stats->red_byte = p_aclqos_if_stats->red_byte;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT:
        p_db_aclqos_if_stats->green_packet_out = p_aclqos_if_stats->green_packet_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT:
        p_db_aclqos_if_stats->green_byte_out = p_aclqos_if_stats->green_byte_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT:
        p_db_aclqos_if_stats->yellow_packet_out = p_aclqos_if_stats->yellow_packet_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT:
        p_db_aclqos_if_stats->yellow_byte_out = p_aclqos_if_stats->yellow_byte_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT:
        p_db_aclqos_if_stats->red_packet_out = p_aclqos_if_stats->red_packet_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT:
        p_db_aclqos_if_stats->red_byte_out = p_aclqos_if_stats->red_byte_out;
        break;

    case TBL_ACLQOS_IF_STATS_FLD_MAX:
        sal_memcpy(p_db_aclqos_if_stats, p_aclqos_if_stats, sizeof(tbl_aclqos_if_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACLQOS_IF_STATS, field_id, p_db_aclqos_if_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACLQOS_IF_STATS, field_id, p_db_aclqos_if_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_add_aclqos_if_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    return tbl_aclqos_if_stats_add_aclqos_if_stats_sync(p_aclqos_if_stats, TRUE);
}

int32
tbl_aclqos_if_stats_del_aclqos_if_stats(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key)
{
    return tbl_aclqos_if_stats_del_aclqos_if_stats_sync(p_aclqos_if_stats_key, TRUE);
}

int32
tbl_aclqos_if_stats_set_aclqos_if_stats_field(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_aclqos_if_stats_field_id_t field_id)
{
    return tbl_aclqos_if_stats_set_aclqos_if_stats_field_sync(p_aclqos_if_stats, field_id, TRUE);
}

tbl_aclqos_if_stats_t*
tbl_aclqos_if_stats_get_aclqos_if_stats(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key)
{
    tbl_aclqos_if_stats_master_t *p_master = _g_p_tbl_aclqos_if_stats_master;
    tbl_aclqos_if_stats_t lkp;

    sal_memcpy(&lkp.key, p_aclqos_if_stats_key, sizeof(tbl_aclqos_if_stats_key_t));
    return ctclib_hash_lookup(p_master->if_hash, &lkp);
}

char*
tbl_aclqos_if_stats_key_val2str(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_aclqos_if_stats->key.name);
    return str;
}

int32
tbl_aclqos_if_stats_key_str2val(char *str, tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    int32 ret = 0;

    sal_strcpy(p_aclqos_if_stats->key.name, str);
    return ret;
}

char*
tbl_aclqos_if_stats_key_name_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF_STATS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_aclqos_if_stats->key.name);
    return str;
}

char*
tbl_aclqos_if_stats_key_value_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char *str)
{
    sal_sprintf(str, "%s", p_aclqos_if_stats->key.name);
    return str;
}

char*
tbl_aclqos_if_stats_field_name_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACLQOS_IF_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_aclqos_if_stats_field_value_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if_stats->key.name);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_QUEUE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT, field_id))
    {
        cdb_uint64_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_transimt_pkt, sizeof(p_aclqos_if_stats->queue_transimt_pkt)/4);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE, field_id))
    {
        cdb_uint64_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_transimt_byte, sizeof(p_aclqos_if_stats->queue_transimt_byte)/4);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT, field_id))
    {
        cdb_uint64_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_drop_pkt, sizeof(p_aclqos_if_stats->queue_drop_pkt)/4);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE, field_id))
    {
        cdb_uint64_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_drop_byte, sizeof(p_aclqos_if_stats->queue_drop_byte)/4);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->green_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->green_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->yellow_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->yellow_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->red_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->red_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->green_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->green_byte_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->yellow_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->yellow_byte_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->red_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if_stats->red_byte_out);
    }
    return str;
}

char**
tbl_aclqos_if_stats_table_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_aclqos_if_stats_key_name_dump(p_aclqos_if_stats, buf));
    for(i=1; i<TBL_ACLQOS_IF_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_aclqos_if_stats_field_name_dump(p_aclqos_if_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_aclqos_if_stats_field_value_dump(p_aclqos_if_stats, i, buf));
    }
    return str;
}

int32
tbl_aclqos_if_stats_field_value_parser(char *str, int32 field_id, tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_KEY, field_id))
    {
        sal_strcpy(p_aclqos_if_stats->key.name, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_QUEUE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT, field_id))
    {
        cdb_uint64_array_str2val(str, p_aclqos_if_stats->queue_transimt_pkt);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE, field_id))
    {
        cdb_uint64_array_str2val(str, p_aclqos_if_stats->queue_transimt_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT, field_id))
    {
        cdb_uint64_array_str2val(str, p_aclqos_if_stats->queue_drop_pkt);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE, field_id))
    {
        cdb_uint64_array_str2val(str, p_aclqos_if_stats->queue_drop_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->green_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->green_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->yellow_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->yellow_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->red_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->red_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->green_packet_out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->green_byte_out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->yellow_packet_out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->yellow_byte_out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->red_packet_out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT, field_id))
    {
        int32 ret;
        p_aclqos_if_stats->red_byte_out = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_table_parser(char** array, char* key_value,tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_aclqos_if_stats_key_str2val(key_value, p_aclqos_if_stats));

    for(i=1; i<TBL_ACLQOS_IF_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_aclqos_if_stats_field_value_parser( array[j++], i, p_aclqos_if_stats));
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_dump_one(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_KEY].name,
            p_aclqos_if_stats->key.name);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS].name,
            GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_QUEUE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT].name, 
            cdb_uint64_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_transimt_pkt, sizeof(p_aclqos_if_stats->queue_transimt_pkt)/4));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE].name, 
            cdb_uint64_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_transimt_byte, sizeof(p_aclqos_if_stats->queue_transimt_byte)/4));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT].name, 
            cdb_uint64_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_drop_pkt, sizeof(p_aclqos_if_stats->queue_drop_pkt)/4));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE].name, 
            cdb_uint64_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if_stats->queue_drop_byte, sizeof(p_aclqos_if_stats->queue_drop_byte)/4));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET].name,
            p_aclqos_if_stats->green_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE].name,
            p_aclqos_if_stats->green_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET].name,
            p_aclqos_if_stats->yellow_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE].name,
            p_aclqos_if_stats->yellow_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET].name,
            p_aclqos_if_stats->red_packet);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE].name,
            p_aclqos_if_stats->red_byte);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT].name,
            p_aclqos_if_stats->green_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT].name,
            p_aclqos_if_stats->green_byte_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT].name,
            p_aclqos_if_stats->yellow_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT].name,
            p_aclqos_if_stats->yellow_byte_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT].name,
            p_aclqos_if_stats->red_packet_out);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT].name,
            p_aclqos_if_stats->red_byte_out);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_aclqos_if_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_aclqos_if_stats_master_t *p_master = _g_p_tbl_aclqos_if_stats_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_aclqos_if_stats_t *p_db_aclqos_if_stats = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_aclqos_if_stats, listnode, next)
    {
        rc |= fn(p_db_aclqos_if_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_aclqos_if_stats_master_t*
tbl_aclqos_if_stats_get_master()
{
    return _g_p_tbl_aclqos_if_stats_master;
}

tbl_aclqos_if_stats_master_t*
tbl_aclqos_if_stats_init_aclqos_if_stats()
{
    _g_p_tbl_aclqos_if_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_aclqos_if_stats_master_t));
    _g_p_tbl_aclqos_if_stats_master->if_hash = ctclib_hash_create(_tbl_aclqos_if_stats_hash_make, _tbl_aclqos_if_stats_hash_cmp);
    _g_p_tbl_aclqos_if_stats_master->if_list = ctclib_slist_create(tbl_aclqos_if_stats_cmp, NULL);
    return _g_p_tbl_aclqos_if_stats_master;
}

tbl_aclqos_if_stats_t*
tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(const char* ifname)
{
    tbl_aclqos_if_stats_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_aclqos_if_stats_get_aclqos_if_stats(&key);
}

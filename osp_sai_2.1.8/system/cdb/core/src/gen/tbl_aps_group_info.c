
#include "proto.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "cdb_data_cmp.h"

tbl_aps_group_info_master_t *_g_p_tbl_aps_group_info_master = NULL;

static uint32
_tbl_aps_group_info_hash_make(tbl_aps_group_info_t *p_aps_group_info)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_aps_group_info->key;
    for (index = 0; index < sizeof(p_aps_group_info->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_aps_group_info_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_aps_group_info_t *p_aps_group_info1 = (tbl_aps_group_info_t*)p_arg1;
    tbl_aps_group_info_t *p_aps_group_info2 = (tbl_aps_group_info_t*)p_arg2;

    if (0 == sal_memcmp(&p_aps_group_info1->key, &p_aps_group_info2->key, sizeof(tbl_aps_group_info_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_aps_group_info_add_aps_group_info_sync(tbl_aps_group_info_t *p_aps_group_info, uint32 sync)
{
    tbl_aps_group_info_master_t *p_master = _g_p_tbl_aps_group_info_master;
    tbl_aps_group_info_t *p_db_aps_group_info = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_aps_group_info_get_aps_group_info(&p_aps_group_info->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_aps_group_info = XCALLOC(MEM_TBL_APS_GROUP_INFO, sizeof(tbl_aps_group_info_t));
    if (NULL == p_db_aps_group_info)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_aps_group_info, p_aps_group_info, sizeof(tbl_aps_group_info_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->aps_group_info_hash, (void*)p_db_aps_group_info, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->aps_group_info_list, p_db_aps_group_info);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_APS_GROUP_INFO, p_db_aps_group_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_APS_GROUP_INFO, p_db_aps_group_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aps_group_info_del_aps_group_info_sync(tbl_aps_group_info_key_t *p_aps_group_info_key, uint32 sync)
{
    tbl_aps_group_info_master_t *p_master = _g_p_tbl_aps_group_info_master;
    tbl_aps_group_info_t *p_db_aps_group_info = NULL;

    /* 1. lookup entry exist */
    p_db_aps_group_info = tbl_aps_group_info_get_aps_group_info(p_aps_group_info_key);
    if (NULL == p_db_aps_group_info)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_APS_GROUP_INFO, p_db_aps_group_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_APS_GROUP_INFO, p_db_aps_group_info);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->aps_group_info_hash, (void*)p_db_aps_group_info);
    ctclib_slistnode_delete(p_master->aps_group_info_list, p_db_aps_group_info);

    /* 4. free db entry */
    XFREE(MEM_TBL_APS_GROUP_INFO, p_db_aps_group_info);

    return PM_E_NONE;
}

int32
tbl_aps_group_info_set_aps_group_info_field_sync(tbl_aps_group_info_t *p_aps_group_info, tbl_aps_group_info_field_id_t field_id, uint32 sync)
{
    tbl_aps_group_info_t *p_db_aps_group_info = NULL;

    /* 1. lookup entry exist */
    p_db_aps_group_info = tbl_aps_group_info_get_aps_group_info(&p_aps_group_info->key);
    if (NULL == p_db_aps_group_info)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_APS_GROUP_INFO_FLD_WORKING_NAME:
        sal_memcpy(p_db_aps_group_info->working_name, p_aps_group_info->working_name, sizeof(p_aps_group_info->working_name));
        break;

    case TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME:
        sal_memcpy(p_db_aps_group_info->protection_name, p_aps_group_info->protection_name, sizeof(p_aps_group_info->protection_name));
        break;

    case TBL_APS_GROUP_INFO_FLD_SELECT_NAME:
        sal_memcpy(p_db_aps_group_info->select_name, p_aps_group_info->select_name, sizeof(p_aps_group_info->select_name));
        break;

    case TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE:
        p_db_aps_group_info->aps_group_type = p_aps_group_info->aps_group_type;
        break;

    case TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME:
        p_db_aps_group_info->wait_to_restore_time = p_aps_group_info->wait_to_restore_time;
        break;

    case TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME:
        p_db_aps_group_info->hold_off_time = p_aps_group_info->hold_off_time;
        break;

    case TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE:
        p_db_aps_group_info->is_revertive = p_aps_group_info->is_revertive;
        break;

    case TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH:
        p_db_aps_group_info->active_path = p_aps_group_info->active_path;
        break;

    case TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT:
        p_db_aps_group_info->bridge_permanent = p_aps_group_info->bridge_permanent;
        break;

    case TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT:
        p_db_aps_group_info->current_event = p_aps_group_info->current_event;
        break;

    case TBL_APS_GROUP_INFO_FLD_DFOP_REASON:
        p_db_aps_group_info->dfop_reason = p_aps_group_info->dfop_reason;
        break;

    case TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL:
        p_db_aps_group_info->sc_aps_intervel = p_aps_group_info->sc_aps_intervel;
        break;

    case TBL_APS_GROUP_INFO_FLD_FAST_APS_EN:
        p_db_aps_group_info->fast_aps_en = p_aps_group_info->fast_aps_en;
        break;

    case TBL_APS_GROUP_INFO_FLD_APS_CHANNEL:
        p_db_aps_group_info->aps_channel = p_aps_group_info->aps_channel;
        break;

    case TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL:
        p_db_aps_group_info->requested_signal = p_aps_group_info->requested_signal;
        break;

    case TBL_APS_GROUP_INFO_FLD_DIRECTION:
        p_db_aps_group_info->direction = p_aps_group_info->direction;
        break;

    case TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE:
        p_db_aps_group_info->bridge_type = p_aps_group_info->bridge_type;
        break;

    case TBL_APS_GROUP_INFO_FLD_LAST_STATE:
        p_db_aps_group_info->last_state = p_aps_group_info->last_state;
        break;

    case TBL_APS_GROUP_INFO_FLD_CURRENT_STATE:
        p_db_aps_group_info->current_state = p_aps_group_info->current_state;
        break;

    case TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT:
        p_db_aps_group_info->far_end_last_event = p_aps_group_info->far_end_last_event;
        break;

    case TBL_APS_GROUP_INFO_FLD_WTR_TIMER:
        p_db_aps_group_info->wtr_timer = p_aps_group_info->wtr_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER:
        p_db_aps_group_info->sf_hold_off_timer = p_aps_group_info->sf_hold_off_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER:
        p_db_aps_group_info->sfp_hold_off_timer = p_aps_group_info->sfp_hold_off_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER:
        p_db_aps_group_info->sd_hold_off_timer = p_aps_group_info->sd_hold_off_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER:
        p_db_aps_group_info->sdp_hold_off_timer = p_aps_group_info->sdp_hold_off_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER:
        p_db_aps_group_info->sc_out_aps_timer = p_aps_group_info->sc_out_aps_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER:
        p_db_aps_group_info->aps_tx_timer = p_aps_group_info->aps_tx_timer;
        break;

    case TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT:
        p_db_aps_group_info->sc_aps_out_pdu_count = p_aps_group_info->sc_aps_out_pdu_count;
        break;

    case TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED:
        p_db_aps_group_info->hold_off_expired = p_aps_group_info->hold_off_expired;
        break;

    case TBL_APS_GROUP_INFO_FLD_MAX:
        sal_memcpy(p_db_aps_group_info, p_aps_group_info, sizeof(tbl_aps_group_info_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_APS_GROUP_INFO, field_id, p_db_aps_group_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_APS_GROUP_INFO, field_id, p_db_aps_group_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aps_group_info_add_aps_group_info(tbl_aps_group_info_t *p_aps_group_info)
{
    return tbl_aps_group_info_add_aps_group_info_sync(p_aps_group_info, TRUE);
}

int32
tbl_aps_group_info_del_aps_group_info(tbl_aps_group_info_key_t *p_aps_group_info_key)
{
    return tbl_aps_group_info_del_aps_group_info_sync(p_aps_group_info_key, TRUE);
}

int32
tbl_aps_group_info_set_aps_group_info_field(tbl_aps_group_info_t *p_aps_group_info, tbl_aps_group_info_field_id_t field_id)
{
    return tbl_aps_group_info_set_aps_group_info_field_sync(p_aps_group_info, field_id, TRUE);
}

tbl_aps_group_info_t*
tbl_aps_group_info_get_aps_group_info(tbl_aps_group_info_key_t *p_aps_group_info_key)
{
    tbl_aps_group_info_master_t *p_master = _g_p_tbl_aps_group_info_master;
    tbl_aps_group_info_t lkp;

    sal_memcpy(&lkp.key, p_aps_group_info_key, sizeof(tbl_aps_group_info_key_t));
    return ctclib_hash_lookup(p_master->aps_group_info_hash, &lkp);
}

char*
tbl_aps_group_info_key_val2str(tbl_aps_group_info_t *p_aps_group_info, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_aps_group_info->key.aps_group_id);
    return str;
}

int32
tbl_aps_group_info_key_str2val(char *str, tbl_aps_group_info_t *p_aps_group_info)
{
    int32 ret = 0;

    p_aps_group_info->key.aps_group_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_aps_group_info_key_name_dump(tbl_aps_group_info_t *p_aps_group_info, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_APS_GROUP_INFO);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_aps_group_info->key.aps_group_id);
    return str;
}

char*
tbl_aps_group_info_key_value_dump(tbl_aps_group_info_t *p_aps_group_info, char *str)
{
    sal_sprintf(str, "%u", p_aps_group_info->key.aps_group_id);
    return str;
}

char*
tbl_aps_group_info_field_name_dump(tbl_aps_group_info_t *p_aps_group_info, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_APS_GROUP_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_APS_GROUP_INFO_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_aps_group_info_field_value_dump(tbl_aps_group_info_t *p_aps_group_info, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->key.aps_group_id);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WORKING_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_aps_group_info->working_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_aps_group_info->protection_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SELECT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_aps_group_info->select_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_aps_group_info->aps_group_type);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->wait_to_restore_time);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->hold_off_time);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->is_revertive);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->active_path);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->bridge_permanent);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->current_event);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DFOP_REASON, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->dfop_reason);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->sc_aps_intervel);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAST_APS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->fast_aps_en);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_CHANNEL, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->aps_channel);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->requested_signal);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DIRECTION, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->direction);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->bridge_type);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_LAST_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->last_state);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->current_state);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->far_end_last_event);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WTR_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->wtr_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->sf_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->sfp_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->sd_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->sdp_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->sc_out_aps_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_aps_group_info->aps_tx_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->sc_aps_out_pdu_count);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED, field_id))
    {
        sal_sprintf(str, "%u", p_aps_group_info->hold_off_expired);
    }
    return str;
}

char**
tbl_aps_group_info_table_dump(tbl_aps_group_info_t *p_aps_group_info, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_aps_group_info_key_name_dump(p_aps_group_info, buf));
    for(i=1; i<TBL_APS_GROUP_INFO_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_aps_group_info_field_name_dump(p_aps_group_info, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_aps_group_info_field_value_dump(p_aps_group_info, i, buf));
    }
    return str;
}

int32
tbl_aps_group_info_field_value_parser(char *str, int32 field_id, tbl_aps_group_info_t *p_aps_group_info)
{
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_KEY, field_id))
    {
        int32 ret;
        p_aps_group_info->key.aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WORKING_NAME, field_id))
    {
        sal_strcpy(p_aps_group_info->working_name, str);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME, field_id))
    {
        sal_strcpy(p_aps_group_info->protection_name, str);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SELECT_NAME, field_id))
    {
        sal_strcpy(p_aps_group_info->select_name, str);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME, field_id))
    {
        int32 ret;
        p_aps_group_info->wait_to_restore_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME, field_id))
    {
        int32 ret;
        p_aps_group_info->hold_off_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE, field_id))
    {
        int32 ret;
        p_aps_group_info->is_revertive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH, field_id))
    {
        int32 ret;
        p_aps_group_info->active_path = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT, field_id))
    {
        int32 ret;
        p_aps_group_info->bridge_permanent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT, field_id))
    {
        int32 ret;
        p_aps_group_info->current_event = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DFOP_REASON, field_id))
    {
        int32 ret;
        p_aps_group_info->dfop_reason = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL, field_id))
    {
        int32 ret;
        p_aps_group_info->sc_aps_intervel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAST_APS_EN, field_id))
    {
        int32 ret;
        p_aps_group_info->fast_aps_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_CHANNEL, field_id))
    {
        int32 ret;
        p_aps_group_info->aps_channel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL, field_id))
    {
        int32 ret;
        p_aps_group_info->requested_signal = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DIRECTION, field_id))
    {
        int32 ret;
        p_aps_group_info->direction = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE, field_id))
    {
        int32 ret;
        p_aps_group_info->bridge_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_LAST_STATE, field_id))
    {
        int32 ret;
        p_aps_group_info->last_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_STATE, field_id))
    {
        int32 ret;
        p_aps_group_info->current_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT, field_id))
    {
        int32 ret;
        p_aps_group_info->far_end_last_event = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WTR_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->wtr_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->sf_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->sfp_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->sd_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->sdp_hold_off_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->sc_out_aps_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER, field_id))
    {
        ctc_task_str2val(str, p_aps_group_info->aps_tx_timer);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT, field_id))
    {
        int32 ret;
        p_aps_group_info->sc_aps_out_pdu_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED, field_id))
    {
        int32 ret;
        p_aps_group_info->hold_off_expired = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_aps_group_info_table_parser(char** array, char* key_value,tbl_aps_group_info_t *p_aps_group_info)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_APS_GROUP_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_aps_group_info_key_str2val(key_value, p_aps_group_info));

    for(i=1; i<TBL_APS_GROUP_INFO_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_aps_group_info_field_value_parser( array[j++], i, p_aps_group_info));
    }

    return PM_E_NONE;
}

int32
tbl_aps_group_info_dump_one(tbl_aps_group_info_t *p_aps_group_info, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_APS_GROUP_INFO);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_KEY].name,
            p_aps_group_info->key.aps_group_id);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WORKING_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_WORKING_NAME].name,
            p_aps_group_info->working_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME].name,
            p_aps_group_info->protection_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SELECT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SELECT_NAME].name,
            p_aps_group_info->select_name);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE].name,
            p_aps_group_info->aps_group_type);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME].name,
            p_aps_group_info->wait_to_restore_time);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME].name,
            p_aps_group_info->hold_off_time);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE].name,
            p_aps_group_info->is_revertive);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH].name,
            p_aps_group_info->active_path);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT].name,
            p_aps_group_info->bridge_permanent);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT].name,
            p_aps_group_info->current_event);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DFOP_REASON, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_DFOP_REASON].name,
            p_aps_group_info->dfop_reason);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL].name,
            p_aps_group_info->sc_aps_intervel);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAST_APS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_FAST_APS_EN].name,
            p_aps_group_info->fast_aps_en);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_CHANNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_APS_CHANNEL].name,
            p_aps_group_info->aps_channel);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL].name,
            p_aps_group_info->requested_signal);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_DIRECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_DIRECTION].name,
            p_aps_group_info->direction);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE].name,
            p_aps_group_info->bridge_type);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_LAST_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_LAST_STATE].name,
            p_aps_group_info->last_state);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_CURRENT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_CURRENT_STATE].name,
            p_aps_group_info->current_state);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT].name,
            p_aps_group_info->far_end_last_event);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_WTR_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_WTR_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->wtr_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->sf_hold_off_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->sfp_hold_off_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->sd_hold_off_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->sdp_hold_off_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->sc_out_aps_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_aps_group_info->aps_tx_timer));
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT].name,
            p_aps_group_info->sc_aps_out_pdu_count);
    }
    if (FLD_MATCH(TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED].name,
            p_aps_group_info->hold_off_expired);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_aps_group_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_aps_group_info_master_t *p_master = _g_p_tbl_aps_group_info_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_aps_group_info_t *p_db_aps_group_info = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->aps_group_info_list, p_db_aps_group_info, listnode, next)
    {
        rc |= fn(p_db_aps_group_info, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_aps_group_info_master_t*
tbl_aps_group_info_get_master()
{
    return _g_p_tbl_aps_group_info_master;
}

tbl_aps_group_info_master_t*
tbl_aps_group_info_init_aps_group_info()
{
    _g_p_tbl_aps_group_info_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_aps_group_info_master_t));
    _g_p_tbl_aps_group_info_master->aps_group_info_hash = ctclib_hash_create(_tbl_aps_group_info_hash_make, _tbl_aps_group_info_hash_cmp);
    _g_p_tbl_aps_group_info_master->aps_group_info_list = ctclib_slist_create(tbl_aps_group_info_cmp, NULL);
    return _g_p_tbl_aps_group_info_master;
}


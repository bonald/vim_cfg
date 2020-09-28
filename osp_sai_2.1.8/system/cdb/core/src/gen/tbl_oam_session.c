
#include "proto.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "cdb_data_cmp.h"

tbl_oam_session_master_t *_g_p_tbl_oam_session_master = NULL;

static uint32
_tbl_oam_session_hash_make(tbl_oam_session_t *p_oam_session)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_oam_session->key;
    for (index = 0; index < sizeof(p_oam_session->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_oam_session_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_oam_session_t *p_oam_session1 = (tbl_oam_session_t*)p_arg1;
    tbl_oam_session_t *p_oam_session2 = (tbl_oam_session_t*)p_arg2;

    if (0 == sal_memcmp(&p_oam_session1->key, &p_oam_session2->key, sizeof(tbl_oam_session_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_oam_session_add_oam_session_sync(tbl_oam_session_t *p_oam_session, uint32 sync)
{
    tbl_oam_session_master_t *p_master = _g_p_tbl_oam_session_master;
    tbl_oam_session_t *p_db_oam_session = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_oam_session_get_oam_session(&p_oam_session->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_oam_session = XCALLOC(MEM_TBL_OAM_SESSION, sizeof(tbl_oam_session_t));
    if (NULL == p_db_oam_session)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_oam_session, p_oam_session, sizeof(tbl_oam_session_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->oam_session_hash, (void*)p_db_oam_session, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->oam_session_list, p_db_oam_session);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OAM_SESSION, p_db_oam_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OAM_SESSION, p_db_oam_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_del_oam_session_sync(tbl_oam_session_key_t *p_oam_session_key, uint32 sync)
{
    tbl_oam_session_master_t *p_master = _g_p_tbl_oam_session_master;
    tbl_oam_session_t *p_db_oam_session = NULL;

    /* 1. lookup entry exist */
    p_db_oam_session = tbl_oam_session_get_oam_session(p_oam_session_key);
    if (NULL == p_db_oam_session)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OAM_SESSION, p_db_oam_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OAM_SESSION, p_db_oam_session);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->oam_session_hash, (void*)p_db_oam_session);
    ctclib_slistnode_delete(p_master->oam_session_list, p_db_oam_session);

    /* 4. free db entry */
    XFREE(MEM_TBL_OAM_SESSION, p_db_oam_session);

    return PM_E_NONE;
}

int32
tbl_oam_session_set_oam_session_field_sync(tbl_oam_session_t *p_oam_session, tbl_oam_session_field_id_t field_id, uint32 sync)
{
    tbl_oam_session_t *p_db_oam_session = NULL;

    /* 1. lookup entry exist */
    p_db_oam_session = tbl_oam_session_get_oam_session(&p_oam_session->key);
    if (NULL == p_db_oam_session)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OAM_SESSION_FLD_CREAT_BY_FLOW:
        p_db_oam_session->creat_by_flow = p_oam_session->creat_by_flow;
        break;

    case TBL_OAM_SESSION_FLD_OAM_TYPE:
        p_db_oam_session->oam_type = p_oam_session->oam_type;
        break;

    case TBL_OAM_SESSION_FLD_PW_TYPE:
        p_db_oam_session->pw_type = p_oam_session->pw_type;
        break;

    case TBL_OAM_SESSION_FLD_MP_TYPE:
        p_db_oam_session->mp_type = p_oam_session->mp_type;
        break;

    case TBL_OAM_SESSION_FLD_SERVICE_IFINDEX:
        p_db_oam_session->service_ifindex = p_oam_session->service_ifindex;
        break;

    case TBL_OAM_SESSION_FLD_SERVICE_NAME:
        sal_memcpy(p_db_oam_session->service_name, p_oam_session->service_name, sizeof(p_oam_session->service_name));
        break;

    case TBL_OAM_SESSION_FLD_MEGID_STR:
        sal_memcpy(p_db_oam_session->megid_str, p_oam_session->megid_str, sizeof(p_oam_session->megid_str));
        break;

    case TBL_OAM_SESSION_FLD_MEPID:
        p_db_oam_session->mepid = p_oam_session->mepid;
        break;

    case TBL_OAM_SESSION_FLD_RMEPID:
        p_db_oam_session->rmepid = p_oam_session->rmepid;
        break;

    case TBL_OAM_SESSION_FLD_CC_ENABLE:
        p_db_oam_session->cc_enable = p_oam_session->cc_enable;
        break;

    case TBL_OAM_SESSION_FLD_CC_INTERVAL:
        p_db_oam_session->cc_interval = p_oam_session->cc_interval;
        break;

    case TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG:
        p_db_oam_session->cc_interval_cfg = p_oam_session->cc_interval_cfg;
        break;

    case TBL_OAM_SESSION_FLD_LM_ENABLE:
        p_db_oam_session->lm_enable = p_oam_session->lm_enable;
        break;

    case TBL_OAM_SESSION_FLD_DM_ENABLE:
        p_db_oam_session->dm_enable = p_oam_session->dm_enable;
        break;

    case TBL_OAM_SESSION_FLD_EXP:
        p_db_oam_session->exp = p_oam_session->exp;
        break;

    case TBL_OAM_SESSION_FLD_LEVEL:
        p_db_oam_session->level = p_oam_session->level;
        break;

    case TBL_OAM_SESSION_FLD_TTL:
        p_db_oam_session->ttl = p_oam_session->ttl;
        break;

    case TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID:
        p_db_oam_session->bind_oam_flow_id = p_oam_session->bind_oam_flow_id;
        break;

    case TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX:
        p_db_oam_session->bind_oam_ifindex = p_oam_session->bind_oam_ifindex;
        break;

    case TBL_OAM_SESSION_FLD_ICCLEN:
        p_db_oam_session->icclen = p_oam_session->icclen;
        break;

    case TBL_OAM_SESSION_FLD_DESCRIPTION:
        sal_memcpy(p_db_oam_session->description, p_oam_session->description, sizeof(p_oam_session->description));
        break;

    case TBL_OAM_SESSION_FLD_OAM_DM:
        sal_memcpy(&p_db_oam_session->oam_dm, &p_oam_session->oam_dm, sizeof(p_oam_session->oam_dm));
        break;

    case TBL_OAM_SESSION_FLD_OAM_LM:
        sal_memcpy(&p_db_oam_session->oam_lm, &p_oam_session->oam_lm, sizeof(p_oam_session->oam_lm));
        break;

    case TBL_OAM_SESSION_FLD_OAM_LB:
        sal_memcpy(&p_db_oam_session->oam_lb, &p_oam_session->oam_lb, sizeof(p_oam_session->oam_lb));
        break;

    case TBL_OAM_SESSION_FLD_OAM_EVENT:
        sal_memcpy(&p_db_oam_session->oam_event, &p_oam_session->oam_event, sizeof(p_oam_session->oam_event));
        break;

    case TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER:
        p_db_oam_session->p_t_send_dm_timer = p_oam_session->p_t_send_dm_timer;
        break;

    case TBL_OAM_SESSION_FLD_DM_ACTIVE:
        p_db_oam_session->dm_active = p_oam_session->dm_active;
        break;

    case TBL_OAM_SESSION_FLD_OAM_STATUS:
        p_db_oam_session->oam_status = p_oam_session->oam_status;
        break;

    case TBL_OAM_SESSION_FLD_SIGNAL_STATUS:
        p_db_oam_session->signal_status = p_oam_session->signal_status;
        break;

    case TBL_OAM_SESSION_FLD_OAM_STATS:
        sal_memcpy(&p_db_oam_session->oam_stats, &p_oam_session->oam_stats, sizeof(p_oam_session->oam_stats));
        break;

    case TBL_OAM_SESSION_FLD_OAM_LIST:
        sal_memcpy(&p_db_oam_session->oam_list, &p_oam_session->oam_list, sizeof(p_oam_session->oam_list));
        break;

    case TBL_OAM_SESSION_FLD_APS_GROUP_ID:
        p_db_oam_session->aps_group_id = p_oam_session->aps_group_id;
        break;

    case TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW:
        p_db_oam_session->flag_add_flow = p_oam_session->flag_add_flow;
        break;

    case TBL_OAM_SESSION_FLD_MAX:
        sal_memcpy(p_db_oam_session, p_oam_session, sizeof(tbl_oam_session_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OAM_SESSION, field_id, p_db_oam_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OAM_SESSION, field_id, p_db_oam_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_add_oam_session(tbl_oam_session_t *p_oam_session)
{
    return tbl_oam_session_add_oam_session_sync(p_oam_session, TRUE);
}

int32
tbl_oam_session_del_oam_session(tbl_oam_session_key_t *p_oam_session_key)
{
    return tbl_oam_session_del_oam_session_sync(p_oam_session_key, TRUE);
}

int32
tbl_oam_session_set_oam_session_field(tbl_oam_session_t *p_oam_session, tbl_oam_session_field_id_t field_id)
{
    return tbl_oam_session_set_oam_session_field_sync(p_oam_session, field_id, TRUE);
}

tbl_oam_session_t*
tbl_oam_session_get_oam_session(tbl_oam_session_key_t *p_oam_session_key)
{
    tbl_oam_session_master_t *p_master = _g_p_tbl_oam_session_master;
    tbl_oam_session_t lkp;

    sal_memcpy(&lkp.key, p_oam_session_key, sizeof(tbl_oam_session_key_t));
    return ctclib_hash_lookup(p_master->oam_session_hash, &lkp);
}

char*
tbl_oam_session_key_val2str(tbl_oam_session_t *p_oam_session, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_oam_session->key.session_id);
    return str;
}

int32
tbl_oam_session_key_str2val(char *str, tbl_oam_session_t *p_oam_session)
{
    int32 ret = 0;

    p_oam_session->key.session_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_oam_session_key_name_dump(tbl_oam_session_t *p_oam_session, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_oam_session->key.session_id);
    return str;
}

char*
tbl_oam_session_key_value_dump(tbl_oam_session_t *p_oam_session, char *str)
{
    sal_sprintf(str, "%u", p_oam_session->key.session_id);
    return str;
}

char*
tbl_oam_session_field_name_dump(tbl_oam_session_t *p_oam_session, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OAM_SESSION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_oam_session_field_value_dump(tbl_oam_session_t *p_oam_session, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->key.session_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CREAT_BY_FLOW, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->creat_by_flow);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->oam_type);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_PW_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(tpoam_pw_type_strs, TPOAM_PW_TYPE_MAX, p_oam_session->pw_type));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MP_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(tpoam_mp_type_strs, TPOAM_MP_TYPE_MAX, p_oam_session->mp_type));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->service_ifindex);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_oam_session->service_name);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEGID_STR, field_id))
    {
        sal_sprintf(str, "%s", p_oam_session->megid_str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEPID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->mepid);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_RMEPID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->rmepid);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->cc_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->cc_interval);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->cc_interval_cfg);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LM_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->lm_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->dm_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->exp);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->level);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->ttl);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->bind_oam_flow_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->bind_oam_ifindex);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_ICCLEN, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->icclen);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DESCRIPTION, field_id))
    {
        sal_sprintf(str, "%s", p_oam_session->description);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_DM, field_id))
    {
        cdb_tpoam_dm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_dm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LM, field_id))
    {
        cdb_tpoam_lm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_lm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LB, field_id))
    {
        cdb_tpoam_lb_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_lb);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_EVENT, field_id))
    {
        cdb_tpoam_event_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_oam_session->p_t_send_dm_timer);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ACTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->dm_active);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->oam_status);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SIGNAL_STATUS, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(tpoam_status_strs, TPOAM_STATUS_MAX, p_oam_session->signal_status));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATS, field_id))
    {
        cdb_tpoam_stats_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_stats);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LIST, field_id))
    {
        cdb_tpoam_list_val2str(str, MAX_CMD_STR_LEN, &p_oam_session->oam_list);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_APS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->aps_group_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session->flag_add_flow);
    }
    return str;
}

char**
tbl_oam_session_table_dump(tbl_oam_session_t *p_oam_session, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_oam_session_key_name_dump(p_oam_session, buf));
    for(i=1; i<TBL_OAM_SESSION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_oam_session_field_name_dump(p_oam_session, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_oam_session_field_value_dump(p_oam_session, i, buf));
    }
    return str;
}

int32
tbl_oam_session_field_value_parser(char *str, int32 field_id, tbl_oam_session_t *p_oam_session)
{
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_KEY, field_id))
    {
        int32 ret;
        p_oam_session->key.session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CREAT_BY_FLOW, field_id))
    {
        int32 ret;
        p_oam_session->creat_by_flow = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_TYPE, field_id))
    {
        int32 ret;
        p_oam_session->oam_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_PW_TYPE, field_id))
    {
        cdb_enum_str2val(tpoam_pw_type_strs, TPOAM_PW_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MP_TYPE, field_id))
    {
        cdb_enum_str2val(tpoam_mp_type_strs, TPOAM_MP_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_IFINDEX, field_id))
    {
        int32 ret;
        p_oam_session->service_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_NAME, field_id))
    {
        sal_strcpy(p_oam_session->service_name, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEGID_STR, field_id))
    {
        sal_strcpy(p_oam_session->megid_str, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEPID, field_id))
    {
        int32 ret;
        p_oam_session->mepid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_RMEPID, field_id))
    {
        int32 ret;
        p_oam_session->rmepid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_ENABLE, field_id))
    {
        int32 ret;
        p_oam_session->cc_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL, field_id))
    {
        int32 ret;
        p_oam_session->cc_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG, field_id))
    {
        int32 ret;
        p_oam_session->cc_interval_cfg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LM_ENABLE, field_id))
    {
        int32 ret;
        p_oam_session->lm_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ENABLE, field_id))
    {
        int32 ret;
        p_oam_session->dm_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_EXP, field_id))
    {
        int32 ret;
        p_oam_session->exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LEVEL, field_id))
    {
        int32 ret;
        p_oam_session->level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_TTL, field_id))
    {
        int32 ret;
        p_oam_session->ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID, field_id))
    {
        int32 ret;
        p_oam_session->bind_oam_flow_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX, field_id))
    {
        int32 ret;
        p_oam_session->bind_oam_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_ICCLEN, field_id))
    {
        int32 ret;
        p_oam_session->icclen = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DESCRIPTION, field_id))
    {
        sal_strcpy(p_oam_session->description, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_DM, field_id))
    {
        cdb_tpoam_dm_str2val(str, &p_oam_session->oam_dm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LM, field_id))
    {
        cdb_tpoam_lm_str2val(str, &p_oam_session->oam_lm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LB, field_id))
    {
        cdb_tpoam_lb_str2val(str, &p_oam_session->oam_lb);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_EVENT, field_id))
    {
        cdb_tpoam_event_str2val(str, &p_oam_session->oam_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER, field_id))
    {
        ctc_task_str2val(str, p_oam_session->p_t_send_dm_timer);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ACTIVE, field_id))
    {
        int32 ret;
        p_oam_session->dm_active = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATUS, field_id))
    {
        int32 ret;
        p_oam_session->oam_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SIGNAL_STATUS, field_id))
    {
        cdb_enum_str2val(tpoam_status_strs, TPOAM_STATUS_MAX, str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATS, field_id))
    {
        cdb_tpoam_stats_str2val(str, &p_oam_session->oam_stats);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LIST, field_id))
    {
        cdb_tpoam_list_str2val(str, &p_oam_session->oam_list);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_APS_GROUP_ID, field_id))
    {
        int32 ret;
        p_oam_session->aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW, field_id))
    {
        int32 ret;
        p_oam_session->flag_add_flow = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_oam_session_table_parser(char** array, char* key_value,tbl_oam_session_t *p_oam_session)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_oam_session_key_str2val(key_value, p_oam_session));

    for(i=1; i<TBL_OAM_SESSION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_oam_session_field_value_parser( array[j++], i, p_oam_session));
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_dump_one(tbl_oam_session_t *p_oam_session, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OAM_SESSION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_KEY].name,
            p_oam_session->key.session_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CREAT_BY_FLOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_CREAT_BY_FLOW].name,
            p_oam_session->creat_by_flow);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_TYPE].name,
            p_oam_session->oam_type);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_PW_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_PW_TYPE].name, 
            cdb_enum_val2str(tpoam_pw_type_strs, TPOAM_PW_TYPE_MAX, p_oam_session->pw_type));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_MP_TYPE].name, 
            cdb_enum_val2str(tpoam_mp_type_strs, TPOAM_MP_TYPE_MAX, p_oam_session->mp_type));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_SERVICE_IFINDEX].name,
            p_oam_session->service_ifindex);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SERVICE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_SERVICE_NAME].name,
            p_oam_session->service_name);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEGID_STR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_MEGID_STR].name,
            p_oam_session->megid_str);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_MEPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_MEPID].name,
            p_oam_session->mepid);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_RMEPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_RMEPID].name,
            p_oam_session->rmepid);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_CC_ENABLE].name,
            p_oam_session->cc_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_CC_INTERVAL].name,
            p_oam_session->cc_interval);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG].name,
            p_oam_session->cc_interval_cfg);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LM_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_LM_ENABLE].name,
            p_oam_session->lm_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_DM_ENABLE].name,
            p_oam_session->dm_enable);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_EXP].name,
            p_oam_session->exp);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_LEVEL].name,
            p_oam_session->level);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_TTL].name,
            p_oam_session->ttl);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID].name,
            p_oam_session->bind_oam_flow_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX].name,
            p_oam_session->bind_oam_ifindex);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_ICCLEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_ICCLEN].name,
            p_oam_session->icclen);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DESCRIPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_DESCRIPTION].name,
            p_oam_session->description);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_DM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_DM].name, 
            cdb_tpoam_dm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_dm));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_LM].name, 
            cdb_tpoam_lm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_lm));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_LB].name, 
            cdb_tpoam_lb_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_lb));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_EVENT].name, 
            cdb_tpoam_event_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_event));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_oam_session->p_t_send_dm_timer));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_DM_ACTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_DM_ACTIVE].name,
            p_oam_session->dm_active);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_STATUS].name,
            p_oam_session->oam_status);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_SIGNAL_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_SIGNAL_STATUS].name, 
            cdb_enum_val2str(tpoam_status_strs, TPOAM_STATUS_MAX, p_oam_session->signal_status));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_STATS].name, 
            cdb_tpoam_stats_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_stats));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_OAM_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_FLD_OAM_LIST].name, 
            cdb_tpoam_list_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session->oam_list));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_APS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_APS_GROUP_ID].name,
            p_oam_session->aps_group_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW].name,
            p_oam_session->flag_add_flow);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_oam_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_oam_session_master_t *p_master = _g_p_tbl_oam_session_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_oam_session_t *p_db_oam_session = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->oam_session_list, p_db_oam_session, listnode, next)
    {
        rc |= fn(p_db_oam_session, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_oam_session_master_t*
tbl_oam_session_get_master()
{
    return _g_p_tbl_oam_session_master;
}

tbl_oam_session_master_t*
tbl_oam_session_init_oam_session()
{
    _g_p_tbl_oam_session_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_oam_session_master_t));
    _g_p_tbl_oam_session_master->oam_session_hash = ctclib_hash_create(_tbl_oam_session_hash_make, _tbl_oam_session_hash_cmp);
    _g_p_tbl_oam_session_master->oam_session_list = ctclib_slist_create(tbl_oam_session_cmp, NULL);
    return _g_p_tbl_oam_session_master;
}


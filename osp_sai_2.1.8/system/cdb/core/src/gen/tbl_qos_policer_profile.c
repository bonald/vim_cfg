
#include "proto.h"
#include "gen/tbl_qos_policer_profile_define.h"
#include "gen/tbl_qos_policer_profile.h"
#include "cdb_data_cmp.h"

tbl_qos_policer_profile_master_t *_g_p_tbl_qos_policer_profile_master = NULL;

int32
tbl_qos_policer_profile_add_qos_policer_profile_sync(tbl_qos_policer_profile_t *p_qos_policer_profile, uint32 sync)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_policer_profile_get_qos_policer_profile(&p_qos_policer_profile->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_policer_profile = XCALLOC(MEM_TBL_QOS_POLICER_PROFILE, sizeof(tbl_qos_policer_profile_t));
    if (NULL == p_db_qos_policer_profile)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_policer_profile, p_qos_policer_profile, sizeof(tbl_qos_policer_profile_t));

    /* 4. add to db */
    p_master->qos_policer_profile_array[p_qos_policer_profile->key.id] = p_db_qos_policer_profile;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_POLICER_PROFILE, p_db_qos_policer_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_POLICER_PROFILE, p_db_qos_policer_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_del_qos_policer_profile_sync(tbl_qos_policer_profile_key_t *p_qos_policer_profile_key, uint32 sync)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;

    /* 1. lookup entry exist */
    p_db_qos_policer_profile = tbl_qos_policer_profile_get_qos_policer_profile(p_qos_policer_profile_key);
    if (NULL == p_db_qos_policer_profile)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_POLICER_PROFILE, p_db_qos_policer_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_POLICER_PROFILE, p_db_qos_policer_profile);
        #endif 
    }

    /* 3. delete from db */
    p_master->qos_policer_profile_array[p_qos_policer_profile_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_POLICER_PROFILE, p_db_qos_policer_profile);

    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_set_qos_policer_profile_field_sync(tbl_qos_policer_profile_t *p_qos_policer_profile, tbl_qos_policer_profile_field_id_t field_id, uint32 sync)
{
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;

    /* 1. lookup entry exist */
    p_db_qos_policer_profile = tbl_qos_policer_profile_get_qos_policer_profile(&p_qos_policer_profile->key);
    if (NULL == p_db_qos_policer_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_POLICER_PROFILE_FLD_NAME:
        sal_memcpy(p_db_qos_policer_profile->name, p_qos_policer_profile->name, sizeof(p_qos_policer_profile->name));
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_AGP)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_CIR)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_CBS)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_STATS)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS:
        CDB_FLAG_CHECK_SET(p_db_qos_policer_profile, p_qos_policer_profile, flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS)
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE:
        p_db_qos_policer_profile->algorithm_mode = p_qos_policer_profile->algorithm_mode;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE:
        p_db_qos_policer_profile->color_mode = p_qos_policer_profile->color_mode;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR:
        p_db_qos_policer_profile->drop_color = p_qos_policer_profile->drop_color;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_CIR:
        p_db_qos_policer_profile->cir = p_qos_policer_profile->cir;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_CBS:
        p_db_qos_policer_profile->cbs = p_qos_policer_profile->cbs;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR:
        p_db_qos_policer_profile->eir_or_pir = p_qos_policer_profile->eir_or_pir;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS:
        p_db_qos_policer_profile->ebs_or_pbs = p_qos_policer_profile->ebs_or_pbs;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT:
        p_db_qos_policer_profile->confirm_pkt = p_qos_policer_profile->confirm_pkt;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE:
        p_db_qos_policer_profile->confirm_byte = p_qos_policer_profile->confirm_byte;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT:
        p_db_qos_policer_profile->exceed_pkt = p_qos_policer_profile->exceed_pkt;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE:
        p_db_qos_policer_profile->exceed_byte = p_qos_policer_profile->exceed_byte;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT:
        p_db_qos_policer_profile->violate_pkt = p_qos_policer_profile->violate_pkt;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE:
        p_db_qos_policer_profile->violate_byte = p_qos_policer_profile->violate_byte;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_IS_PPS:
        p_db_qos_policer_profile->is_pps = p_qos_policer_profile->is_pps;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME:
        sal_memcpy(p_db_qos_policer_profile->action_profile_name, p_qos_policer_profile->action_profile_name, sizeof(p_qos_policer_profile->action_profile_name));
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_REF_CNT:
        p_db_qos_policer_profile->ref_cnt = p_qos_policer_profile->ref_cnt;
        break;

    case TBL_QOS_POLICER_PROFILE_FLD_MAX:
        sal_memcpy(p_db_qos_policer_profile, p_qos_policer_profile, sizeof(tbl_qos_policer_profile_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_POLICER_PROFILE, field_id, p_db_qos_policer_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_POLICER_PROFILE, field_id, p_db_qos_policer_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_add_qos_policer_profile(tbl_qos_policer_profile_t *p_qos_policer_profile)
{
    return tbl_qos_policer_profile_add_qos_policer_profile_sync(p_qos_policer_profile, TRUE);
}

int32
tbl_qos_policer_profile_del_qos_policer_profile(tbl_qos_policer_profile_key_t *p_qos_policer_profile_key)
{
    return tbl_qos_policer_profile_del_qos_policer_profile_sync(p_qos_policer_profile_key, TRUE);
}

int32
tbl_qos_policer_profile_set_qos_policer_profile_field(tbl_qos_policer_profile_t *p_qos_policer_profile, tbl_qos_policer_profile_field_id_t field_id)
{
    return tbl_qos_policer_profile_set_qos_policer_profile_field_sync(p_qos_policer_profile, field_id, TRUE);
}

tbl_qos_policer_profile_t*
tbl_qos_policer_profile_get_qos_policer_profile(tbl_qos_policer_profile_key_t *p_qos_policer_profile_key)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;

    if (GLB_QOS_POLICER_PROFILE_NUM <= p_qos_policer_profile_key->id)
    {
        return NULL;
    }
    return (p_master->qos_policer_profile_array[p_qos_policer_profile_key->id]);
}

char*
tbl_qos_policer_profile_key_val2str(tbl_qos_policer_profile_t *p_qos_policer_profile, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_policer_profile->key.id);
    return str;
}

int32
tbl_qos_policer_profile_key_str2val(char *str, tbl_qos_policer_profile_t *p_qos_policer_profile)
{
    int32 ret = 0;

    p_qos_policer_profile->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_qos_policer_profile_key_name_dump(tbl_qos_policer_profile_t *p_qos_policer_profile, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_PROFILE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_qos_policer_profile->key.id);
    return str;
}

char*
tbl_qos_policer_profile_key_value_dump(tbl_qos_policer_profile_t *p_qos_policer_profile, char *str)
{
    sal_sprintf(str, "%u", p_qos_policer_profile->key.id);
    return str;
}

char*
tbl_qos_policer_profile_field_name_dump(tbl_qos_policer_profile_t *p_qos_policer_profile, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_POLICER_PROFILE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_policer_profile_field_value_dump(tbl_qos_policer_profile_t *p_qos_policer_profile, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->key.id);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_qos_policer_profile->name);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CIR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CBS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->algorithm_mode);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->color_mode);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->drop_color);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CIR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->cir);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CBS, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->cbs);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->eir_or_pir);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->ebs_or_pbs);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->confirm_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->confirm_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->exceed_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->exceed_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->violate_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_qos_policer_profile->violate_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_IS_PPS, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->is_pps);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_qos_policer_profile->action_profile_name);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_profile->ref_cnt);
    }
    return str;
}

char**
tbl_qos_policer_profile_table_dump(tbl_qos_policer_profile_t *p_qos_policer_profile, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_policer_profile_key_name_dump(p_qos_policer_profile, buf));
    for(i=1; i<TBL_QOS_POLICER_PROFILE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_policer_profile_field_name_dump(p_qos_policer_profile, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_policer_profile_field_value_dump(p_qos_policer_profile, i, buf));
    }
    return str;
}

int32
tbl_qos_policer_profile_field_value_parser(char *str, int32 field_id, tbl_qos_policer_profile_t *p_qos_policer_profile)
{
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_KEY, field_id))
    {
        int32 ret;
        p_qos_policer_profile->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_NAME, field_id))
    {
        sal_strcpy(p_qos_policer_profile->name, str);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CIR);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CBS);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS, field_id))
    {
        GLB_SET_FLAG(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE, field_id))
    {
        int32 ret;
        p_qos_policer_profile->algorithm_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE, field_id))
    {
        int32 ret;
        p_qos_policer_profile->color_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR, field_id))
    {
        int32 ret;
        p_qos_policer_profile->drop_color = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CIR, field_id))
    {
        int32 ret;
        p_qos_policer_profile->cir = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CBS, field_id))
    {
        int32 ret;
        p_qos_policer_profile->cbs = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR, field_id))
    {
        int32 ret;
        p_qos_policer_profile->eir_or_pir = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS, field_id))
    {
        int32 ret;
        p_qos_policer_profile->ebs_or_pbs = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT, field_id))
    {
        int32 ret;
        p_qos_policer_profile->confirm_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE, field_id))
    {
        int32 ret;
        p_qos_policer_profile->confirm_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT, field_id))
    {
        int32 ret;
        p_qos_policer_profile->exceed_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE, field_id))
    {
        int32 ret;
        p_qos_policer_profile->exceed_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT, field_id))
    {
        int32 ret;
        p_qos_policer_profile->violate_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE, field_id))
    {
        int32 ret;
        p_qos_policer_profile->violate_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_IS_PPS, field_id))
    {
        int32 ret;
        p_qos_policer_profile->is_pps = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME, field_id))
    {
        sal_strcpy(p_qos_policer_profile->action_profile_name, str);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_qos_policer_profile->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_table_parser(char** array, char* key_value,tbl_qos_policer_profile_t *p_qos_policer_profile)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_policer_profile_key_str2val(key_value, p_qos_policer_profile));

    for(i=1; i<TBL_QOS_POLICER_PROFILE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_policer_profile_field_value_parser( array[j++], i, p_qos_policer_profile));
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_dump_one(tbl_qos_policer_profile_t *p_qos_policer_profile, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_PROFILE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_KEY].name,
            p_qos_policer_profile->key.id);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_NAME].name,
            p_qos_policer_profile->name);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_AGP));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_COLOR_MODE));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CIR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_CBS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DROP_COLOR));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS].name,
            GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_DUMP_STATS));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE].name,
            p_qos_policer_profile->algorithm_mode);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE].name,
            p_qos_policer_profile->color_mode);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR].name,
            p_qos_policer_profile->drop_color);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_CIR].name,
            p_qos_policer_profile->cir);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CBS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_CBS].name,
            p_qos_policer_profile->cbs);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR].name,
            p_qos_policer_profile->eir_or_pir);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS].name,
            p_qos_policer_profile->ebs_or_pbs);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT].name,
            p_qos_policer_profile->confirm_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE].name,
            p_qos_policer_profile->confirm_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT].name,
            p_qos_policer_profile->exceed_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE].name,
            p_qos_policer_profile->exceed_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT].name,
            p_qos_policer_profile->violate_pkt);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE].name,
            p_qos_policer_profile->violate_byte);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_IS_PPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_IS_PPS].name,
            p_qos_policer_profile->is_pps);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME].name,
            p_qos_policer_profile->action_profile_name);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_PROFILE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_PROFILE_FLD_REF_CNT].name,
            p_qos_policer_profile->ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_policer_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_QOS_POLICER_PROFILE_NUM; id++)
    {
        p_db_qos_policer_profile = p_master->qos_policer_profile_array[id];
        if (NULL == p_db_qos_policer_profile)
        {
            continue;
        }
        rc |= fn(p_db_qos_policer_profile, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_policer_profile_master_t*
tbl_qos_policer_profile_get_master()
{
    return _g_p_tbl_qos_policer_profile_master;
}

tbl_qos_policer_profile_master_t*
tbl_qos_policer_profile_init_qos_policer_profile()
{
    _g_p_tbl_qos_policer_profile_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_policer_profile_master_t));
    return _g_p_tbl_qos_policer_profile_master;
}

tbl_qos_policer_profile_t*
tbl_qos_policer_profile_get_profile_by_name(char *name)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    uint32 id = 0;

    for (id = 0; id < GLB_QOS_POLICER_PROFILE_NUM; id++)
    {
        p_db_qos_policer_profile = p_master->qos_policer_profile_array[id];
        if (NULL == p_db_qos_policer_profile)
        {
            continue;
        }

        if (0 == sal_strcmp(p_db_qos_policer_profile->name, name))
        {
            return p_db_qos_policer_profile;
        }
    }
    return NULL;
}
int32
tbl_qos_policer_profile_alloc_profile_id(uint32 *p_id)
{
    tbl_qos_policer_profile_master_t *p_master = _g_p_tbl_qos_policer_profile_master;
    uint32 i = 0;

    for (i = 0; i < GLB_QOS_POLICER_PROFILE_NUM; i++)
    {
        if (!p_master->qos_policer_profile_array[i])
        {
            *p_id = i;
            return PM_E_NONE;
        }
    }

    return PM_E_RESOURCE_FULL;
}



#include "proto.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "gen/tbl_qos_drop_profile.h"
#include "cdb_data_cmp.h"

tbl_qos_drop_profile_master_t *_g_p_tbl_qos_drop_profile_master = NULL;

int32
tbl_qos_drop_profile_add_qos_drop_profile_sync(tbl_qos_drop_profile_t *p_qos_drop_profile, uint32 sync)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_drop_profile_get_qos_drop_profile(&p_qos_drop_profile->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_drop_profile = XCALLOC(MEM_TBL_QOS_DROP_PROFILE, sizeof(tbl_qos_drop_profile_t));
    if (NULL == p_db_qos_drop_profile)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_drop_profile, p_qos_drop_profile, sizeof(tbl_qos_drop_profile_t));

    /* 4. add to db */
    p_master->qos_drop_profile_array[p_qos_drop_profile->key.id] = p_db_qos_drop_profile;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_DROP_PROFILE, p_db_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_DROP_PROFILE, p_db_qos_drop_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_del_qos_drop_profile_sync(tbl_qos_drop_profile_key_t *p_qos_drop_profile_key, uint32 sync)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    p_db_qos_drop_profile = tbl_qos_drop_profile_get_qos_drop_profile(p_qos_drop_profile_key);
    if (NULL == p_db_qos_drop_profile)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_DROP_PROFILE, p_db_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_DROP_PROFILE, p_db_qos_drop_profile);
        #endif 
    }

    /* 3. delete from db */
    p_master->qos_drop_profile_array[p_qos_drop_profile_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_DROP_PROFILE, p_db_qos_drop_profile);

    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_set_qos_drop_profile_field_sync(tbl_qos_drop_profile_t *p_qos_drop_profile, tbl_qos_drop_profile_field_id_t field_id, uint32 sync)
{
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    p_db_qos_drop_profile = tbl_qos_drop_profile_get_qos_drop_profile(&p_qos_drop_profile->key);
    if (NULL == p_db_qos_drop_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_DROP_PROFILE_FLD_NAME:
        sal_memcpy(p_db_qos_drop_profile->name, p_qos_drop_profile->name, sizeof(p_qos_drop_profile->name));
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_RED_PROB)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT:
        CDB_FLAG_CHECK_SET(p_db_qos_drop_profile, p_qos_drop_profile, flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT)
        break;

    case TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD:
        p_db_qos_drop_profile->green_max_threashold = p_qos_drop_profile->green_max_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD:
        p_db_qos_drop_profile->green_min_threashold = p_qos_drop_profile->green_min_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY:
        p_db_qos_drop_profile->green_probability = p_qos_drop_profile->green_probability;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD:
        p_db_qos_drop_profile->yellow_max_threashold = p_qos_drop_profile->yellow_max_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD:
        p_db_qos_drop_profile->yellow_min_threashold = p_qos_drop_profile->yellow_min_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY:
        p_db_qos_drop_profile->yellow_probability = p_qos_drop_profile->yellow_probability;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD:
        p_db_qos_drop_profile->red_max_threashold = p_qos_drop_profile->red_max_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD:
        p_db_qos_drop_profile->red_min_threashold = p_qos_drop_profile->red_min_threashold;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY:
        p_db_qos_drop_profile->red_probability = p_qos_drop_profile->red_probability;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_REF_CNT:
        p_db_qos_drop_profile->ref_cnt = p_qos_drop_profile->ref_cnt;
        break;

    case TBL_QOS_DROP_PROFILE_FLD_MAX:
        sal_memcpy(p_db_qos_drop_profile, p_qos_drop_profile, sizeof(tbl_qos_drop_profile_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_DROP_PROFILE, field_id, p_db_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_DROP_PROFILE, field_id, p_db_qos_drop_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_add_qos_drop_profile(tbl_qos_drop_profile_t *p_qos_drop_profile)
{
    return tbl_qos_drop_profile_add_qos_drop_profile_sync(p_qos_drop_profile, TRUE);
}

int32
tbl_qos_drop_profile_del_qos_drop_profile(tbl_qos_drop_profile_key_t *p_qos_drop_profile_key)
{
    return tbl_qos_drop_profile_del_qos_drop_profile_sync(p_qos_drop_profile_key, TRUE);
}

int32
tbl_qos_drop_profile_set_qos_drop_profile_field(tbl_qos_drop_profile_t *p_qos_drop_profile, tbl_qos_drop_profile_field_id_t field_id)
{
    return tbl_qos_drop_profile_set_qos_drop_profile_field_sync(p_qos_drop_profile, field_id, TRUE);
}

tbl_qos_drop_profile_t*
tbl_qos_drop_profile_get_qos_drop_profile(tbl_qos_drop_profile_key_t *p_qos_drop_profile_key)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;

    if (GLB_QOS_DROP_PROFILE_NUM <= p_qos_drop_profile_key->id)
    {
        return NULL;
    }
    return (p_master->qos_drop_profile_array[p_qos_drop_profile_key->id]);
}

char*
tbl_qos_drop_profile_key_val2str(tbl_qos_drop_profile_t *p_qos_drop_profile, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_drop_profile->key.id);
    return str;
}

int32
tbl_qos_drop_profile_key_str2val(char *str, tbl_qos_drop_profile_t *p_qos_drop_profile)
{
    int32 ret = 0;

    p_qos_drop_profile->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_qos_drop_profile_key_name_dump(tbl_qos_drop_profile_t *p_qos_drop_profile, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DROP_PROFILE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_qos_drop_profile->key.id);
    return str;
}

char*
tbl_qos_drop_profile_key_value_dump(tbl_qos_drop_profile_t *p_qos_drop_profile, char *str)
{
    sal_sprintf(str, "%u", p_qos_drop_profile->key.id);
    return str;
}

char*
tbl_qos_drop_profile_field_name_dump(tbl_qos_drop_profile_t *p_qos_drop_profile, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DROP_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_DROP_PROFILE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_drop_profile_field_value_dump(tbl_qos_drop_profile_t *p_qos_drop_profile, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->key.id);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_qos_drop_profile->name);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->green_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->green_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->green_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->yellow_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->yellow_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->yellow_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->red_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->red_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->red_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_drop_profile->ref_cnt);
    }
    return str;
}

char**
tbl_qos_drop_profile_table_dump(tbl_qos_drop_profile_t *p_qos_drop_profile, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_drop_profile_key_name_dump(p_qos_drop_profile, buf));
    for(i=1; i<TBL_QOS_DROP_PROFILE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_drop_profile_field_name_dump(p_qos_drop_profile, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_drop_profile_field_value_dump(p_qos_drop_profile, i, buf));
    }
    return str;
}

int32
tbl_qos_drop_profile_field_value_parser(char *str, int32 field_id, tbl_qos_drop_profile_t *p_qos_drop_profile)
{
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_KEY, field_id))
    {
        int32 ret;
        p_qos_drop_profile->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_NAME, field_id))
    {
        sal_strcpy(p_qos_drop_profile->name, str);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_PROB);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT, field_id))
    {
        GLB_SET_FLAG(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->green_max_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->green_min_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY, field_id))
    {
        int32 ret;
        p_qos_drop_profile->green_probability = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->yellow_max_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->yellow_min_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY, field_id))
    {
        int32 ret;
        p_qos_drop_profile->yellow_probability = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->red_max_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD, field_id))
    {
        int32 ret;
        p_qos_drop_profile->red_min_threashold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY, field_id))
    {
        int32 ret;
        p_qos_drop_profile->red_probability = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_qos_drop_profile->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_table_parser(char** array, char* key_value,tbl_qos_drop_profile_t *p_qos_drop_profile)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DROP_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_drop_profile_key_str2val(key_value, p_qos_drop_profile));

    for(i=1; i<TBL_QOS_DROP_PROFILE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_drop_profile_field_value_parser( array[j++], i, p_qos_drop_profile));
    }

    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_dump_one(tbl_qos_drop_profile_t *p_qos_drop_profile, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_DROP_PROFILE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_KEY].name,
            p_qos_drop_profile->key.id);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_NAME].name,
            p_qos_drop_profile->name);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_PROB));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT].name,
            GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT));
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD].name,
            p_qos_drop_profile->green_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD].name,
            p_qos_drop_profile->green_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY].name,
            p_qos_drop_profile->green_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD].name,
            p_qos_drop_profile->yellow_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD].name,
            p_qos_drop_profile->yellow_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY].name,
            p_qos_drop_profile->yellow_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD].name,
            p_qos_drop_profile->red_max_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD].name,
            p_qos_drop_profile->red_min_threashold);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY].name,
            p_qos_drop_profile->red_probability);
    }
    if (FLD_MATCH(TBL_QOS_DROP_PROFILE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_DROP_PROFILE_FLD_REF_CNT].name,
            p_qos_drop_profile->ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_drop_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_QOS_DROP_PROFILE_NUM; id++)
    {
        p_db_qos_drop_profile = p_master->qos_drop_profile_array[id];
        if (NULL == p_db_qos_drop_profile)
        {
            continue;
        }
        rc |= fn(p_db_qos_drop_profile, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_drop_profile_master_t*
tbl_qos_drop_profile_get_master()
{
    return _g_p_tbl_qos_drop_profile_master;
}

tbl_qos_drop_profile_master_t*
tbl_qos_drop_profile_init_qos_drop_profile()
{
    _g_p_tbl_qos_drop_profile_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_drop_profile_master_t));
    return _g_p_tbl_qos_drop_profile_master;
}

tbl_qos_drop_profile_t*
tbl_qos_drop_profile_get_profile_by_name(char *name)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    tbl_qos_drop_profile_t *p_db_qos_drop = NULL;
    uint32 id = 0;

    for (id = 0; id < GLB_QOS_DROP_PROFILE_NUM; id++)
    {
        p_db_qos_drop = p_master->qos_drop_profile_array[id];
        if (NULL == p_db_qos_drop)
        {
            continue;
        }
    
        if (0 == sal_strcmp(p_db_qos_drop->name, name))
        {
            return p_db_qos_drop;
        }
    }

    return NULL;
}

int32
tbl_qos_drop_profile_alloc_profile_id(uint32 *p_id)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    uint32 i = 0;

    for (i = 0; i < GLB_QOS_DROP_PROFILE_NUM; i++)
    {
        if (!p_master->qos_drop_profile_array[i])
        {
            *p_id = i;
            return PM_E_NONE;
        }
    }

    return PM_E_RESOURCE_FULL;
}


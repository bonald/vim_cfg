
#include "proto.h"
#include "gen/tbl_fea_qos_drop_profile_define.h"
#include "gen/tbl_fea_qos_drop_profile.h"
#include "cdb_data_cmp.h"

tbl_fea_qos_drop_profile_master_t *_g_p_tbl_fea_qos_drop_profile_master = NULL;

int32
tbl_fea_qos_drop_profile_add_fea_qos_drop_profile_sync(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, uint32 sync)
{
    tbl_fea_qos_drop_profile_master_t *p_master = _g_p_tbl_fea_qos_drop_profile_master;
    tbl_fea_qos_drop_profile_t *p_db_fea_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(&p_fea_qos_drop_profile->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_qos_drop_profile = XCALLOC(MEM_TBL_FEA_QOS_DROP_PROFILE, sizeof(tbl_fea_qos_drop_profile_t));
    if (NULL == p_db_fea_qos_drop_profile)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_qos_drop_profile, p_fea_qos_drop_profile, sizeof(tbl_fea_qos_drop_profile_t));

    /* 4. add to db */
    p_master->fea_qos_drop_profile_array[p_fea_qos_drop_profile->key.id] = p_db_fea_qos_drop_profile;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_QOS_DROP_PROFILE, p_db_fea_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_QOS_DROP_PROFILE, p_db_fea_qos_drop_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_del_fea_qos_drop_profile_sync(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key, uint32 sync)
{
    tbl_fea_qos_drop_profile_master_t *p_master = _g_p_tbl_fea_qos_drop_profile_master;
    tbl_fea_qos_drop_profile_t *p_db_fea_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    p_db_fea_qos_drop_profile = tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(p_fea_qos_drop_profile_key);
    if (NULL == p_db_fea_qos_drop_profile)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_QOS_DROP_PROFILE, p_db_fea_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_QOS_DROP_PROFILE, p_db_fea_qos_drop_profile);
        #endif 
    }

    /* 3. delete from db */
    p_master->fea_qos_drop_profile_array[p_fea_qos_drop_profile_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_QOS_DROP_PROFILE, p_db_fea_qos_drop_profile);

    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field_sync(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_fea_qos_drop_profile_field_id_t field_id, uint32 sync)
{
    tbl_fea_qos_drop_profile_t *p_db_fea_qos_drop_profile = NULL;

    /* 1. lookup entry exist */
    p_db_fea_qos_drop_profile = tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(&p_fea_qos_drop_profile->key);
    if (NULL == p_db_fea_qos_drop_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_QOS_DROP_PROFILE_FLD_SAI_DROP_ID:
        p_db_fea_qos_drop_profile->sai_drop_id = p_fea_qos_drop_profile->sai_drop_id;
        break;

    case TBL_FEA_QOS_DROP_PROFILE_FLD_MAX:
        sal_memcpy(p_db_fea_qos_drop_profile, p_fea_qos_drop_profile, sizeof(tbl_fea_qos_drop_profile_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_QOS_DROP_PROFILE, field_id, p_db_fea_qos_drop_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_QOS_DROP_PROFILE, field_id, p_db_fea_qos_drop_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_add_fea_qos_drop_profile(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile)
{
    return tbl_fea_qos_drop_profile_add_fea_qos_drop_profile_sync(p_fea_qos_drop_profile, TRUE);
}

int32
tbl_fea_qos_drop_profile_del_fea_qos_drop_profile(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key)
{
    return tbl_fea_qos_drop_profile_del_fea_qos_drop_profile_sync(p_fea_qos_drop_profile_key, TRUE);
}

int32
tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_fea_qos_drop_profile_field_id_t field_id)
{
    return tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field_sync(p_fea_qos_drop_profile, field_id, TRUE);
}

tbl_fea_qos_drop_profile_t*
tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key)
{
    tbl_fea_qos_drop_profile_master_t *p_master = _g_p_tbl_fea_qos_drop_profile_master;

    if (GLB_QOS_DROP_PROFILE_NUM <= p_fea_qos_drop_profile_key->id)
    {
        return NULL;
    }
    return (p_master->fea_qos_drop_profile_array[p_fea_qos_drop_profile_key->id]);
}

char*
tbl_fea_qos_drop_profile_key_val2str(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_fea_qos_drop_profile->key.id);
    return str;
}

int32
tbl_fea_qos_drop_profile_key_str2val(char *str, tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile)
{
    int32 ret = 0;

    p_fea_qos_drop_profile->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_qos_drop_profile_key_name_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DROP_PROFILE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_fea_qos_drop_profile->key.id);
    return str;
}

char*
tbl_fea_qos_drop_profile_key_value_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char *str)
{
    sal_sprintf(str, "%u", p_fea_qos_drop_profile->key.id);
    return str;
}

char*
tbl_fea_qos_drop_profile_field_name_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DROP_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_QOS_DROP_PROFILE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_qos_drop_profile_field_value_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_fea_qos_drop_profile->key.id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_SAI_DROP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_drop_profile->sai_drop_id);
    }
    return str;
}

char**
tbl_fea_qos_drop_profile_table_dump(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_qos_drop_profile_key_name_dump(p_fea_qos_drop_profile, buf));
    for(i=1; i<TBL_FEA_QOS_DROP_PROFILE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_qos_drop_profile_field_name_dump(p_fea_qos_drop_profile, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_qos_drop_profile_field_value_dump(p_fea_qos_drop_profile, i, buf));
    }
    return str;
}

int32
tbl_fea_qos_drop_profile_field_value_parser(char *str, int32 field_id, tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile)
{
    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_qos_drop_profile->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_SAI_DROP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_drop_profile->sai_drop_id = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_table_parser(char** array, char* key_value,tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DROP_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_qos_drop_profile_key_str2val(key_value, p_fea_qos_drop_profile));

    for(i=1; i<TBL_FEA_QOS_DROP_PROFILE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_qos_drop_profile_field_value_parser( array[j++], i, p_fea_qos_drop_profile));
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_dump_one(tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DROP_PROFILE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_QOS_DROP_PROFILE_FLD_KEY].name,
            p_fea_qos_drop_profile->key.id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DROP_PROFILE_FLD_SAI_DROP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DROP_PROFILE_FLD_SAI_DROP_ID].name,
            p_fea_qos_drop_profile->sai_drop_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_qos_drop_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_qos_drop_profile_master_t *p_master = _g_p_tbl_fea_qos_drop_profile_master;
    tbl_fea_qos_drop_profile_t *p_db_fea_qos_drop_profile = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_QOS_DROP_PROFILE_NUM; id++)
    {
        p_db_fea_qos_drop_profile = p_master->fea_qos_drop_profile_array[id];
        if (NULL == p_db_fea_qos_drop_profile)
        {
            continue;
        }
        rc |= fn(p_db_fea_qos_drop_profile, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_qos_drop_profile_master_t*
tbl_fea_qos_drop_profile_get_master()
{
    return _g_p_tbl_fea_qos_drop_profile_master;
}

tbl_fea_qos_drop_profile_master_t*
tbl_fea_qos_drop_profile_init_fea_qos_drop_profile()
{
    _g_p_tbl_fea_qos_drop_profile_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_qos_drop_profile_master_t));
    return _g_p_tbl_fea_qos_drop_profile_master;
}

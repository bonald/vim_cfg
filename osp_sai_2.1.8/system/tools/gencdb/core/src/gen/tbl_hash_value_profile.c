
#include "proto.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "cdb_data_cmp.h"

tbl_hash_value_profile_master_t *_g_p_tbl_hash_value_profile_master = NULL;

static uint32
_tbl_hash_value_profile_hash_make(tbl_hash_value_profile_t *p_hash_value_profile)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_hash_value_profile->key;
    for (index = 0; index < sizeof(p_hash_value_profile->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_hash_value_profile_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_hash_value_profile_t *p_hash_value_profile1 = (tbl_hash_value_profile_t*)p_arg1;
    tbl_hash_value_profile_t *p_hash_value_profile2 = (tbl_hash_value_profile_t*)p_arg2;

    if (0 == sal_memcmp(&p_hash_value_profile1->key, &p_hash_value_profile2->key, sizeof(tbl_hash_value_profile_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_hash_value_profile_add_hash_value_profile_sync(tbl_hash_value_profile_t *p_hash_value_profile, uint32 sync)
{
    tbl_hash_value_profile_master_t *p_master = _g_p_tbl_hash_value_profile_master;
    tbl_hash_value_profile_t *p_db_hash_value_profile = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_hash_value_profile_get_hash_value_profile(&p_hash_value_profile->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_hash_value_profile = XCALLOC(MEM_TBL_HASH_VALUE_PROFILE, sizeof(tbl_hash_value_profile_t));
    if (NULL == p_db_hash_value_profile)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_hash_value_profile, p_hash_value_profile, sizeof(tbl_hash_value_profile_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->hash_value_profile_hash, (void*)p_db_hash_value_profile, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->hash_value_profile_list, p_db_hash_value_profile);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_HASH_VALUE_PROFILE, p_db_hash_value_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_HASH_VALUE_PROFILE, p_db_hash_value_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hash_value_profile_del_hash_value_profile_sync(tbl_hash_value_profile_key_t *p_hash_value_profile_key, uint32 sync)
{
    tbl_hash_value_profile_master_t *p_master = _g_p_tbl_hash_value_profile_master;
    tbl_hash_value_profile_t *p_db_hash_value_profile = NULL;

    /* 1. lookup entry exist */
    p_db_hash_value_profile = tbl_hash_value_profile_get_hash_value_profile(p_hash_value_profile_key);
    if (NULL == p_db_hash_value_profile)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_HASH_VALUE_PROFILE, p_db_hash_value_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_HASH_VALUE_PROFILE, p_db_hash_value_profile);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->hash_value_profile_hash, (void*)p_db_hash_value_profile);
    ctclib_slistnode_delete(p_master->hash_value_profile_list, p_db_hash_value_profile);

    /* 4. free db entry */
    XFREE(MEM_TBL_HASH_VALUE_PROFILE, p_db_hash_value_profile);

    return PM_E_NONE;
}

int32
tbl_hash_value_profile_set_hash_value_profile_field_sync(tbl_hash_value_profile_t *p_hash_value_profile, tbl_hash_value_profile_field_id_t field_id, uint32 sync)
{
    tbl_hash_value_profile_t *p_db_hash_value_profile = NULL;

    /* 1. lookup entry exist */
    p_db_hash_value_profile = tbl_hash_value_profile_get_hash_value_profile(&p_hash_value_profile->key);
    if (NULL == p_db_hash_value_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION:
        sal_memcpy(p_db_hash_value_profile->description, p_hash_value_profile->description, sizeof(p_hash_value_profile->description));
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA:
        p_db_hash_value_profile->bitmap_data = p_hash_value_profile->bitmap_data;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND:
        p_db_hash_value_profile->bitmap_command = p_hash_value_profile->bitmap_command;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_INDEX:
        p_db_hash_value_profile->index = p_hash_value_profile->index;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL:
        p_db_hash_value_profile->agg_all = p_hash_value_profile->agg_all;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT:
        p_db_hash_value_profile->agg_input = p_hash_value_profile->agg_input;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC:
        p_db_hash_value_profile->agg_static = p_hash_value_profile->agg_static;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC:
        p_db_hash_value_profile->agg_dynamic = p_hash_value_profile->agg_dynamic;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT:
        p_db_hash_value_profile->agg_resilient = p_hash_value_profile->agg_resilient;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL:
        p_db_hash_value_profile->ecmp_all = p_hash_value_profile->ecmp_all;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC:
        p_db_hash_value_profile->ecmp_static = p_hash_value_profile->ecmp_static;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3:
        p_db_hash_value_profile->ecmp_static_l3 = p_hash_value_profile->ecmp_static_l3;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS:
        p_db_hash_value_profile->ecmp_static_mpls = p_hash_value_profile->ecmp_static_mpls;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE:
        p_db_hash_value_profile->ecmp_static_nvgre = p_hash_value_profile->ecmp_static_nvgre;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN:
        p_db_hash_value_profile->ecmp_static_vxlan = p_hash_value_profile->ecmp_static_vxlan;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC:
        p_db_hash_value_profile->ecmp_dynamic = p_hash_value_profile->ecmp_dynamic;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID:
        p_db_hash_value_profile->ecmp_dynamic_flow_id = p_hash_value_profile->ecmp_dynamic_flow_id;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_REF_CNT:
        p_db_hash_value_profile->ref_cnt = p_hash_value_profile->ref_cnt;
        break;

    case TBL_HASH_VALUE_PROFILE_FLD_MAX:
        sal_memcpy(p_db_hash_value_profile, p_hash_value_profile, sizeof(tbl_hash_value_profile_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_HASH_VALUE_PROFILE, field_id, p_db_hash_value_profile);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_HASH_VALUE_PROFILE, field_id, p_db_hash_value_profile);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hash_value_profile_add_hash_value_profile(tbl_hash_value_profile_t *p_hash_value_profile)
{
    return tbl_hash_value_profile_add_hash_value_profile_sync(p_hash_value_profile, TRUE);
}

int32
tbl_hash_value_profile_del_hash_value_profile(tbl_hash_value_profile_key_t *p_hash_value_profile_key)
{
    return tbl_hash_value_profile_del_hash_value_profile_sync(p_hash_value_profile_key, TRUE);
}

int32
tbl_hash_value_profile_set_hash_value_profile_field(tbl_hash_value_profile_t *p_hash_value_profile, tbl_hash_value_profile_field_id_t field_id)
{
    return tbl_hash_value_profile_set_hash_value_profile_field_sync(p_hash_value_profile, field_id, TRUE);
}

tbl_hash_value_profile_t*
tbl_hash_value_profile_get_hash_value_profile(tbl_hash_value_profile_key_t *p_hash_value_profile_key)
{
    tbl_hash_value_profile_master_t *p_master = _g_p_tbl_hash_value_profile_master;
    tbl_hash_value_profile_t lkp;

    sal_memcpy(&lkp.key, p_hash_value_profile_key, sizeof(tbl_hash_value_profile_key_t));
    return ctclib_hash_lookup(p_master->hash_value_profile_hash, &lkp);
}

char*
tbl_hash_value_profile_key_val2str(tbl_hash_value_profile_t *p_hash_value_profile, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_hash_value_profile->key.name);
    return str;
}

int32
tbl_hash_value_profile_key_str2val(char *str, tbl_hash_value_profile_t *p_hash_value_profile)
{
    int32 ret = 0;

    sal_strcpy(p_hash_value_profile->key.name, str);
    return ret;
}

char*
tbl_hash_value_profile_key_name_dump(tbl_hash_value_profile_t *p_hash_value_profile, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_PROFILE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_hash_value_profile->key.name);
    return str;
}

char*
tbl_hash_value_profile_key_value_dump(tbl_hash_value_profile_t *p_hash_value_profile, char *str)
{
    sal_sprintf(str, "%s", p_hash_value_profile->key.name);
    return str;
}

char*
tbl_hash_value_profile_field_name_dump(tbl_hash_value_profile_t *p_hash_value_profile, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_HASH_VALUE_PROFILE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_hash_value_profile_field_value_dump(tbl_hash_value_profile_t *p_hash_value_profile, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_hash_value_profile->key.name);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION, field_id))
    {
        sal_sprintf(str, "%s", p_hash_value_profile->description);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->bitmap_command);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->index);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->agg_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->agg_input);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->agg_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->agg_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->agg_resilient);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_static_l3);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_static_mpls);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_static_nvgre);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_static_vxlan);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ecmp_dynamic_flow_id);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_profile->ref_cnt);
    }
    return str;
}

char**
tbl_hash_value_profile_table_dump(tbl_hash_value_profile_t *p_hash_value_profile, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_hash_value_profile_key_name_dump(p_hash_value_profile, buf));
    for(i=1; i<TBL_HASH_VALUE_PROFILE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_hash_value_profile_field_name_dump(p_hash_value_profile, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_hash_value_profile_field_value_dump(p_hash_value_profile, i, buf));
    }
    return str;
}

int32
tbl_hash_value_profile_field_value_parser(char *str, int32 field_id, tbl_hash_value_profile_t *p_hash_value_profile)
{
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_KEY, field_id))
    {
        sal_strcpy(p_hash_value_profile->key.name, str);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION, field_id))
    {
        sal_strcpy(p_hash_value_profile->description, str);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA, field_id))
    {
        int32 ret;
        p_hash_value_profile->bitmap_data = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND, field_id))
    {
        int32 ret;
        p_hash_value_profile->bitmap_command = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_INDEX, field_id))
    {
        int32 ret;
        p_hash_value_profile->index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL, field_id))
    {
        int32 ret;
        p_hash_value_profile->agg_all = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT, field_id))
    {
        int32 ret;
        p_hash_value_profile->agg_input = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC, field_id))
    {
        int32 ret;
        p_hash_value_profile->agg_static = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC, field_id))
    {
        int32 ret;
        p_hash_value_profile->agg_dynamic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT, field_id))
    {
        int32 ret;
        p_hash_value_profile->agg_resilient = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_all = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_static = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_static_l3 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_static_mpls = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_static_nvgre = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_static_vxlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_dynamic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID, field_id))
    {
        int32 ret;
        p_hash_value_profile->ecmp_dynamic_flow_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_hash_value_profile->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_hash_value_profile_table_parser(char** array, char* key_value,tbl_hash_value_profile_t *p_hash_value_profile)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_PROFILE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_hash_value_profile_key_str2val(key_value, p_hash_value_profile));

    for(i=1; i<TBL_HASH_VALUE_PROFILE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_hash_value_profile_field_value_parser( array[j++], i, p_hash_value_profile));
    }

    return PM_E_NONE;
}

int32
tbl_hash_value_profile_dump_one(tbl_hash_value_profile_t *p_hash_value_profile, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_PROFILE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_KEY].name,
            p_hash_value_profile->key.name);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION].name,
            p_hash_value_profile->description);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA].name,
            p_hash_value_profile->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND].name,
            p_hash_value_profile->bitmap_command);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_INDEX].name,
            p_hash_value_profile->index);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL].name,
            p_hash_value_profile->agg_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT].name,
            p_hash_value_profile->agg_input);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC].name,
            p_hash_value_profile->agg_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC].name,
            p_hash_value_profile->agg_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT].name,
            p_hash_value_profile->agg_resilient);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL].name,
            p_hash_value_profile->ecmp_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC].name,
            p_hash_value_profile->ecmp_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3].name,
            p_hash_value_profile->ecmp_static_l3);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS].name,
            p_hash_value_profile->ecmp_static_mpls);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE].name,
            p_hash_value_profile->ecmp_static_nvgre);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN].name,
            p_hash_value_profile->ecmp_static_vxlan);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC].name,
            p_hash_value_profile->ecmp_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID].name,
            p_hash_value_profile->ecmp_dynamic_flow_id);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_PROFILE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_PROFILE_FLD_REF_CNT].name,
            p_hash_value_profile->ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_hash_value_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_hash_value_profile_master_t *p_master = _g_p_tbl_hash_value_profile_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_hash_value_profile_t *p_db_hash_value_profile = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->hash_value_profile_list, p_db_hash_value_profile, listnode, next)
    {
        rc |= fn(p_db_hash_value_profile, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_hash_value_profile_master_t*
tbl_hash_value_profile_get_master()
{
    return _g_p_tbl_hash_value_profile_master;
}

tbl_hash_value_profile_master_t*
tbl_hash_value_profile_init_hash_value_profile()
{
    _g_p_tbl_hash_value_profile_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_hash_value_profile_master_t));
    _g_p_tbl_hash_value_profile_master->hash_value_profile_hash = ctclib_hash_create(_tbl_hash_value_profile_hash_make, _tbl_hash_value_profile_hash_cmp);
    _g_p_tbl_hash_value_profile_master->hash_value_profile_list = ctclib_slist_create(tbl_hash_value_profile_cmp, NULL);
    return _g_p_tbl_hash_value_profile_master;
}


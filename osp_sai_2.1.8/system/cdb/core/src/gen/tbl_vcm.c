
#include "proto.h"
#include "gen/tbl_vcm_define.h"
#include "gen/tbl_vcm.h"
#include "cdb_data_cmp.h"

tbl_vcm_master_t *_g_p_tbl_vcm_master = NULL;

static uint32
_tbl_vcm_hash_make(tbl_vcm_t *p_vcm)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vcm->key;
    for (index = 0; index < sizeof(p_vcm->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_vcm_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_vcm_t *p_vcm1 = (tbl_vcm_t*)p_arg1;
    tbl_vcm_t *p_vcm2 = (tbl_vcm_t*)p_arg2;

    if (0 == sal_memcmp(&p_vcm1->key, &p_vcm2->key, sizeof(tbl_vcm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_vcm_add_vcm_sync(tbl_vcm_t *p_vcm, uint32 sync)
{
    tbl_vcm_master_t *p_master = _g_p_tbl_vcm_master;
    tbl_vcm_t *p_db_vcm = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_vcm_get_vcm(&p_vcm->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vcm = XCALLOC(MEM_TBL_VCM, sizeof(tbl_vcm_t));
    if (NULL == p_db_vcm)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vcm, p_vcm, sizeof(tbl_vcm_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->vcm_hash, (void*)p_db_vcm, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->vcm_list, p_db_vcm);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VCM, p_db_vcm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VCM, p_db_vcm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vcm_del_vcm_sync(tbl_vcm_key_t *p_vcm_key, uint32 sync)
{
    tbl_vcm_master_t *p_master = _g_p_tbl_vcm_master;
    tbl_vcm_t *p_db_vcm = NULL;

    /* 1. lookup entry exist */
    p_db_vcm = tbl_vcm_get_vcm(p_vcm_key);
    if (NULL == p_db_vcm)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VCM, p_db_vcm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VCM, p_db_vcm);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->vcm_hash, (void*)p_db_vcm);
    ctclib_slistnode_delete(p_master->vcm_list, p_db_vcm);

    /* 4. free db entry */
    XFREE(MEM_TBL_VCM, p_db_vcm);

    return PM_E_NONE;
}

int32
tbl_vcm_set_vcm_field_sync(tbl_vcm_t *p_vcm, tbl_vcm_field_id_t field_id, uint32 sync)
{
    tbl_vcm_t *p_db_vcm = NULL;

    /* 1. lookup entry exist */
    p_db_vcm = tbl_vcm_get_vcm(&p_vcm->key);
    if (NULL == p_db_vcm)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VCM_FLD_SLOT:
        p_db_vcm->slot = p_vcm->slot;
        break;

    case TBL_VCM_FLD_VCM_TYPE:
        p_db_vcm->vcm_type = p_vcm->vcm_type;
        break;

    case TBL_VCM_FLD_VCM_CHANNEL_NUMS:
        p_db_vcm->vcm_channel_nums = p_vcm->vcm_channel_nums;
        break;

    case TBL_VCM_FLD_CHANNEL_NAME0:
        sal_memcpy(p_db_vcm->channel_name0, p_vcm->channel_name0, sizeof(p_vcm->channel_name0));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME1:
        sal_memcpy(p_db_vcm->channel_name1, p_vcm->channel_name1, sizeof(p_vcm->channel_name1));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME2:
        sal_memcpy(p_db_vcm->channel_name2, p_vcm->channel_name2, sizeof(p_vcm->channel_name2));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME3:
        sal_memcpy(p_db_vcm->channel_name3, p_vcm->channel_name3, sizeof(p_vcm->channel_name3));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME4:
        sal_memcpy(p_db_vcm->channel_name4, p_vcm->channel_name4, sizeof(p_vcm->channel_name4));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME5:
        sal_memcpy(p_db_vcm->channel_name5, p_vcm->channel_name5, sizeof(p_vcm->channel_name5));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME6:
        sal_memcpy(p_db_vcm->channel_name6, p_vcm->channel_name6, sizeof(p_vcm->channel_name6));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME7:
        sal_memcpy(p_db_vcm->channel_name7, p_vcm->channel_name7, sizeof(p_vcm->channel_name7));
        break;

    case TBL_VCM_FLD_CHANNEL_NAME8:
        sal_memcpy(p_db_vcm->channel_name8, p_vcm->channel_name8, sizeof(p_vcm->channel_name8));
        break;

    case TBL_VCM_FLD_FLAG:
        sal_memcpy(p_db_vcm->flag, p_vcm->flag, sizeof(p_vcm->flag));
        break;

    case TBL_VCM_FLD_CURR_DEPEND_PIN:
        sal_memcpy(p_db_vcm->curr_depend_pin, p_vcm->curr_depend_pin, sizeof(p_vcm->curr_depend_pin));
        break;

    case TBL_VCM_FLD_VOLT_SPLIT_FACTOR:
        sal_memcpy(p_db_vcm->volt_split_factor, p_vcm->volt_split_factor, sizeof(p_vcm->volt_split_factor));
        break;

    case TBL_VCM_FLD_LOW:
        sal_memcpy(p_db_vcm->low, p_vcm->low, sizeof(p_vcm->low));
        break;

    case TBL_VCM_FLD_HIGH:
        sal_memcpy(p_db_vcm->high, p_vcm->high, sizeof(p_vcm->high));
        break;

    case TBL_VCM_FLD_VOLT:
        sal_memcpy(p_db_vcm->volt, p_vcm->volt, sizeof(p_vcm->volt));
        break;

    case TBL_VCM_FLD_CURR:
        sal_memcpy(p_db_vcm->curr, p_vcm->curr, sizeof(p_vcm->curr));
        break;

    case TBL_VCM_FLD_TMPR:
        sal_memcpy(p_db_vcm->tmpr, p_vcm->tmpr, sizeof(p_vcm->tmpr));
        break;

    case TBL_VCM_FLD_MAX:
        sal_memcpy(p_db_vcm, p_vcm, sizeof(tbl_vcm_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VCM, field_id, p_db_vcm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VCM, field_id, p_db_vcm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vcm_add_vcm(tbl_vcm_t *p_vcm)
{
    return tbl_vcm_add_vcm_sync(p_vcm, TRUE);
}

int32
tbl_vcm_del_vcm(tbl_vcm_key_t *p_vcm_key)
{
    return tbl_vcm_del_vcm_sync(p_vcm_key, TRUE);
}

int32
tbl_vcm_set_vcm_field(tbl_vcm_t *p_vcm, tbl_vcm_field_id_t field_id)
{
    return tbl_vcm_set_vcm_field_sync(p_vcm, field_id, TRUE);
}

tbl_vcm_t*
tbl_vcm_get_vcm(tbl_vcm_key_t *p_vcm_key)
{
    tbl_vcm_master_t *p_master = _g_p_tbl_vcm_master;
    tbl_vcm_t lkp;

    sal_memcpy(&lkp.key, p_vcm_key, sizeof(tbl_vcm_key_t));
    return ctclib_hash_lookup(p_master->vcm_hash, &lkp);
}

char*
tbl_vcm_key_val2str(tbl_vcm_t *p_vcm, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_vcm->key.id);
    return str;
}

int32
tbl_vcm_key_str2val(char *str, tbl_vcm_t *p_vcm)
{
    int32 ret = 0;

    p_vcm->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_vcm_key_name_dump(tbl_vcm_t *p_vcm, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VCM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_vcm->key.id);
    return str;
}

char*
tbl_vcm_key_value_dump(tbl_vcm_t *p_vcm, char *str)
{
    sal_sprintf(str, "%d", p_vcm->key.id);
    return str;
}

char*
tbl_vcm_field_name_dump(tbl_vcm_t *p_vcm, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VCM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VCM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vcm_field_value_dump(tbl_vcm_t *p_vcm, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VCM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_vcm->key.id);
    }
    if (FLD_MATCH(TBL_VCM_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_vcm->slot);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_vcm->vcm_type);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_CHANNEL_NUMS, field_id))
    {
        sal_sprintf(str, "%u", p_vcm->vcm_channel_nums);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME0, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name0);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME1, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name1);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME2, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name2);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME3, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name3);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME4, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name4);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME5, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name5);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME6, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name6);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME7, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name7);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME8, field_id))
    {
        sal_sprintf(str, "%s", p_vcm->channel_name8);
    }
    if (FLD_MATCH(TBL_VCM_FLD_FLAG, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->flag, sizeof(p_vcm->flag)/4);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR_DEPEND_PIN, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->curr_depend_pin, sizeof(p_vcm->curr_depend_pin));
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT_SPLIT_FACTOR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->volt_split_factor, sizeof(p_vcm->volt_split_factor)/sizeof(double));
    }
    if (FLD_MATCH(TBL_VCM_FLD_LOW, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->low, sizeof(p_vcm->low)/sizeof(double));
    }
    if (FLD_MATCH(TBL_VCM_FLD_HIGH, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->high, sizeof(p_vcm->high)/sizeof(double));
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->volt, sizeof(p_vcm->volt)/sizeof(double));
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->curr, sizeof(p_vcm->curr)/sizeof(double));
    }
    if (FLD_MATCH(TBL_VCM_FLD_TMPR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_vcm->tmpr, sizeof(p_vcm->tmpr)/sizeof(double));
    }
    return str;
}

char**
tbl_vcm_table_dump(tbl_vcm_t *p_vcm, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vcm_key_name_dump(p_vcm, buf));
    for(i=1; i<TBL_VCM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vcm_field_name_dump(p_vcm, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vcm_field_value_dump(p_vcm, i, buf));
    }
    return str;
}

int32
tbl_vcm_field_value_parser(char *str, int32 field_id, tbl_vcm_t *p_vcm)
{
    if (FLD_MATCH(TBL_VCM_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_VCM_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_TYPE, field_id))
    {
        int32 ret;
        p_vcm->vcm_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_CHANNEL_NUMS, field_id))
    {
        int32 ret;
        p_vcm->vcm_channel_nums = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME0, field_id))
    {
        sal_strcpy(p_vcm->channel_name0, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME1, field_id))
    {
        sal_strcpy(p_vcm->channel_name1, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME2, field_id))
    {
        sal_strcpy(p_vcm->channel_name2, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME3, field_id))
    {
        sal_strcpy(p_vcm->channel_name3, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME4, field_id))
    {
        sal_strcpy(p_vcm->channel_name4, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME5, field_id))
    {
        sal_strcpy(p_vcm->channel_name5, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME6, field_id))
    {
        sal_strcpy(p_vcm->channel_name6, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME7, field_id))
    {
        sal_strcpy(p_vcm->channel_name7, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME8, field_id))
    {
        sal_strcpy(p_vcm->channel_name8, str);
    }
    if (FLD_MATCH(TBL_VCM_FLD_FLAG, field_id))
    {
        cdb_uint32_array_str2val(str, p_vcm->flag);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR_DEPEND_PIN, field_id))
    {
        cdb_uint8_array_str2val(str, p_vcm->curr_depend_pin);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT_SPLIT_FACTOR, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->volt_split_factor);
    }
    if (FLD_MATCH(TBL_VCM_FLD_LOW, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->low);
    }
    if (FLD_MATCH(TBL_VCM_FLD_HIGH, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->high);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->volt);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->curr);
    }
    if (FLD_MATCH(TBL_VCM_FLD_TMPR, field_id))
    {
        cdb_double_array_str2val(str, p_vcm->tmpr);
    }
    return PM_E_NONE;
}

int32
tbl_vcm_table_parser(char** array, char* key_value,tbl_vcm_t *p_vcm)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VCM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vcm_key_str2val(key_value, p_vcm));

    for(i=1; i<TBL_VCM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vcm_field_value_parser( array[j++], i, p_vcm));
    }

    return PM_E_NONE;
}

int32
tbl_vcm_dump_one(tbl_vcm_t *p_vcm, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VCM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VCM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_VCM_FLD_KEY].name,
            p_vcm->key.id);
    }
    if (FLD_MATCH(TBL_VCM_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_VCM_FLD_SLOT].name,
            p_vcm->slot);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VCM_FLD_VCM_TYPE].name,
            p_vcm->vcm_type);
    }
    if (FLD_MATCH(TBL_VCM_FLD_VCM_CHANNEL_NUMS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VCM_FLD_VCM_CHANNEL_NUMS].name,
            p_vcm->vcm_channel_nums);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME0, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME0].name,
            p_vcm->channel_name0);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME1, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME1].name,
            p_vcm->channel_name1);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME2].name,
            p_vcm->channel_name2);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME3, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME3].name,
            p_vcm->channel_name3);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME4].name,
            p_vcm->channel_name4);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME5, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME5].name,
            p_vcm->channel_name5);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME6].name,
            p_vcm->channel_name6);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME7, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME7].name,
            p_vcm->channel_name7);
    }
    if (FLD_MATCH(TBL_VCM_FLD_CHANNEL_NAME8, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CHANNEL_NAME8].name,
            p_vcm->channel_name8);
    }
    if (FLD_MATCH(TBL_VCM_FLD_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_FLAG].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->flag, sizeof(p_vcm->flag)/4));
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR_DEPEND_PIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CURR_DEPEND_PIN].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->curr_depend_pin, sizeof(p_vcm->curr_depend_pin)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT_SPLIT_FACTOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_VOLT_SPLIT_FACTOR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->volt_split_factor, sizeof(p_vcm->volt_split_factor)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_LOW].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->low, sizeof(p_vcm->low)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_HIGH].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->high, sizeof(p_vcm->high)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_VOLT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_VOLT].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->volt, sizeof(p_vcm->volt)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_CURR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_CURR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->curr, sizeof(p_vcm->curr)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_VCM_FLD_TMPR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VCM_FLD_TMPR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_vcm->tmpr, sizeof(p_vcm->tmpr)/sizeof(double)));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vcm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vcm_master_t *p_master = _g_p_tbl_vcm_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vcm_t *p_db_vcm = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vcm_list, p_db_vcm, listnode, next)
    {
        rc |= fn(p_db_vcm, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vcm_master_t*
tbl_vcm_get_master()
{
    return _g_p_tbl_vcm_master;
}

tbl_vcm_master_t*
tbl_vcm_init_vcm()
{
    _g_p_tbl_vcm_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vcm_master_t));
    _g_p_tbl_vcm_master->vcm_hash = ctclib_hash_create(_tbl_vcm_hash_make, _tbl_vcm_hash_cmp);
    _g_p_tbl_vcm_master->vcm_list = ctclib_slist_create(tbl_vcm_cmp, NULL);
    return _g_p_tbl_vcm_master;
}


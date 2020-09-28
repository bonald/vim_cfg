
#include "proto.h"
#include "gen/tbl_psu_define.h"
#include "gen/tbl_psu.h"
#include "cdb_data_cmp.h"

tbl_psu_master_t *_g_p_tbl_psu_master = NULL;

static uint32
_tbl_psu_hash_make(tbl_psu_t *p_psu)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_psu->key;
    for (index = 0; index < sizeof(p_psu->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_psu_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_psu_t *p_psu1 = (tbl_psu_t*)p_arg1;
    tbl_psu_t *p_psu2 = (tbl_psu_t*)p_arg2;

    if (0 == sal_memcmp(&p_psu1->key, &p_psu2->key, sizeof(tbl_psu_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_psu_add_psu_sync(tbl_psu_t *p_psu, uint32 sync)
{
    tbl_psu_master_t *p_master = _g_p_tbl_psu_master;
    tbl_psu_t *p_db_psu = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_psu_get_psu(&p_psu->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_psu = XCALLOC(MEM_TBL_PSU, sizeof(tbl_psu_t));
    if (NULL == p_db_psu)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_psu, p_psu, sizeof(tbl_psu_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->psu_hash, (void*)p_db_psu, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->psu_list, p_db_psu);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PSU, p_db_psu);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PSU, p_db_psu);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_psu_del_psu_sync(tbl_psu_key_t *p_psu_key, uint32 sync)
{
    tbl_psu_master_t *p_master = _g_p_tbl_psu_master;
    tbl_psu_t *p_db_psu = NULL;

    /* 1. lookup entry exist */
    p_db_psu = tbl_psu_get_psu(p_psu_key);
    if (NULL == p_db_psu)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PSU, p_db_psu);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PSU, p_db_psu);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->psu_hash, (void*)p_db_psu);
    ctclib_slistnode_delete(p_master->psu_list, p_db_psu);

    /* 4. free db entry */
    XFREE(MEM_TBL_PSU, p_db_psu);

    return PM_E_NONE;
}

int32
tbl_psu_set_psu_field_sync(tbl_psu_t *p_psu, tbl_psu_field_id_t field_id, uint32 sync)
{
    tbl_psu_t *p_db_psu = NULL;

    /* 1. lookup entry exist */
    p_db_psu = tbl_psu_get_psu(&p_psu->key);
    if (NULL == p_db_psu)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PSU_FLD_SLOT:
        p_db_psu->slot = p_psu->slot;
        break;

    case TBL_PSU_FLD_NAME:
        sal_memcpy(p_db_psu->name, p_psu->name, sizeof(p_psu->name));
        break;

    case TBL_PSU_FLD_ABSENT:
        p_db_psu->absent = p_psu->absent;
        break;

    case TBL_PSU_FLD_RUN_STATUS:
        sal_memcpy(p_db_psu->run_status, p_psu->run_status, sizeof(p_psu->run_status));
        break;

    case TBL_PSU_FLD_ALERT_STATUS:
        sal_memcpy(p_db_psu->alert_status, p_psu->alert_status, sizeof(p_psu->alert_status));
        break;

    case TBL_PSU_FLD_MODE:
        sal_memcpy(p_db_psu->mode, p_psu->mode, sizeof(p_psu->mode));
        break;

    case TBL_PSU_FLD_FIXED_STATUS:
        p_db_psu->fixed_status = p_psu->fixed_status;
        break;

    case TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE:
        p_db_psu->psu_health_status_change = p_psu->psu_health_status_change;
        break;

    case TBL_PSU_FLD_SUPPORT_PMBUS:
        p_db_psu->support_pmbus = p_psu->support_pmbus;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_BYTE:
        p_db_psu->pmbus_status_byte = p_psu->pmbus_status_byte;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_WORD:
        p_db_psu->pmbus_status_word = p_psu->pmbus_status_word;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_VOUT:
        p_db_psu->pmbus_status_vout = p_psu->pmbus_status_vout;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_IOUT:
        p_db_psu->pmbus_status_iout = p_psu->pmbus_status_iout;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_INPUT:
        p_db_psu->pmbus_status_input = p_psu->pmbus_status_input;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE:
        p_db_psu->pmbus_status_temperature = p_psu->pmbus_status_temperature;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_CML:
        p_db_psu->pmbus_status_cml = p_psu->pmbus_status_cml;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_OTHER:
        p_db_psu->pmbus_status_other = p_psu->pmbus_status_other;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC:
        p_db_psu->pmbus_status_mfr_specific = p_psu->pmbus_status_mfr_specific;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2:
        p_db_psu->pmbus_status_fans_1_2 = p_psu->pmbus_status_fans_1_2;
        break;

    case TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4:
        p_db_psu->pmbus_status_fans_3_4 = p_psu->pmbus_status_fans_3_4;
        break;

    case TBL_PSU_FLD_MAX:
        sal_memcpy(p_db_psu, p_psu, sizeof(tbl_psu_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PSU, field_id, p_db_psu);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PSU, field_id, p_db_psu);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_psu_add_psu(tbl_psu_t *p_psu)
{
    return tbl_psu_add_psu_sync(p_psu, TRUE);
}

int32
tbl_psu_del_psu(tbl_psu_key_t *p_psu_key)
{
    return tbl_psu_del_psu_sync(p_psu_key, TRUE);
}

int32
tbl_psu_set_psu_field(tbl_psu_t *p_psu, tbl_psu_field_id_t field_id)
{
    return tbl_psu_set_psu_field_sync(p_psu, field_id, TRUE);
}

tbl_psu_t*
tbl_psu_get_psu(tbl_psu_key_t *p_psu_key)
{
    tbl_psu_master_t *p_master = _g_p_tbl_psu_master;
    tbl_psu_t lkp;

    sal_memcpy(&lkp.key, p_psu_key, sizeof(tbl_psu_key_t));
    return ctclib_hash_lookup(p_master->psu_hash, &lkp);
}

char*
tbl_psu_key_val2str(tbl_psu_t *p_psu, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_psu->key.id);
    return str;
}

int32
tbl_psu_key_str2val(char *str, tbl_psu_t *p_psu)
{
    int32 ret = 0;

    p_psu->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_psu_key_name_dump(tbl_psu_t *p_psu, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSU);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_psu->key.id);
    return str;
}

char*
tbl_psu_key_value_dump(tbl_psu_t *p_psu, char *str)
{
    sal_sprintf(str, "%d", p_psu->key.id);
    return str;
}

char*
tbl_psu_field_name_dump(tbl_psu_t *p_psu, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PSU);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PSU_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_psu_field_value_dump(tbl_psu_t *p_psu, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PSU_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_psu->key.id);
    }
    if (FLD_MATCH(TBL_PSU_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_psu->slot);
    }
    if (FLD_MATCH(TBL_PSU_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_psu->name);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ABSENT, field_id))
    {
        sal_sprintf(str, "%d", p_psu->absent);
    }
    if (FLD_MATCH(TBL_PSU_FLD_RUN_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_psu->run_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ALERT_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_psu->alert_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", p_psu->mode);
    }
    if (FLD_MATCH(TBL_PSU_FLD_FIXED_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_psu->fixed_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE, field_id))
    {
        sal_sprintf(str, "%u", p_psu->psu_health_status_change);
    }
    if (FLD_MATCH(TBL_PSU_FLD_SUPPORT_PMBUS, field_id))
    {
        sal_sprintf(str, "%u", p_psu->support_pmbus);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_BYTE, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_byte);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_WORD, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_word);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_VOUT, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_vout);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_IOUT, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_iout);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_INPUT, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_input);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_temperature);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_CML, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_cml);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_OTHER, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_other);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_mfr_specific);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_fans_1_2);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4, field_id))
    {
        sal_sprintf(str, "%u", p_psu->pmbus_status_fans_3_4);
    }
    return str;
}

char**
tbl_psu_table_dump(tbl_psu_t *p_psu, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_psu_key_name_dump(p_psu, buf));
    for(i=1; i<TBL_PSU_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_psu_field_name_dump(p_psu, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_psu_field_value_dump(p_psu, i, buf));
    }
    return str;
}

int32
tbl_psu_field_value_parser(char *str, int32 field_id, tbl_psu_t *p_psu)
{
    if (FLD_MATCH(TBL_PSU_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_PSU_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_PSU_FLD_NAME, field_id))
    {
        sal_strcpy(p_psu->name, str);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ABSENT, field_id))
    {
    }
    if (FLD_MATCH(TBL_PSU_FLD_RUN_STATUS, field_id))
    {
        sal_strcpy(p_psu->run_status, str);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ALERT_STATUS, field_id))
    {
        sal_strcpy(p_psu->alert_status, str);
    }
    if (FLD_MATCH(TBL_PSU_FLD_MODE, field_id))
    {
        sal_strcpy(p_psu->mode, str);
    }
    if (FLD_MATCH(TBL_PSU_FLD_FIXED_STATUS, field_id))
    {
        int32 ret;
        p_psu->fixed_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE, field_id))
    {
        int32 ret;
        p_psu->psu_health_status_change = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_SUPPORT_PMBUS, field_id))
    {
        int32 ret;
        p_psu->support_pmbus = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_BYTE, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_byte = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_WORD, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_word = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_VOUT, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_vout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_IOUT, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_iout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_INPUT, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_input = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_temperature = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_CML, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_cml = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_OTHER, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_other = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_mfr_specific = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_fans_1_2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4, field_id))
    {
        int32 ret;
        p_psu->pmbus_status_fans_3_4 = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_psu_table_parser(char** array, char* key_value,tbl_psu_t *p_psu)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSU);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_psu_key_str2val(key_value, p_psu));

    for(i=1; i<TBL_PSU_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_psu_field_value_parser( array[j++], i, p_psu));
    }

    return PM_E_NONE;
}

int32
tbl_psu_dump_one(tbl_psu_t *p_psu, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSU);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PSU_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PSU_FLD_KEY].name,
            p_psu->key.id);
    }
    if (FLD_MATCH(TBL_PSU_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PSU_FLD_SLOT].name,
            p_psu->slot);
    }
    if (FLD_MATCH(TBL_PSU_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PSU_FLD_NAME].name,
            p_psu->name);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ABSENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PSU_FLD_ABSENT].name,
            p_psu->absent);
    }
    if (FLD_MATCH(TBL_PSU_FLD_RUN_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PSU_FLD_RUN_STATUS].name,
            p_psu->run_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_ALERT_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PSU_FLD_ALERT_STATUS].name,
            p_psu->alert_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PSU_FLD_MODE].name,
            p_psu->mode);
    }
    if (FLD_MATCH(TBL_PSU_FLD_FIXED_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_FIXED_STATUS].name,
            p_psu->fixed_status);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE].name,
            p_psu->psu_health_status_change);
    }
    if (FLD_MATCH(TBL_PSU_FLD_SUPPORT_PMBUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_SUPPORT_PMBUS].name,
            p_psu->support_pmbus);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_BYTE].name,
            p_psu->pmbus_status_byte);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_WORD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_WORD].name,
            p_psu->pmbus_status_word);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_VOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_VOUT].name,
            p_psu->pmbus_status_vout);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_IOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_IOUT].name,
            p_psu->pmbus_status_iout);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_INPUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_INPUT].name,
            p_psu->pmbus_status_input);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE].name,
            p_psu->pmbus_status_temperature);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_CML, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_CML].name,
            p_psu->pmbus_status_cml);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_OTHER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_OTHER].name,
            p_psu->pmbus_status_other);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC].name,
            p_psu->pmbus_status_mfr_specific);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2].name,
            p_psu->pmbus_status_fans_1_2);
    }
    if (FLD_MATCH(TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4].name,
            p_psu->pmbus_status_fans_3_4);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_psu_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_psu_master_t *p_master = _g_p_tbl_psu_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_psu_t *p_db_psu = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->psu_list, p_db_psu, listnode, next)
    {
        rc |= fn(p_db_psu, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_psu_master_t*
tbl_psu_get_master()
{
    return _g_p_tbl_psu_master;
}

tbl_psu_master_t*
tbl_psu_init_psu()
{
    _g_p_tbl_psu_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_psu_master_t));
    _g_p_tbl_psu_master->psu_hash = ctclib_hash_create(_tbl_psu_hash_make, _tbl_psu_hash_cmp);
    _g_p_tbl_psu_master->psu_list = ctclib_slist_create(tbl_psu_cmp, NULL);
    return _g_p_tbl_psu_master;
}


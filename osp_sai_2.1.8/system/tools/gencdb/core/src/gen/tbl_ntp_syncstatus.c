
#include "proto.h"
#include "gen/tbl_ntp_syncstatus_define.h"
#include "gen/tbl_ntp_syncstatus.h"
#include "cdb_data_cmp.h"

tbl_ntp_syncstatus_t *_g_p_tbl_ntp_syncstatus = NULL;

int32
tbl_ntp_syncstatus_set_ntp_syncstatus_field_sync(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_ntp_syncstatus_field_id_t field_id, uint32 sync)
{
    tbl_ntp_syncstatus_t *p_db_ntp_syncstatus = _g_p_tbl_ntp_syncstatus;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_SYNCSTATUS_FLD_STATUS:
        sal_memcpy(p_db_ntp_syncstatus->status, p_ntp_syncstatus->status, sizeof(p_ntp_syncstatus->status));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_STRATUM:
        sal_memcpy(p_db_ntp_syncstatus->stratum, p_ntp_syncstatus->stratum, sizeof(p_ntp_syncstatus->stratum));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_FREQUENCY:
        sal_memcpy(p_db_ntp_syncstatus->frequency, p_ntp_syncstatus->frequency, sizeof(p_ntp_syncstatus->frequency));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_PRECISION:
        sal_memcpy(p_db_ntp_syncstatus->precision, p_ntp_syncstatus->precision, sizeof(p_ntp_syncstatus->precision));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME:
        sal_memcpy(p_db_ntp_syncstatus->reference_time, p_ntp_syncstatus->reference_time, sizeof(p_ntp_syncstatus->reference_time));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY:
        sal_memcpy(p_db_ntp_syncstatus->root_delay, p_ntp_syncstatus->root_delay, sizeof(p_ntp_syncstatus->root_delay));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION:
        sal_memcpy(p_db_ntp_syncstatus->root_dispersion, p_ntp_syncstatus->root_dispersion, sizeof(p_ntp_syncstatus->root_dispersion));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_STABILITY:
        sal_memcpy(p_db_ntp_syncstatus->stability, p_ntp_syncstatus->stability, sizeof(p_ntp_syncstatus->stability));
        break;

    case TBL_NTP_SYNCSTATUS_FLD_MAX:
        sal_memcpy(p_db_ntp_syncstatus, p_ntp_syncstatus, sizeof(tbl_ntp_syncstatus_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_SYNCSTATUS, field_id, p_db_ntp_syncstatus);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_SYNCSTATUS, field_id, p_db_ntp_syncstatus);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_syncstatus_set_ntp_syncstatus_field(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_ntp_syncstatus_field_id_t field_id)
{
    return tbl_ntp_syncstatus_set_ntp_syncstatus_field_sync(p_ntp_syncstatus, field_id, TRUE);
}

tbl_ntp_syncstatus_t*
tbl_ntp_syncstatus_get_ntp_syncstatus()
{
    return _g_p_tbl_ntp_syncstatus;
}

int32
tbl_ntp_syncstatus_key_str2val(char *str, tbl_ntp_syncstatus_t *p_ntp_syncstatus)
{
    return PM_E_NONE;
}

char*
tbl_ntp_syncstatus_key_name_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SYNCSTATUS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ntp_syncstatus_key_value_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ntp_syncstatus_field_name_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SYNCSTATUS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_SYNCSTATUS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_syncstatus_field_value_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->status);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STRATUM, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->stratum);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_FREQUENCY, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->frequency);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_PRECISION, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->precision);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->reference_time);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->root_delay);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->root_dispersion);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STABILITY, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_syncstatus->stability);
    }
    return str;
}

char**
tbl_ntp_syncstatus_table_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_syncstatus_key_name_dump(p_ntp_syncstatus, buf));
    for(i=1; i<TBL_NTP_SYNCSTATUS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_syncstatus_field_name_dump(p_ntp_syncstatus, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_syncstatus_field_value_dump(p_ntp_syncstatus, i, buf));
    }
    return str;
}

int32
tbl_ntp_syncstatus_field_value_parser(char *str, int32 field_id, tbl_ntp_syncstatus_t *p_ntp_syncstatus)
{
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STATUS, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->status, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STRATUM, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->stratum, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_FREQUENCY, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->frequency, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_PRECISION, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->precision, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->reference_time, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->root_delay, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->root_dispersion, str);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STABILITY, field_id))
    {
        sal_strcpy(p_ntp_syncstatus->stability, str);
    }
    return PM_E_NONE;
}

int32
tbl_ntp_syncstatus_table_parser(char** array, char* key_value,tbl_ntp_syncstatus_t *p_ntp_syncstatus)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SYNCSTATUS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_syncstatus_key_str2val(key_value, p_ntp_syncstatus));

    for(i=1; i<TBL_NTP_SYNCSTATUS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_syncstatus_field_value_parser( array[j++], i, p_ntp_syncstatus));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_syncstatus_dump_one(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SYNCSTATUS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_STATUS].name,
            p_ntp_syncstatus->status);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STRATUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_STRATUM].name,
            p_ntp_syncstatus->stratum);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_FREQUENCY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_FREQUENCY].name,
            p_ntp_syncstatus->frequency);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_PRECISION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_PRECISION].name,
            p_ntp_syncstatus->precision);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME].name,
            p_ntp_syncstatus->reference_time);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY].name,
            p_ntp_syncstatus->root_delay);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION].name,
            p_ntp_syncstatus->root_dispersion);
    }
    if (FLD_MATCH(TBL_NTP_SYNCSTATUS_FLD_STABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SYNCSTATUS_FLD_STABILITY].name,
            p_ntp_syncstatus->stability);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_syncstatus_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_syncstatus_t *p_db_ntp_syncstatus = _g_p_tbl_ntp_syncstatus;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ntp_syncstatus)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ntp_syncstatus, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_syncstatus_t*
tbl_ntp_syncstatus_init_ntp_syncstatus()
{
    _g_p_tbl_ntp_syncstatus = XCALLOC(MEM_TBL_NTP_SYNCSTATUS, sizeof(tbl_ntp_syncstatus_t));
    return _g_p_tbl_ntp_syncstatus;
}


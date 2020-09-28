
#include "proto.h"
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "cdb_data_cmp.h"

tbl_nd_stats_t *_g_p_tbl_nd_stats = NULL;

int32
tbl_nd_stats_set_nd_stats_field_sync(tbl_nd_stats_t *p_nd_stats, tbl_nd_stats_field_id_t field_id, uint32 sync)
{
    tbl_nd_stats_t *p_db_nd_stats = _g_p_tbl_nd_stats;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ND_STATS_FLD_RCV:
        p_db_nd_stats->rcv = p_nd_stats->rcv;
        break;

    case TBL_ND_STATS_FLD_RCV_ERROR:
        p_db_nd_stats->rcv_error = p_nd_stats->rcv_error;
        break;

    case TBL_ND_STATS_FLD_RCV_TOOSHORT:
        p_db_nd_stats->rcv_tooshort = p_nd_stats->rcv_tooshort;
        break;

    case TBL_ND_STATS_FLD_RCV_INVALID:
        p_db_nd_stats->rcv_invalid = p_nd_stats->rcv_invalid;
        break;

    case TBL_ND_STATS_FLD_RCV_INCONSISTENT:
        p_db_nd_stats->rcv_inconsistent = p_nd_stats->rcv_inconsistent;
        break;

    case TBL_ND_STATS_FLD_RCV_RS:
        p_db_nd_stats->rcv_rs = p_nd_stats->rcv_rs;
        break;

    case TBL_ND_STATS_FLD_RCV_RA:
        p_db_nd_stats->rcv_ra = p_nd_stats->rcv_ra;
        break;

    case TBL_ND_STATS_FLD_RCV_NS:
        p_db_nd_stats->rcv_ns = p_nd_stats->rcv_ns;
        break;

    case TBL_ND_STATS_FLD_RCV_NA:
        p_db_nd_stats->rcv_na = p_nd_stats->rcv_na;
        break;

    case TBL_ND_STATS_FLD_RCV_REDIRECT:
        p_db_nd_stats->rcv_redirect = p_nd_stats->rcv_redirect;
        break;

    case TBL_ND_STATS_FLD_OVERFLOW_FAILED:
        p_db_nd_stats->overflow_failed = p_nd_stats->overflow_failed;
        break;

    case TBL_ND_STATS_FLD_RCV_MISS:
        p_db_nd_stats->rcv_miss = p_nd_stats->rcv_miss;
        break;

    case TBL_ND_STATS_FLD_RCV_MISS_CONFLICT:
        p_db_nd_stats->rcv_miss_conflict = p_nd_stats->rcv_miss_conflict;
        break;

    case TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT:
        p_db_nd_stats->rcv_static_conflict = p_nd_stats->rcv_static_conflict;
        break;

    case TBL_ND_STATS_FLD_ALLOCS:
        p_db_nd_stats->allocs = p_nd_stats->allocs;
        break;

    case TBL_ND_STATS_FLD_DESTROYS:
        p_db_nd_stats->destroys = p_nd_stats->destroys;
        break;

    case TBL_ND_STATS_FLD_ERROR:
        p_db_nd_stats->error = p_nd_stats->error;
        break;

    case TBL_ND_STATS_FLD_INTF_INVALID:
        p_db_nd_stats->intf_invalid = p_nd_stats->intf_invalid;
        break;

    case TBL_ND_STATS_FLD_RES_FAILED:
        p_db_nd_stats->res_failed = p_nd_stats->res_failed;
        break;

    case TBL_ND_STATS_FLD_RCV_PROBES_MCAST:
        p_db_nd_stats->rcv_probes_mcast = p_nd_stats->rcv_probes_mcast;
        break;

    case TBL_ND_STATS_FLD_RCV_PROBES_UCAST:
        p_db_nd_stats->rcv_probes_ucast = p_nd_stats->rcv_probes_ucast;
        break;

    case TBL_ND_STATS_FLD_SND_SOLICITED_NA:
        p_db_nd_stats->snd_solicited_na = p_nd_stats->snd_solicited_na;
        break;

    case TBL_ND_STATS_FLD_SND_UNSOLICITED_NA:
        p_db_nd_stats->snd_unsolicited_na = p_nd_stats->snd_unsolicited_na;
        break;

    case TBL_ND_STATS_FLD_SND_DAD_NS:
        p_db_nd_stats->snd_dad_ns = p_nd_stats->snd_dad_ns;
        break;

    case TBL_ND_STATS_FLD_MAX:
        sal_memcpy(p_db_nd_stats, p_nd_stats, sizeof(tbl_nd_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ND_STATS, field_id, p_db_nd_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ND_STATS, field_id, p_db_nd_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nd_stats_set_nd_stats_field(tbl_nd_stats_t *p_nd_stats, tbl_nd_stats_field_id_t field_id)
{
    return tbl_nd_stats_set_nd_stats_field_sync(p_nd_stats, field_id, TRUE);
}

tbl_nd_stats_t*
tbl_nd_stats_get_nd_stats()
{
    return _g_p_tbl_nd_stats;
}

int32
tbl_nd_stats_key_str2val(char *str, tbl_nd_stats_t *p_nd_stats)
{
    return PM_E_NONE;
}

char*
tbl_nd_stats_key_name_dump(tbl_nd_stats_t *p_nd_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_STATS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_nd_stats_key_value_dump(tbl_nd_stats_t *p_nd_stats, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_nd_stats_field_name_dump(tbl_nd_stats_t *p_nd_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ND_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nd_stats_field_value_dump(tbl_nd_stats_t *p_nd_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_ERROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_error);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_TOOSHORT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_tooshort);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INVALID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_invalid);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INCONSISTENT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_inconsistent);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_rs);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_ra);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_ns);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_REDIRECT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_redirect);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_OVERFLOW_FAILED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->overflow_failed);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_miss);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS_CONFLICT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_miss_conflict);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_static_conflict);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ALLOCS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->allocs);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_DESTROYS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->destroys);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ERROR, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->error);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_INTF_INVALID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->intf_invalid);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RES_FAILED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->res_failed);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_MCAST, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_probes_mcast);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_UCAST, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->rcv_probes_ucast);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_SOLICITED_NA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->snd_solicited_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_UNSOLICITED_NA, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->snd_unsolicited_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_DAD_NS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nd_stats->snd_dad_ns);
    }
    return str;
}

char**
tbl_nd_stats_table_dump(tbl_nd_stats_t *p_nd_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nd_stats_key_name_dump(p_nd_stats, buf));
    for(i=1; i<TBL_ND_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nd_stats_field_name_dump(p_nd_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nd_stats_field_value_dump(p_nd_stats, i, buf));
    }
    return str;
}

int32
tbl_nd_stats_field_value_parser(char *str, int32 field_id, tbl_nd_stats_t *p_nd_stats)
{
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV, field_id))
    {
        int32 ret;
        p_nd_stats->rcv = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_ERROR, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_error = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_TOOSHORT, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_tooshort = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INVALID, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_invalid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INCONSISTENT, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_inconsistent = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RS, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_rs = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RA, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_ra = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NS, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_ns = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NA, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_na = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_REDIRECT, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_redirect = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_OVERFLOW_FAILED, field_id))
    {
        int32 ret;
        p_nd_stats->overflow_failed = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_miss = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS_CONFLICT, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_miss_conflict = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_static_conflict = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ALLOCS, field_id))
    {
        int32 ret;
        p_nd_stats->allocs = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_DESTROYS, field_id))
    {
        int32 ret;
        p_nd_stats->destroys = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ERROR, field_id))
    {
        int32 ret;
        p_nd_stats->error = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_INTF_INVALID, field_id))
    {
        int32 ret;
        p_nd_stats->intf_invalid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RES_FAILED, field_id))
    {
        int32 ret;
        p_nd_stats->res_failed = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_MCAST, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_probes_mcast = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_UCAST, field_id))
    {
        int32 ret;
        p_nd_stats->rcv_probes_ucast = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_SOLICITED_NA, field_id))
    {
        int32 ret;
        p_nd_stats->snd_solicited_na = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_UNSOLICITED_NA, field_id))
    {
        int32 ret;
        p_nd_stats->snd_unsolicited_na = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_DAD_NS, field_id))
    {
        int32 ret;
        p_nd_stats->snd_dad_ns = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nd_stats_table_parser(char** array, char* key_value,tbl_nd_stats_t *p_nd_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nd_stats_key_str2val(key_value, p_nd_stats));

    for(i=1; i<TBL_ND_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nd_stats_field_value_parser( array[j++], i, p_nd_stats));
    }

    return PM_E_NONE;
}

int32
tbl_nd_stats_dump_one(tbl_nd_stats_t *p_nd_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV].name,
            p_nd_stats->rcv);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_ERROR].name,
            p_nd_stats->rcv_error);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_TOOSHORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_TOOSHORT].name,
            p_nd_stats->rcv_tooshort);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INVALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_INVALID].name,
            p_nd_stats->rcv_invalid);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_INCONSISTENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_INCONSISTENT].name,
            p_nd_stats->rcv_inconsistent);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_RS].name,
            p_nd_stats->rcv_rs);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_RA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_RA].name,
            p_nd_stats->rcv_ra);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_NS].name,
            p_nd_stats->rcv_ns);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_NA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_NA].name,
            p_nd_stats->rcv_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_REDIRECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_REDIRECT].name,
            p_nd_stats->rcv_redirect);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_OVERFLOW_FAILED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_OVERFLOW_FAILED].name,
            p_nd_stats->overflow_failed);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_MISS].name,
            p_nd_stats->rcv_miss);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_MISS_CONFLICT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_MISS_CONFLICT].name,
            p_nd_stats->rcv_miss_conflict);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT].name,
            p_nd_stats->rcv_static_conflict);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ALLOCS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_ALLOCS].name,
            p_nd_stats->allocs);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_DESTROYS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_DESTROYS].name,
            p_nd_stats->destroys);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_ERROR].name,
            p_nd_stats->error);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_INTF_INVALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_INTF_INVALID].name,
            p_nd_stats->intf_invalid);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RES_FAILED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RES_FAILED].name,
            p_nd_stats->res_failed);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_MCAST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_PROBES_MCAST].name,
            p_nd_stats->rcv_probes_mcast);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_RCV_PROBES_UCAST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_RCV_PROBES_UCAST].name,
            p_nd_stats->rcv_probes_ucast);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_SOLICITED_NA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_SND_SOLICITED_NA].name,
            p_nd_stats->snd_solicited_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_UNSOLICITED_NA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_SND_UNSOLICITED_NA].name,
            p_nd_stats->snd_unsolicited_na);
    }
    if (FLD_MATCH(TBL_ND_STATS_FLD_SND_DAD_NS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ND_STATS_FLD_SND_DAD_NS].name,
            p_nd_stats->snd_dad_ns);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nd_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nd_stats_t *p_db_nd_stats = _g_p_tbl_nd_stats;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_nd_stats)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_nd_stats, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nd_stats_t*
tbl_nd_stats_init_nd_stats()
{
    _g_p_tbl_nd_stats = XCALLOC(MEM_TBL_ND_STATS, sizeof(tbl_nd_stats_t));
    return _g_p_tbl_nd_stats;
}


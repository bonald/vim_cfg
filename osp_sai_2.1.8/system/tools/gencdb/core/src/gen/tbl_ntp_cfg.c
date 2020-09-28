
#include "proto.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_cfg.h"
#include "cdb_data_cmp.h"

tbl_ntp_cfg_t *_g_p_tbl_ntp_cfg = NULL;

int32
tbl_ntp_cfg_set_ntp_cfg_field_sync(tbl_ntp_cfg_t *p_ntp_cfg, tbl_ntp_cfg_field_id_t field_id, uint32 sync)
{
    tbl_ntp_cfg_t *p_db_ntp_cfg = _g_p_tbl_ntp_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_CFG_FLD_ACLCOUNT:
        p_db_ntp_cfg->aclCount = p_ntp_cfg->aclCount;
        break;

    case TBL_NTP_CFG_FLD_UCASTSERVERCOUNT:
        p_db_ntp_cfg->ucastServerCount = p_ntp_cfg->ucastServerCount;
        break;

    case TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT:
        p_db_ntp_cfg->ucastClientCount = p_ntp_cfg->ucastClientCount;
        break;

    case TBL_NTP_CFG_FLD_BCASTSERVERCOUNT:
        p_db_ntp_cfg->bcastServerCount = p_ntp_cfg->bcastServerCount;
        break;

    case TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT:
        p_db_ntp_cfg->mcastClientCount = p_ntp_cfg->mcastClientCount;
        break;

    case TBL_NTP_CFG_FLD_MCASTSERVERCOUNT:
        p_db_ntp_cfg->mcastServerCount = p_ntp_cfg->mcastServerCount;
        break;

    case TBL_NTP_CFG_FLD_BCASTDELAY:
        p_db_ntp_cfg->bcastDelay = p_ntp_cfg->bcastDelay;
        break;

    case TBL_NTP_CFG_FLD_MINIMUMDISTANCE:
        p_db_ntp_cfg->minimumDistance = p_ntp_cfg->minimumDistance;
        break;

    case TBL_NTP_CFG_FLD_AUTHENTICATION:
        p_db_ntp_cfg->authentication = p_ntp_cfg->authentication;
        break;

    case TBL_NTP_CFG_FLD_BREFCLOCK:
        p_db_ntp_cfg->brefclock = p_ntp_cfg->brefclock;
        break;

    case TBL_NTP_CFG_FLD_REFCLOCK_STRATUM:
        p_db_ntp_cfg->refclock_stratum = p_ntp_cfg->refclock_stratum;
        break;

    case TBL_NTP_CFG_FLD_IFRELOADCOUNT:
        p_db_ntp_cfg->ifreloadCount = p_ntp_cfg->ifreloadCount;
        break;

    case TBL_NTP_CFG_FLD_CLEARSTATS:
        p_db_ntp_cfg->clearStats = p_ntp_cfg->clearStats;
        break;

    case TBL_NTP_CFG_FLD_DEBUG_LEVEL:
        p_db_ntp_cfg->debug_level = p_ntp_cfg->debug_level;
        break;

    case TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND:
        p_db_ntp_cfg->ucastServerCount_inband = p_ntp_cfg->ucastServerCount_inband;
        break;

    case TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND:
        p_db_ntp_cfg->ucastServerCount_outband = p_ntp_cfg->ucastServerCount_outband;
        break;

    case TBL_NTP_CFG_FLD_MAX:
        sal_memcpy(p_db_ntp_cfg, p_ntp_cfg, sizeof(tbl_ntp_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_CFG, field_id, p_db_ntp_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_CFG, field_id, p_db_ntp_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_cfg_set_ntp_cfg_field(tbl_ntp_cfg_t *p_ntp_cfg, tbl_ntp_cfg_field_id_t field_id)
{
    return tbl_ntp_cfg_set_ntp_cfg_field_sync(p_ntp_cfg, field_id, TRUE);
}

tbl_ntp_cfg_t*
tbl_ntp_cfg_get_ntp_cfg()
{
    return _g_p_tbl_ntp_cfg;
}

int32
tbl_ntp_cfg_key_str2val(char *str, tbl_ntp_cfg_t *p_ntp_cfg)
{
    return PM_E_NONE;
}

char*
tbl_ntp_cfg_key_name_dump(tbl_ntp_cfg_t *p_ntp_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ntp_cfg_key_value_dump(tbl_ntp_cfg_t *p_ntp_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ntp_cfg_field_name_dump(tbl_ntp_cfg_t *p_ntp_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_cfg_field_value_dump(tbl_ntp_cfg_t *p_ntp_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_CFG_FLD_ACLCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->aclCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->ucastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->ucastClientCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTSERVERCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->bcastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->mcastClientCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTSERVERCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->mcastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTDELAY, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->bcastDelay);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MINIMUMDISTANCE, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->minimumDistance);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_AUTHENTICATION, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->authentication);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BREFCLOCK, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->brefclock);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_REFCLOCK_STRATUM, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->refclock_stratum);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_IFRELOADCOUNT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->ifreloadCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_CLEARSTATS, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->clearStats);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_DEBUG_LEVEL, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->debug_level);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->ucastServerCount_inband);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_cfg->ucastServerCount_outband);
    }
    return str;
}

char**
tbl_ntp_cfg_table_dump(tbl_ntp_cfg_t *p_ntp_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_cfg_key_name_dump(p_ntp_cfg, buf));
    for(i=1; i<TBL_NTP_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_cfg_field_name_dump(p_ntp_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_cfg_field_value_dump(p_ntp_cfg, i, buf));
    }
    return str;
}

int32
tbl_ntp_cfg_field_value_parser(char *str, int32 field_id, tbl_ntp_cfg_t *p_ntp_cfg)
{
    if (FLD_MATCH(TBL_NTP_CFG_FLD_ACLCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTSERVERCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTSERVERCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTDELAY, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MINIMUMDISTANCE, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_AUTHENTICATION, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BREFCLOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_REFCLOCK_STRATUM, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_IFRELOADCOUNT, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_CLEARSTATS, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_DEBUG_LEVEL, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_ntp_cfg_table_parser(char** array, char* key_value,tbl_ntp_cfg_t *p_ntp_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_cfg_key_str2val(key_value, p_ntp_cfg));

    for(i=1; i<TBL_NTP_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_cfg_field_value_parser( array[j++], i, p_ntp_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_cfg_dump_one(tbl_ntp_cfg_t *p_ntp_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_CFG_FLD_ACLCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_ACLCOUNT].name,
            p_ntp_cfg->aclCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_UCASTSERVERCOUNT].name,
            p_ntp_cfg->ucastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT].name,
            p_ntp_cfg->ucastClientCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTSERVERCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_BCASTSERVERCOUNT].name,
            p_ntp_cfg->bcastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT].name,
            p_ntp_cfg->mcastClientCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MCASTSERVERCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_MCASTSERVERCOUNT].name,
            p_ntp_cfg->mcastServerCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BCASTDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_BCASTDELAY].name,
            p_ntp_cfg->bcastDelay);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_MINIMUMDISTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_MINIMUMDISTANCE].name,
            p_ntp_cfg->minimumDistance);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_AUTHENTICATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_AUTHENTICATION].name,
            p_ntp_cfg->authentication);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_BREFCLOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_BREFCLOCK].name,
            p_ntp_cfg->brefclock);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_REFCLOCK_STRATUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_REFCLOCK_STRATUM].name,
            p_ntp_cfg->refclock_stratum);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_IFRELOADCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_IFRELOADCOUNT].name,
            p_ntp_cfg->ifreloadCount);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_CLEARSTATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_CLEARSTATS].name,
            p_ntp_cfg->clearStats);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_DEBUG_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_DEBUG_LEVEL].name,
            p_ntp_cfg->debug_level);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND].name,
            p_ntp_cfg->ucastServerCount_inband);
    }
    if (FLD_MATCH(TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND].name,
            p_ntp_cfg->ucastServerCount_outband);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_cfg_t *p_db_ntp_cfg = _g_p_tbl_ntp_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ntp_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ntp_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_cfg_t*
tbl_ntp_cfg_init_ntp_cfg()
{
    _g_p_tbl_ntp_cfg = XCALLOC(MEM_TBL_NTP_CFG, sizeof(tbl_ntp_cfg_t));
    return _g_p_tbl_ntp_cfg;
}


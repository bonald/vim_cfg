
#include "proto.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_app_debug.h"
#include "cdb_data_cmp.h"

tbl_app_debug_t *_g_p_tbl_app_debug = NULL;

int32
tbl_app_debug_set_app_debug_field_sync(tbl_app_debug_t *p_appdbg, tbl_app_debug_field_id_t field_id, uint32 sync)
{
    tbl_app_debug_t *p_db_appdbg = _g_p_tbl_app_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_APP_DEBUG_FLD_SNMP_AGENT:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, snmp, APPDBG_FLAG_SNMP_AGENT)
        break;

    case TBL_APP_DEBUG_FLD_RMON_EVENT:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, rmon, APPDBG_FLAG_RMON_EVENT)
        break;

    case TBL_APP_DEBUG_FLD_RMON_ALARM:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, rmon, APPDBG_FLAG_RMON_ALARM)
        break;

    case TBL_APP_DEBUG_FLD_RMON_STATS:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, rmon, APPDBG_FLAG_RMON_STATS)
        break;

    case TBL_APP_DEBUG_FLD_RMON_HISTORY:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, rmon, APPDBG_FLAG_RMON_HISTORY)
        break;

    case TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, ntp, APPDBG_FLAG_NTP_DEBUG_LEVEL)
        break;

    case TBL_APP_DEBUG_FLD_NETCONF_RPC:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, netconf, APPDBG_FLAG_NETCONF_RPC)
        break;

    case TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, netconf, APPDBG_FLAG_NETCONF_RPC_CLEAR)
        break;

    case TBL_APP_DEBUG_FLD_NETCONF_DEBUG:
        CDB_FLAG_CHECK_SET(p_db_appdbg, p_appdbg, netconf, APPDBG_FLAG_NETCONF_DEBUG)
        break;

    case TBL_APP_DEBUG_FLD_MAX:
        sal_memcpy(p_db_appdbg, p_appdbg, sizeof(tbl_app_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_APP_DEBUG, field_id, p_db_appdbg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_APP_DEBUG, field_id, p_db_appdbg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_app_debug_set_app_debug_field(tbl_app_debug_t *p_appdbg, tbl_app_debug_field_id_t field_id)
{
    return tbl_app_debug_set_app_debug_field_sync(p_appdbg, field_id, TRUE);
}

tbl_app_debug_t*
tbl_app_debug_get_app_debug()
{
    return _g_p_tbl_app_debug;
}

int32
tbl_app_debug_key_str2val(char *str, tbl_app_debug_t *p_appdbg)
{
    return PM_E_NONE;
}

char*
tbl_app_debug_key_name_dump(tbl_app_debug_t *p_appdbg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_APP_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_app_debug_key_value_dump(tbl_app_debug_t *p_appdbg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_app_debug_field_name_dump(tbl_app_debug_t *p_appdbg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_APP_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_APP_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_app_debug_field_value_dump(tbl_app_debug_t *p_appdbg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_SNMP_AGENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->snmp, APPDBG_FLAG_SNMP_AGENT));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_EVENT));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_ALARM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_ALARM));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_STATS));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_HISTORY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_HISTORY));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->ntp, APPDBG_FLAG_NTP_DEBUG_LEVEL));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC_CLEAR));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_DEBUG, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_DEBUG));
    }
    return str;
}

char**
tbl_app_debug_table_dump(tbl_app_debug_t *p_appdbg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_app_debug_key_name_dump(p_appdbg, buf));
    for(i=1; i<TBL_APP_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_app_debug_field_name_dump(p_appdbg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_app_debug_field_value_dump(p_appdbg, i, buf));
    }
    return str;
}

int32
tbl_app_debug_field_value_parser(char *str, int32 field_id, tbl_app_debug_t *p_appdbg)
{
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_SNMP_AGENT, field_id))
    {
        GLB_SET_FLAG(p_appdbg->snmp, APPDBG_FLAG_SNMP_AGENT);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_EVENT, field_id))
    {
        GLB_SET_FLAG(p_appdbg->rmon, APPDBG_FLAG_RMON_EVENT);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_ALARM, field_id))
    {
        GLB_SET_FLAG(p_appdbg->rmon, APPDBG_FLAG_RMON_ALARM);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_STATS, field_id))
    {
        GLB_SET_FLAG(p_appdbg->rmon, APPDBG_FLAG_RMON_STATS);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_HISTORY, field_id))
    {
        GLB_SET_FLAG(p_appdbg->rmon, APPDBG_FLAG_RMON_HISTORY);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL, field_id))
    {
        GLB_SET_FLAG(p_appdbg->ntp, APPDBG_FLAG_NTP_DEBUG_LEVEL);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC, field_id))
    {
        GLB_SET_FLAG(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR, field_id))
    {
        GLB_SET_FLAG(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC_CLEAR);
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_DEBUG, field_id))
    {
        GLB_SET_FLAG(p_appdbg->netconf, APPDBG_FLAG_NETCONF_DEBUG);
    }
    return PM_E_NONE;
}

int32
tbl_app_debug_table_parser(char** array, char* key_value,tbl_app_debug_t *p_appdbg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_APP_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_app_debug_key_str2val(key_value, p_appdbg));

    for(i=1; i<TBL_APP_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_app_debug_field_value_parser( array[j++], i, p_appdbg));
    }

    return PM_E_NONE;
}

int32
tbl_app_debug_dump_one(tbl_app_debug_t *p_appdbg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_APP_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_APP_DEBUG_FLD_SNMP_AGENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_SNMP_AGENT].name,
            GLB_FLAG_ISSET(p_appdbg->snmp, APPDBG_FLAG_SNMP_AGENT));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_RMON_EVENT].name,
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_EVENT));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_ALARM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_RMON_ALARM].name,
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_ALARM));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_RMON_STATS].name,
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_STATS));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_RMON_HISTORY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_RMON_HISTORY].name,
            GLB_FLAG_ISSET(p_appdbg->rmon, APPDBG_FLAG_RMON_HISTORY));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL].name,
            GLB_FLAG_ISSET(p_appdbg->ntp, APPDBG_FLAG_NTP_DEBUG_LEVEL));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_NETCONF_RPC].name,
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR].name,
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_RPC_CLEAR));
    }
    if (FLD_MATCH(TBL_APP_DEBUG_FLD_NETCONF_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_APP_DEBUG_FLD_NETCONF_DEBUG].name,
            GLB_FLAG_ISSET(p_appdbg->netconf, APPDBG_FLAG_NETCONF_DEBUG));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_app_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_app_debug_t *p_db_appdbg = _g_p_tbl_app_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_appdbg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_appdbg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_app_debug_t*
tbl_app_debug_init_app_debug()
{
    _g_p_tbl_app_debug = XCALLOC(MEM_TBL_APP_DEBUG, sizeof(tbl_app_debug_t));
    return _g_p_tbl_app_debug;
}


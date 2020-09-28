
#include "proto.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "cdb_data_cmp.h"

tbl_snmp_cfg_t *_g_p_tbl_snmp_cfg = NULL;

int32
tbl_snmp_cfg_set_snmp_cfg_field_sync(tbl_snmp_cfg_t *p_snmp_cfg, tbl_snmp_cfg_field_id_t field_id, uint32 sync)
{
    tbl_snmp_cfg_t *p_db_snmp_cfg = _g_p_tbl_snmp_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_CFG_FLD_ENABLE:
        p_db_snmp_cfg->enable = p_snmp_cfg->enable;
        break;

    case TBL_SNMP_CFG_FLD_SERVER_ENABLE:
        p_db_snmp_cfg->server_enable = p_snmp_cfg->server_enable;
        break;

    case TBL_SNMP_CFG_FLD_VERSION:
        p_db_snmp_cfg->version = p_snmp_cfg->version;
        break;

    case TBL_SNMP_CFG_FLD_ENGINEID:
        sal_memcpy(p_db_snmp_cfg->engineid, p_snmp_cfg->engineid, sizeof(p_snmp_cfg->engineid));
        break;

    case TBL_SNMP_CFG_FLD_CONTACT:
        sal_memcpy(&p_db_snmp_cfg->contact, &p_snmp_cfg->contact, sizeof(p_snmp_cfg->contact));
        break;

    case TBL_SNMP_CFG_FLD_LOCATION:
        sal_memcpy(&p_db_snmp_cfg->location, &p_snmp_cfg->location, sizeof(p_snmp_cfg->location));
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_COLDSTART)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_WARMSTART)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_LINKDOWN)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_LINKUP)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_SYSTEM)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_VRRP)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE)
        break;

    case TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL:
        CDB_FLAG_CHECK_SET(p_db_snmp_cfg, p_snmp_cfg, trap_enable, GLB_TRAP_TYPE_ALL)
        break;

    case TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT:
        p_db_snmp_cfg->watch_restart_count = p_snmp_cfg->watch_restart_count;
        break;

    case TBL_SNMP_CFG_FLD_AGENT_STATS:
        p_db_snmp_cfg->agent_stats = p_snmp_cfg->agent_stats;
        break;

    case TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER:
        p_db_snmp_cfg->t_snmp_trap_all_timer = p_snmp_cfg->t_snmp_trap_all_timer;
        break;

    case TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER:
        p_db_snmp_cfg->t_snmp_enable_timer = p_snmp_cfg->t_snmp_enable_timer;
        break;

    case TBL_SNMP_CFG_FLD_MAX:
        sal_memcpy(p_db_snmp_cfg, p_snmp_cfg, sizeof(tbl_snmp_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_CFG, field_id, p_db_snmp_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_CFG, field_id, p_db_snmp_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_cfg_set_snmp_cfg_field(tbl_snmp_cfg_t *p_snmp_cfg, tbl_snmp_cfg_field_id_t field_id)
{
    return tbl_snmp_cfg_set_snmp_cfg_field_sync(p_snmp_cfg, field_id, TRUE);
}

tbl_snmp_cfg_t*
tbl_snmp_cfg_get_snmp_cfg()
{
    return _g_p_tbl_snmp_cfg;
}

int32
tbl_snmp_cfg_key_str2val(char *str, tbl_snmp_cfg_t *p_snmp_cfg)
{
    return PM_E_NONE;
}

char*
tbl_snmp_cfg_key_name_dump(tbl_snmp_cfg_t *p_snmp_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_snmp_cfg_key_value_dump(tbl_snmp_cfg_t *p_snmp_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_snmp_cfg_field_name_dump(tbl_snmp_cfg_t *p_snmp_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_cfg_field_value_dump(tbl_snmp_cfg_t *p_snmp_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_cfg->enable);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_SERVER_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_cfg->server_enable);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(snmp_version_strs, SNMP_VERSION_MAX, p_snmp_cfg->version));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENGINEID, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_cfg->engineid);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_CONTACT, field_id))
    {
        cdb_snmp_info_val2str(str, MAX_CMD_STR_LEN, &p_snmp_cfg->contact);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_LOCATION, field_id))
    {
        cdb_snmp_info_val2str(str, MAX_CMD_STR_LEN, &p_snmp_cfg->location);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_COLDSTART));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_WARMSTART));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKDOWN));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKUP));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_SYSTEM));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_VRRP));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_ALL));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_cfg->watch_restart_count);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_AGENT_STATS, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_cfg->agent_stats);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_snmp_cfg->t_snmp_trap_all_timer);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_snmp_cfg->t_snmp_enable_timer);
    }
    return str;
}

char**
tbl_snmp_cfg_table_dump(tbl_snmp_cfg_t *p_snmp_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_cfg_key_name_dump(p_snmp_cfg, buf));
    for(i=1; i<TBL_SNMP_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_cfg_field_name_dump(p_snmp_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_cfg_field_value_dump(p_snmp_cfg, i, buf));
    }
    return str;
}

int32
tbl_snmp_cfg_field_value_parser(char *str, int32 field_id, tbl_snmp_cfg_t *p_snmp_cfg)
{
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_snmp_cfg->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_SERVER_ENABLE, field_id))
    {
        int32 ret;
        p_snmp_cfg->server_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_VERSION, field_id))
    {
        cdb_enum_str2val(snmp_version_strs, SNMP_VERSION_MAX, str);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENGINEID, field_id))
    {
        sal_strcpy(p_snmp_cfg->engineid, str);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_CONTACT, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_snmp_info_str2val(&p_snmp_cfg->contact, str, err);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_LOCATION, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_snmp_info_str2val(&p_snmp_cfg->location, str, err);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_COLDSTART);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_WARMSTART);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKDOWN);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKUP);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_SYSTEM);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_VRRP);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL, field_id))
    {
        GLB_SET_FLAG(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_ALL);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT, field_id))
    {
        int32 ret;
        p_snmp_cfg->watch_restart_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_AGENT_STATS, field_id))
    {
        int32 ret;
        p_snmp_cfg->agent_stats = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER, field_id))
    {
        ctc_task_str2val(str, p_snmp_cfg->t_snmp_trap_all_timer);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_snmp_cfg->t_snmp_enable_timer);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_cfg_table_parser(char** array, char* key_value,tbl_snmp_cfg_t *p_snmp_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_cfg_key_str2val(key_value, p_snmp_cfg));

    for(i=1; i<TBL_SNMP_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_cfg_field_value_parser( array[j++], i, p_snmp_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_cfg_dump_one(tbl_snmp_cfg_t *p_snmp_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_ENABLE].name,
            p_snmp_cfg->enable);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_SERVER_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_SERVER_ENABLE].name,
            p_snmp_cfg->server_enable);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_VERSION].name, 
            cdb_enum_val2str(snmp_version_strs, SNMP_VERSION_MAX, p_snmp_cfg->version));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_ENGINEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_ENGINEID].name,
            p_snmp_cfg->engineid);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_CONTACT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_CONTACT].name, 
            cdb_snmp_info_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_cfg->contact));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_LOCATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_LOCATION].name, 
            cdb_snmp_info_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_cfg->location));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_COLDSTART));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_WARMSTART));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKDOWN));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_LINKUP));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_SYSTEM));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_VRRP));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL].name,
            GLB_FLAG_ISSET(p_snmp_cfg->trap_enable, GLB_TRAP_TYPE_ALL));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT].name,
            p_snmp_cfg->watch_restart_count);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_AGENT_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_CFG_FLD_AGENT_STATS].name,
            p_snmp_cfg->agent_stats);
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_snmp_cfg->t_snmp_trap_all_timer));
    }
    if (FLD_MATCH(TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_snmp_cfg->t_snmp_enable_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_cfg_t *p_db_snmp_cfg = _g_p_tbl_snmp_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_snmp_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_snmp_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_cfg_t*
tbl_snmp_cfg_init_snmp_cfg()
{
    _g_p_tbl_snmp_cfg = XCALLOC(MEM_TBL_SNMP_CFG, sizeof(tbl_snmp_cfg_t));
    return _g_p_tbl_snmp_cfg;
}


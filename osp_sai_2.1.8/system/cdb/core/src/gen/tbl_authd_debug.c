
#include "proto.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_authd_debug.h"
#include "cdb_data_cmp.h"

tbl_authd_debug_t *_g_p_tbl_authd_debug = NULL;

int32
tbl_authd_debug_set_authd_debug_field_sync(tbl_authd_debug_t *p_authd_debug, tbl_authd_debug_field_id_t field_id, uint32 sync)
{
    tbl_authd_debug_t *p_db_authd_debug = _g_p_tbl_authd_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTHD_DEBUG_FLD_AUTH_EVENT:
        CDB_FLAG_CHECK_SET(p_db_authd_debug, p_authd_debug, flags, AUTHD_DBG_FLAG_EVENT)
        break;

    case TBL_AUTHD_DEBUG_FLD_AUTH_PACKET:
        CDB_FLAG_CHECK_SET(p_db_authd_debug, p_authd_debug, flags, AUTHD_DBG_FLAG_PACKET)
        break;

    case TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_authd_debug, p_authd_debug, flags, AUTHD_DBG_FLAG_PROTO)
        break;

    case TBL_AUTHD_DEBUG_FLD_AUTH_TIMER:
        CDB_FLAG_CHECK_SET(p_db_authd_debug, p_authd_debug, flags, AUTHD_DBG_FLAG_TIMER)
        break;

    case TBL_AUTHD_DEBUG_FLD_MAX:
        sal_memcpy(p_db_authd_debug, p_authd_debug, sizeof(tbl_authd_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTHD_DEBUG, field_id, p_db_authd_debug);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTHD_DEBUG, field_id, p_db_authd_debug);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_authd_debug_set_authd_debug_field(tbl_authd_debug_t *p_authd_debug, tbl_authd_debug_field_id_t field_id)
{
    return tbl_authd_debug_set_authd_debug_field_sync(p_authd_debug, field_id, TRUE);
}

tbl_authd_debug_t*
tbl_authd_debug_get_authd_debug()
{
    return _g_p_tbl_authd_debug;
}

int32
tbl_authd_debug_key_str2val(char *str, tbl_authd_debug_t *p_authd_debug)
{
    return PM_E_NONE;
}

char*
tbl_authd_debug_key_name_dump(tbl_authd_debug_t *p_authd_debug, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_authd_debug_key_value_dump(tbl_authd_debug_t *p_authd_debug, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_authd_debug_field_name_dump(tbl_authd_debug_t *p_authd_debug, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTHD_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_authd_debug_field_value_dump(tbl_authd_debug_t *p_authd_debug, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_PROTO));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_TIMER));
    }
    return str;
}

char**
tbl_authd_debug_table_dump(tbl_authd_debug_t *p_authd_debug, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_authd_debug_key_name_dump(p_authd_debug, buf));
    for(i=1; i<TBL_AUTHD_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_authd_debug_field_name_dump(p_authd_debug, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_authd_debug_field_value_dump(p_authd_debug, i, buf));
    }
    return str;
}

int32
tbl_authd_debug_field_value_parser(char *str, int32 field_id, tbl_authd_debug_t *p_authd_debug)
{
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_EVENT, field_id))
    {
        GLB_SET_FLAG(p_authd_debug->flags, AUTHD_DBG_FLAG_EVENT);
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PACKET, field_id))
    {
        GLB_SET_FLAG(p_authd_debug->flags, AUTHD_DBG_FLAG_PACKET);
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_authd_debug->flags, AUTHD_DBG_FLAG_PROTO);
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_TIMER, field_id))
    {
        GLB_SET_FLAG(p_authd_debug->flags, AUTHD_DBG_FLAG_TIMER);
    }
    return PM_E_NONE;
}

int32
tbl_authd_debug_table_parser(char** array, char* key_value,tbl_authd_debug_t *p_authd_debug)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_authd_debug_key_str2val(key_value, p_authd_debug));

    for(i=1; i<TBL_AUTHD_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_authd_debug_field_value_parser( array[j++], i, p_authd_debug));
    }

    return PM_E_NONE;
}

int32
tbl_authd_debug_dump_one(tbl_authd_debug_t *p_authd_debug, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHD_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTHD_DEBUG_FLD_AUTH_EVENT].name,
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_EVENT));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTHD_DEBUG_FLD_AUTH_PACKET].name,
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL].name,
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_PROTO));
    }
    if (FLD_MATCH(TBL_AUTHD_DEBUG_FLD_AUTH_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTHD_DEBUG_FLD_AUTH_TIMER].name,
            GLB_FLAG_ISSET(p_authd_debug->flags, AUTHD_DBG_FLAG_TIMER));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_authd_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_authd_debug_t *p_db_authd_debug = _g_p_tbl_authd_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_authd_debug)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_authd_debug, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_authd_debug_t*
tbl_authd_debug_init_authd_debug()
{
    _g_p_tbl_authd_debug = XCALLOC(MEM_TBL_AUTHD_DEBUG, sizeof(tbl_authd_debug_t));
    return _g_p_tbl_authd_debug;
}


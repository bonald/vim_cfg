
#include "proto.h"
#include "gen/tbl_dhcpv6_debug_define.h"
#include "gen/tbl_dhcpv6_debug.h"
#include "cdb_data_cmp.h"

tbl_dhcpv6_debug_t *_g_p_tbl_dhcpv6_debug = NULL;

int32
tbl_dhcpv6_debug_set_dhcpv6_debug_field_sync(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_dhcpv6_debug_field_id_t field_id, uint32 sync)
{
    tbl_dhcpv6_debug_t *p_db_dhcpv6_debug = _g_p_tbl_dhcpv6_debug;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR:
        CDB_FLAG_CHECK_SET(p_db_dhcpv6_debug, p_dhcpv6_debug, flags, DHCPV6DBG_FLAG_CLIENTV6_ERROR)
        break;

    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS:
        CDB_FLAG_CHECK_SET(p_db_dhcpv6_debug, p_dhcpv6_debug, flags, DHCPV6DBG_FLAG_CLIENTV6_EVENTS)
        break;

    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET:
        CDB_FLAG_CHECK_SET(p_db_dhcpv6_debug, p_dhcpv6_debug, flags, DHCPV6DBG_FLAG_CLIENTV6_PACKET)
        break;

    case TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP:
        CDB_FLAG_CHECK_SET(p_db_dhcpv6_debug, p_dhcpv6_debug, flags, DHCPV6DBG_FLAG_CLIENTV6_DUMP)
        break;

    case TBL_DHCPV6_DEBUG_FLD_MAX:
        sal_memcpy(p_db_dhcpv6_debug, p_dhcpv6_debug, sizeof(tbl_dhcpv6_debug_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCPV6_DEBUG, field_id, p_db_dhcpv6_debug);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCPV6_DEBUG, field_id, p_db_dhcpv6_debug);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcpv6_debug_set_dhcpv6_debug_field(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_dhcpv6_debug_field_id_t field_id)
{
    return tbl_dhcpv6_debug_set_dhcpv6_debug_field_sync(p_dhcpv6_debug, field_id, TRUE);
}

tbl_dhcpv6_debug_t*
tbl_dhcpv6_debug_get_dhcpv6_debug()
{
    return _g_p_tbl_dhcpv6_debug;
}

int32
tbl_dhcpv6_debug_key_str2val(char *str, tbl_dhcpv6_debug_t *p_dhcpv6_debug)
{
    return PM_E_NONE;
}

char*
tbl_dhcpv6_debug_key_name_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_dhcpv6_debug_key_value_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_dhcpv6_debug_field_name_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCPV6_DEBUG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhcpv6_debug_field_value_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_ERROR));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_EVENTS));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_PACKET));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_DUMP));
    }
    return str;
}

char**
tbl_dhcpv6_debug_table_dump(tbl_dhcpv6_debug_t *p_dhcpv6_debug, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhcpv6_debug_key_name_dump(p_dhcpv6_debug, buf));
    for(i=1; i<TBL_DHCPV6_DEBUG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhcpv6_debug_field_name_dump(p_dhcpv6_debug, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhcpv6_debug_field_value_dump(p_dhcpv6_debug, i, buf));
    }
    return str;
}

int32
tbl_dhcpv6_debug_field_value_parser(char *str, int32 field_id, tbl_dhcpv6_debug_t *p_dhcpv6_debug)
{
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR, field_id))
    {
        GLB_SET_FLAG(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_ERROR);
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS, field_id))
    {
        GLB_SET_FLAG(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_EVENTS);
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET, field_id))
    {
        GLB_SET_FLAG(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_PACKET);
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP, field_id))
    {
        GLB_SET_FLAG(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_DUMP);
    }
    return PM_E_NONE;
}

int32
tbl_dhcpv6_debug_table_parser(char** array, char* key_value,tbl_dhcpv6_debug_t *p_dhcpv6_debug)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhcpv6_debug_key_str2val(key_value, p_dhcpv6_debug));

    for(i=1; i<TBL_DHCPV6_DEBUG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhcpv6_debug_field_value_parser( array[j++], i, p_dhcpv6_debug));
    }

    return PM_E_NONE;
}

int32
tbl_dhcpv6_debug_dump_one(tbl_dhcpv6_debug_t *p_dhcpv6_debug, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCPV6_DEBUG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR].name,
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_ERROR));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS].name,
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_EVENTS));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET].name,
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_PACKET));
    }
    if (FLD_MATCH(TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP].name,
            GLB_FLAG_ISSET(p_dhcpv6_debug->flags, DHCPV6DBG_FLAG_CLIENTV6_DUMP));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhcpv6_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhcpv6_debug_t *p_db_dhcpv6_debug = _g_p_tbl_dhcpv6_debug;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dhcpv6_debug)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dhcpv6_debug, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhcpv6_debug_t*
tbl_dhcpv6_debug_init_dhcpv6_debug()
{
    _g_p_tbl_dhcpv6_debug = XCALLOC(MEM_TBL_DHCPV6_DEBUG, sizeof(tbl_dhcpv6_debug_t));
    return _g_p_tbl_dhcpv6_debug;
}


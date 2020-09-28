
#include "proto.h"
#include "gen/tbl_ntp_if_define.h"
#include "gen/tbl_ntp_if.h"
#include "cdb_data_cmp.h"

tbl_ntp_if_t *_g_p_tbl_ntp_if = NULL;

int32
tbl_ntp_if_set_ntp_if_field_sync(tbl_ntp_if_t *p_ntp_if, tbl_ntp_if_field_id_t field_id, uint32 sync)
{
    tbl_ntp_if_t *p_db_ntp_if = _g_p_tbl_ntp_if;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_IF_FLD_DISABLE:
        p_db_ntp_if->disable = p_ntp_if->disable;
        break;

    case TBL_NTP_IF_FLD_BROADCASTCLIENT:
        p_db_ntp_if->broadcastClient = p_ntp_if->broadcastClient;
        break;

    case TBL_NTP_IF_FLD_MAX:
        sal_memcpy(p_db_ntp_if, p_ntp_if, sizeof(tbl_ntp_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_IF, field_id, p_db_ntp_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_IF, field_id, p_db_ntp_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_if_set_ntp_if_field(tbl_ntp_if_t *p_ntp_if, tbl_ntp_if_field_id_t field_id)
{
    return tbl_ntp_if_set_ntp_if_field_sync(p_ntp_if, field_id, TRUE);
}

tbl_ntp_if_t*
tbl_ntp_if_get_ntp_if()
{
    return _g_p_tbl_ntp_if;
}

int32
tbl_ntp_if_key_str2val(char *str, tbl_ntp_if_t *p_ntp_if)
{
    return PM_E_NONE;
}

char*
tbl_ntp_if_key_name_dump(tbl_ntp_if_t *p_ntp_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ntp_if_key_value_dump(tbl_ntp_if_t *p_ntp_if, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ntp_if_field_name_dump(tbl_ntp_if_t *p_ntp_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_if_field_value_dump(tbl_ntp_if_t *p_ntp_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_IF_FLD_DISABLE, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_if->disable);
    }
    if (FLD_MATCH(TBL_NTP_IF_FLD_BROADCASTCLIENT, field_id))
    {
        sal_sprintf(str, "%d", p_ntp_if->broadcastClient);
    }
    return str;
}

char**
tbl_ntp_if_table_dump(tbl_ntp_if_t *p_ntp_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_if_key_name_dump(p_ntp_if, buf));
    for(i=1; i<TBL_NTP_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_if_field_name_dump(p_ntp_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_if_field_value_dump(p_ntp_if, i, buf));
    }
    return str;
}

int32
tbl_ntp_if_field_value_parser(char *str, int32 field_id, tbl_ntp_if_t *p_ntp_if)
{
    if (FLD_MATCH(TBL_NTP_IF_FLD_DISABLE, field_id))
    {
    }
    if (FLD_MATCH(TBL_NTP_IF_FLD_BROADCASTCLIENT, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_ntp_if_table_parser(char** array, char* key_value,tbl_ntp_if_t *p_ntp_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_if_key_str2val(key_value, p_ntp_if));

    for(i=1; i<TBL_NTP_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_if_field_value_parser( array[j++], i, p_ntp_if));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_if_dump_one(tbl_ntp_if_t *p_ntp_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_IF_FLD_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_IF_FLD_DISABLE].name,
            p_ntp_if->disable);
    }
    if (FLD_MATCH(TBL_NTP_IF_FLD_BROADCASTCLIENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NTP_IF_FLD_BROADCASTCLIENT].name,
            p_ntp_if->broadcastClient);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_if_t *p_db_ntp_if = _g_p_tbl_ntp_if;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ntp_if)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ntp_if, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_if_t*
tbl_ntp_if_init_ntp_if()
{
    _g_p_tbl_ntp_if = XCALLOC(MEM_TBL_NTP_IF, sizeof(tbl_ntp_if_t));
    return _g_p_tbl_ntp_if;
}


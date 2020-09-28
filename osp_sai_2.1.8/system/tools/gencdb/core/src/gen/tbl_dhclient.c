
#include "proto.h"
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
#include "cdb_data_cmp.h"

tbl_dhclient_t *_g_p_tbl_dhclient = NULL;

int32
tbl_dhclient_set_dhclient_field_sync(tbl_dhclient_t *p_dhclient, tbl_dhclient_field_id_t field_id, uint32 sync)
{
    tbl_dhclient_t *p_db_dhclient = _g_p_tbl_dhclient;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCLIENT_FLD_ENABLE:
        p_db_dhclient->enable = p_dhclient->enable;
        break;

    case TBL_DHCLIENT_FLD_DISTANCE:
        p_db_dhclient->distance = p_dhclient->distance;
        break;

    case TBL_DHCLIENT_FLD_BROADCAST:
        p_db_dhclient->broadcast = p_dhclient->broadcast;
        break;

    case TBL_DHCLIENT_FLD_MAX:
        sal_memcpy(p_db_dhclient, p_dhclient, sizeof(tbl_dhclient_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCLIENT, field_id, p_db_dhclient);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCLIENT, field_id, p_db_dhclient);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhclient_set_dhclient_field(tbl_dhclient_t *p_dhclient, tbl_dhclient_field_id_t field_id)
{
    return tbl_dhclient_set_dhclient_field_sync(p_dhclient, field_id, TRUE);
}

tbl_dhclient_t*
tbl_dhclient_get_dhclient()
{
    return _g_p_tbl_dhclient;
}

int32
tbl_dhclient_key_str2val(char *str, tbl_dhclient_t *p_dhclient)
{
    return PM_E_NONE;
}

char*
tbl_dhclient_key_name_dump(tbl_dhclient_t *p_dhclient, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCLIENT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_dhclient_key_value_dump(tbl_dhclient_t *p_dhclient, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_dhclient_field_name_dump(tbl_dhclient_t *p_dhclient, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCLIENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCLIENT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhclient_field_value_dump(tbl_dhclient_t *p_dhclient, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCLIENT_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->enable);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_DISTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->distance);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_BROADCAST, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->broadcast);
    }
    return str;
}

char**
tbl_dhclient_table_dump(tbl_dhclient_t *p_dhclient, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhclient_key_name_dump(p_dhclient, buf));
    for(i=1; i<TBL_DHCLIENT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhclient_field_name_dump(p_dhclient, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhclient_field_value_dump(p_dhclient, i, buf));
    }
    return str;
}

int32
tbl_dhclient_field_value_parser(char *str, int32 field_id, tbl_dhclient_t *p_dhclient)
{
    if (FLD_MATCH(TBL_DHCLIENT_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_dhclient->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_DISTANCE, field_id))
    {
        int32 ret;
        p_dhclient->distance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_BROADCAST, field_id))
    {
        int32 ret;
        p_dhclient->broadcast = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_dhclient_table_parser(char** array, char* key_value,tbl_dhclient_t *p_dhclient)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCLIENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhclient_key_str2val(key_value, p_dhclient));

    for(i=1; i<TBL_DHCLIENT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhclient_field_value_parser( array[j++], i, p_dhclient));
    }

    return PM_E_NONE;
}

int32
tbl_dhclient_dump_one(tbl_dhclient_t *p_dhclient, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCLIENT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_DHCLIENT_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCLIENT_FLD_ENABLE].name,
            p_dhclient->enable);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_DISTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCLIENT_FLD_DISTANCE].name,
            p_dhclient->distance);
    }
    if (FLD_MATCH(TBL_DHCLIENT_FLD_BROADCAST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCLIENT_FLD_BROADCAST].name,
            p_dhclient->broadcast);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhclient_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhclient_t *p_db_dhclient = _g_p_tbl_dhclient;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dhclient)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dhclient, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhclient_t*
tbl_dhclient_init_dhclient()
{
    _g_p_tbl_dhclient = XCALLOC(MEM_TBL_DHCLIENT, sizeof(tbl_dhclient_t));
    return _g_p_tbl_dhclient;
}



#include "proto.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcrelay.h"
#include "cdb_data_cmp.h"

tbl_dhcrelay_t *_g_p_tbl_dhcrelay = NULL;

int32
tbl_dhcrelay_set_dhcrelay_field_sync(tbl_dhcrelay_t *p_dhcrelay, tbl_dhcrelay_field_id_t field_id, uint32 sync)
{
    tbl_dhcrelay_t *p_db_dhcrelay = _g_p_tbl_dhcrelay;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCRELAY_FLD_ENABLE:
        p_db_dhcrelay->enable = p_dhcrelay->enable;
        break;

    case TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH:
        p_db_dhcrelay->drop_agent_mismatch = p_dhcrelay->drop_agent_mismatch;
        break;

    case TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS:
        p_db_dhcrelay->add_agent_options = p_dhcrelay->add_agent_options;
        break;

    case TBL_DHCRELAY_FLD_AGENT_PROC_OPTION:
        p_db_dhcrelay->agent_proc_option = p_dhcrelay->agent_proc_option;
        break;

    case TBL_DHCRELAY_FLD_TRUST_ALL:
        p_db_dhcrelay->trust_all = p_dhcrelay->trust_all;
        break;

    case TBL_DHCRELAY_FLD_MAX:
        sal_memcpy(p_db_dhcrelay, p_dhcrelay, sizeof(tbl_dhcrelay_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCRELAY, field_id, p_db_dhcrelay);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCRELAY, field_id, p_db_dhcrelay);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcrelay_set_dhcrelay_field(tbl_dhcrelay_t *p_dhcrelay, tbl_dhcrelay_field_id_t field_id)
{
    return tbl_dhcrelay_set_dhcrelay_field_sync(p_dhcrelay, field_id, TRUE);
}

tbl_dhcrelay_t*
tbl_dhcrelay_get_dhcrelay()
{
    return _g_p_tbl_dhcrelay;
}

int32
tbl_dhcrelay_key_str2val(char *str, tbl_dhcrelay_t *p_dhcrelay)
{
    return PM_E_NONE;
}

char*
tbl_dhcrelay_key_name_dump(tbl_dhcrelay_t *p_dhcrelay, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCRELAY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_dhcrelay_key_value_dump(tbl_dhcrelay_t *p_dhcrelay, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_dhcrelay_field_name_dump(tbl_dhcrelay_t *p_dhcrelay, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCRELAY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCRELAY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhcrelay_field_value_dump(tbl_dhcrelay_t *p_dhcrelay, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCRELAY_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_dhcrelay->enable);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH, field_id))
    {
        sal_sprintf(str, "%u", p_dhcrelay->drop_agent_mismatch);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_dhcrelay->add_agent_options);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_AGENT_PROC_OPTION, field_id))
    {
        sal_sprintf(str, "%u", p_dhcrelay->agent_proc_option);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_TRUST_ALL, field_id))
    {
        sal_sprintf(str, "%u", p_dhcrelay->trust_all);
    }
    return str;
}

char**
tbl_dhcrelay_table_dump(tbl_dhcrelay_t *p_dhcrelay, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhcrelay_key_name_dump(p_dhcrelay, buf));
    for(i=1; i<TBL_DHCRELAY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhcrelay_field_name_dump(p_dhcrelay, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhcrelay_field_value_dump(p_dhcrelay, i, buf));
    }
    return str;
}

int32
tbl_dhcrelay_field_value_parser(char *str, int32 field_id, tbl_dhcrelay_t *p_dhcrelay)
{
    if (FLD_MATCH(TBL_DHCRELAY_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_dhcrelay->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH, field_id))
    {
        int32 ret;
        p_dhcrelay->drop_agent_mismatch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS, field_id))
    {
        int32 ret;
        p_dhcrelay->add_agent_options = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_AGENT_PROC_OPTION, field_id))
    {
        int32 ret;
        p_dhcrelay->agent_proc_option = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_TRUST_ALL, field_id))
    {
        int32 ret;
        p_dhcrelay->trust_all = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_dhcrelay_table_parser(char** array, char* key_value,tbl_dhcrelay_t *p_dhcrelay)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCRELAY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhcrelay_key_str2val(key_value, p_dhcrelay));

    for(i=1; i<TBL_DHCRELAY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhcrelay_field_value_parser( array[j++], i, p_dhcrelay));
    }

    return PM_E_NONE;
}

int32
tbl_dhcrelay_dump_one(tbl_dhcrelay_t *p_dhcrelay, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCRELAY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_DHCRELAY_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCRELAY_FLD_ENABLE].name,
            p_dhcrelay->enable);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH].name,
            p_dhcrelay->drop_agent_mismatch);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS].name,
            p_dhcrelay->add_agent_options);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_AGENT_PROC_OPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCRELAY_FLD_AGENT_PROC_OPTION].name,
            p_dhcrelay->agent_proc_option);
    }
    if (FLD_MATCH(TBL_DHCRELAY_FLD_TRUST_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCRELAY_FLD_TRUST_ALL].name,
            p_dhcrelay->trust_all);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhcrelay_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhcrelay_t *p_db_dhcrelay = _g_p_tbl_dhcrelay;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dhcrelay)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dhcrelay, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhcrelay_t*
tbl_dhcrelay_init_dhcrelay()
{
    _g_p_tbl_dhcrelay = XCALLOC(MEM_TBL_DHCRELAY, sizeof(tbl_dhcrelay_t));
    return _g_p_tbl_dhcrelay;
}


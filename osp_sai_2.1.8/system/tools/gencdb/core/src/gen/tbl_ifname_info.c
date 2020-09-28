
#include "proto.h"
#include "gen/tbl_ifname_info_define.h"
#include "gen/tbl_ifname_info.h"
#include "cdb_data_cmp.h"

tbl_ifname_info_t *_g_p_tbl_ifname_info = NULL;

int32
tbl_ifname_info_set_ifname_info_field_sync(tbl_ifname_info_t *p_ifname_info, tbl_ifname_info_field_id_t field_id, uint32 sync)
{
    tbl_ifname_info_t *p_db_ifname_info = _g_p_tbl_ifname_info;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IFNAME_INFO_FLD_CONNECTION:
        p_db_ifname_info->connection = p_ifname_info->connection;
        break;

    case TBL_IFNAME_INFO_FLD_SUB_CONNECTION:
        p_db_ifname_info->sub_connection = p_ifname_info->sub_connection;
        break;

    case TBL_IFNAME_INFO_FLD_PREFIX_NUM:
        p_db_ifname_info->prefix_num = p_ifname_info->prefix_num;
        break;

    case TBL_IFNAME_INFO_FLD_PREFIX:
        sal_memcpy(p_db_ifname_info->prefix, p_ifname_info->prefix, sizeof(p_ifname_info->prefix));
        break;

    case TBL_IFNAME_INFO_FLD_FULL_PREFIX:
        sal_memcpy(p_db_ifname_info->full_prefix, p_ifname_info->full_prefix, sizeof(p_ifname_info->full_prefix));
        break;

    case TBL_IFNAME_INFO_FLD_MAX:
        sal_memcpy(p_db_ifname_info, p_ifname_info, sizeof(tbl_ifname_info_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IFNAME_INFO, field_id, p_db_ifname_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IFNAME_INFO, field_id, p_db_ifname_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ifname_info_set_ifname_info_field(tbl_ifname_info_t *p_ifname_info, tbl_ifname_info_field_id_t field_id)
{
    return tbl_ifname_info_set_ifname_info_field_sync(p_ifname_info, field_id, TRUE);
}

tbl_ifname_info_t*
tbl_ifname_info_get_ifname_info()
{
    return _g_p_tbl_ifname_info;
}

int32
tbl_ifname_info_key_str2val(char *str, tbl_ifname_info_t *p_ifname_info)
{
    return PM_E_NONE;
}

char*
tbl_ifname_info_key_name_dump(tbl_ifname_info_t *p_ifname_info, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IFNAME_INFO);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ifname_info_key_value_dump(tbl_ifname_info_t *p_ifname_info, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ifname_info_field_name_dump(tbl_ifname_info_t *p_ifname_info, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IFNAME_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IFNAME_INFO_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ifname_info_field_value_dump(tbl_ifname_info_t *p_ifname_info, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_CONNECTION, field_id))
    {
        sal_sprintf(str, "%d", p_ifname_info->connection);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_SUB_CONNECTION, field_id))
    {
        sal_sprintf(str, "%d", p_ifname_info->sub_connection);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX_NUM, field_id))
    {
        sal_sprintf(str, "%d", p_ifname_info->prefix_num);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_ifname_info->prefix,
        sizeof(p_ifname_info->prefix[0]), sizeof(p_ifname_info->prefix)/sizeof(p_ifname_info->prefix[0]));
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_FULL_PREFIX, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_ifname_info->full_prefix,
        sizeof(p_ifname_info->full_prefix[0]), sizeof(p_ifname_info->full_prefix)/sizeof(p_ifname_info->full_prefix[0]));
    }
    return str;
}

char**
tbl_ifname_info_table_dump(tbl_ifname_info_t *p_ifname_info, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ifname_info_key_name_dump(p_ifname_info, buf));
    for(i=1; i<TBL_IFNAME_INFO_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ifname_info_field_name_dump(p_ifname_info, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ifname_info_field_value_dump(p_ifname_info, i, buf));
    }
    return str;
}

int32
tbl_ifname_info_field_value_parser(char *str, int32 field_id, tbl_ifname_info_t *p_ifname_info)
{
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_CONNECTION, field_id))
    {
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_SUB_CONNECTION, field_id))
    {
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX_NUM, field_id))
    {
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_ifname_info->prefix));
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_FULL_PREFIX, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_ifname_info->full_prefix));
    }
    return PM_E_NONE;
}

int32
tbl_ifname_info_table_parser(char** array, char* key_value,tbl_ifname_info_t *p_ifname_info)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IFNAME_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ifname_info_key_str2val(key_value, p_ifname_info));

    for(i=1; i<TBL_IFNAME_INFO_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ifname_info_field_value_parser( array[j++], i, p_ifname_info));
    }

    return PM_E_NONE;
}

int32
tbl_ifname_info_dump_one(tbl_ifname_info_t *p_ifname_info, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IFNAME_INFO);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_CONNECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IFNAME_INFO_FLD_CONNECTION].name,
            p_ifname_info->connection);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_SUB_CONNECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IFNAME_INFO_FLD_SUB_CONNECTION].name,
            p_ifname_info->sub_connection);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IFNAME_INFO_FLD_PREFIX_NUM].name,
            p_ifname_info->prefix_num);
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_PREFIX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IFNAME_INFO_FLD_PREFIX].name, 
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_ifname_info->prefix,
            sizeof(p_ifname_info->prefix[0]), sizeof(p_ifname_info->prefix)/sizeof(p_ifname_info->prefix[0])));
    }
    if (FLD_MATCH(TBL_IFNAME_INFO_FLD_FULL_PREFIX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IFNAME_INFO_FLD_FULL_PREFIX].name, 
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_ifname_info->full_prefix,
            sizeof(p_ifname_info->full_prefix[0]), sizeof(p_ifname_info->full_prefix)/sizeof(p_ifname_info->full_prefix[0])));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ifname_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ifname_info_t *p_db_ifname_info = _g_p_tbl_ifname_info;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ifname_info)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ifname_info, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ifname_info_t*
tbl_ifname_info_init_ifname_info()
{
    _g_p_tbl_ifname_info = XCALLOC(MEM_TBL_IFNAME_INFO, sizeof(tbl_ifname_info_t));
    return _g_p_tbl_ifname_info;
}



#include "proto.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "cdb_data_cmp.h"

tbl_dhcsnooping_t *_g_p_tbl_dhcsnooping = NULL;

int32
tbl_dhcsnooping_set_dhcsnooping_field_sync(tbl_dhcsnooping_t *p_dhcsnooping, tbl_dhcsnooping_field_id_t field_id, uint32 sync)
{
    tbl_dhcsnooping_t *p_db_dhcsnooping = _g_p_tbl_dhcsnooping;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS:
        p_db_dhcsnooping->verify_mac_address = p_dhcsnooping->verify_mac_address;
        break;

    case TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS:
        p_db_dhcsnooping->add_agent_options = p_dhcsnooping->add_agent_options;
        break;

    case TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED:
        p_db_dhcsnooping->allow_untrusted = p_dhcsnooping->allow_untrusted;
        break;

    case TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID:
        p_db_dhcsnooping->hostname_as_remote_id = p_dhcsnooping->hostname_as_remote_id;
        break;

    case TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING:
        sal_memcpy(p_db_dhcsnooping->remote_id_string, p_dhcsnooping->remote_id_string, sizeof(p_dhcsnooping->remote_id_string));
        break;

    case TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL:
        p_db_dhcsnooping->database_save_interval = p_dhcsnooping->database_save_interval;
        break;

    case TBL_DHCSNOOPING_FLD_VLANS:
        sal_memcpy(p_db_dhcsnooping->vlans, p_dhcsnooping->vlans, sizeof(p_dhcsnooping->vlans));
        break;

    case TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED:
        sal_memcpy(p_db_dhcsnooping->snooping_acl_applied, p_dhcsnooping->snooping_acl_applied, sizeof(p_dhcsnooping->snooping_acl_applied));
        break;

    case TBL_DHCSNOOPING_FLD_MAX:
        sal_memcpy(p_db_dhcsnooping, p_dhcsnooping, sizeof(tbl_dhcsnooping_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCSNOOPING, field_id, p_db_dhcsnooping);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCSNOOPING, field_id, p_db_dhcsnooping);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcsnooping_set_dhcsnooping_field(tbl_dhcsnooping_t *p_dhcsnooping, tbl_dhcsnooping_field_id_t field_id)
{
    return tbl_dhcsnooping_set_dhcsnooping_field_sync(p_dhcsnooping, field_id, TRUE);
}

tbl_dhcsnooping_t*
tbl_dhcsnooping_get_dhcsnooping()
{
    return _g_p_tbl_dhcsnooping;
}

int32
tbl_dhcsnooping_key_str2val(char *str, tbl_dhcsnooping_t *p_dhcsnooping)
{
    return PM_E_NONE;
}

char*
tbl_dhcsnooping_key_name_dump(tbl_dhcsnooping_t *p_dhcsnooping, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSNOOPING);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_dhcsnooping_key_value_dump(tbl_dhcsnooping_t *p_dhcsnooping, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_dhcsnooping_field_name_dump(tbl_dhcsnooping_t *p_dhcsnooping, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSNOOPING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCSNOOPING_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhcsnooping_field_value_dump(tbl_dhcsnooping_t *p_dhcsnooping, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsnooping->verify_mac_address);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsnooping->add_agent_options);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsnooping->allow_untrusted);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsnooping->hostname_as_remote_id);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING, field_id))
    {
        sal_sprintf(str, "%s", p_dhcsnooping->remote_id_string);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsnooping->database_save_interval);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VLANS, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_dhcsnooping->vlans, sizeof(p_dhcsnooping->vlans)/4);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_dhcsnooping->snooping_acl_applied, sizeof(p_dhcsnooping->snooping_acl_applied)/4);
    }
    return str;
}

char**
tbl_dhcsnooping_table_dump(tbl_dhcsnooping_t *p_dhcsnooping, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhcsnooping_key_name_dump(p_dhcsnooping, buf));
    for(i=1; i<TBL_DHCSNOOPING_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhcsnooping_field_name_dump(p_dhcsnooping, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhcsnooping_field_value_dump(p_dhcsnooping, i, buf));
    }
    return str;
}

int32
tbl_dhcsnooping_field_value_parser(char *str, int32 field_id, tbl_dhcsnooping_t *p_dhcsnooping)
{
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS, field_id))
    {
        int32 ret;
        p_dhcsnooping->verify_mac_address = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS, field_id))
    {
        int32 ret;
        p_dhcsnooping->add_agent_options = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED, field_id))
    {
        int32 ret;
        p_dhcsnooping->allow_untrusted = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID, field_id))
    {
        int32 ret;
        p_dhcsnooping->hostname_as_remote_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING, field_id))
    {
        sal_strcpy(p_dhcsnooping->remote_id_string, str);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL, field_id))
    {
        int32 ret;
        p_dhcsnooping->database_save_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VLANS, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_dhcsnooping->vlans, sizeof(p_dhcsnooping->vlans)/4, err);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_dhcsnooping->snooping_acl_applied, sizeof(p_dhcsnooping->snooping_acl_applied)/4, err);
    }
    return PM_E_NONE;
}

int32
tbl_dhcsnooping_table_parser(char** array, char* key_value,tbl_dhcsnooping_t *p_dhcsnooping)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSNOOPING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhcsnooping_key_str2val(key_value, p_dhcsnooping));

    for(i=1; i<TBL_DHCSNOOPING_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhcsnooping_field_value_parser( array[j++], i, p_dhcsnooping));
    }

    return PM_E_NONE;
}

int32
tbl_dhcsnooping_dump_one(tbl_dhcsnooping_t *p_dhcsnooping, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSNOOPING);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS].name,
            p_dhcsnooping->verify_mac_address);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS].name,
            p_dhcsnooping->add_agent_options);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED].name,
            p_dhcsnooping->allow_untrusted);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID].name,
            p_dhcsnooping->hostname_as_remote_id);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING].name,
            p_dhcsnooping->remote_id_string);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL].name,
            p_dhcsnooping->database_save_interval);
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_VLANS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCSNOOPING_FLD_VLANS].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_dhcsnooping->vlans, sizeof(p_dhcsnooping->vlans)/4));
    }
    if (FLD_MATCH(TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_dhcsnooping->snooping_acl_applied, sizeof(p_dhcsnooping->snooping_acl_applied)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhcsnooping_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhcsnooping_t *p_db_dhcsnooping = _g_p_tbl_dhcsnooping;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dhcsnooping)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dhcsnooping, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhcsnooping_t*
tbl_dhcsnooping_init_dhcsnooping()
{
    _g_p_tbl_dhcsnooping = XCALLOC(MEM_TBL_DHCSNOOPING, sizeof(tbl_dhcsnooping_t));
    return _g_p_tbl_dhcsnooping;
}


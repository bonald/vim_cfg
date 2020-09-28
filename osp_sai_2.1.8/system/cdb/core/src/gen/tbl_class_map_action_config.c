
#include "proto.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_class_map_action_config.h"
#include "cdb_data_cmp.h"

tbl_class_map_action_config_master_t *_g_p_tbl_class_map_action_config_master = NULL;

static uint32
_tbl_class_map_action_config_hash_make(tbl_class_map_action_config_t *p_class_map_action_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_class_map_action_config->key;
    for (index = 0; index < sizeof(p_class_map_action_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_class_map_action_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_class_map_action_config_t *p_class_map_action_config1 = (tbl_class_map_action_config_t*)p_arg1;
    tbl_class_map_action_config_t *p_class_map_action_config2 = (tbl_class_map_action_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_class_map_action_config1->key, &p_class_map_action_config2->key, sizeof(tbl_class_map_action_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_class_map_action_config_add_class_map_action_config_sync(tbl_class_map_action_config_t *p_class_map_action_config, uint32 sync)
{
    tbl_class_map_action_config_master_t *p_master = _g_p_tbl_class_map_action_config_master;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_class_map_action_config_get_class_map_action_config(&p_class_map_action_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_class_map_action_config = XCALLOC(MEM_TBL_CLASS_MAP_ACTION_CONFIG, sizeof(tbl_class_map_action_config_t));
    if (NULL == p_db_class_map_action_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_class_map_action_config, p_class_map_action_config, sizeof(tbl_class_map_action_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->class_map_action_config_hash, (void*)p_db_class_map_action_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->class_map_action_config_list, p_db_class_map_action_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CLASS_MAP_ACTION_CONFIG, p_db_class_map_action_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CLASS_MAP_ACTION_CONFIG, p_db_class_map_action_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_class_map_action_config_del_class_map_action_config_sync(tbl_class_map_action_config_key_t *p_class_map_action_config_key, uint32 sync)
{
    tbl_class_map_action_config_master_t *p_master = _g_p_tbl_class_map_action_config_master;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;

    /* 1. lookup entry exist */
    p_db_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config(p_class_map_action_config_key);
    if (NULL == p_db_class_map_action_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CLASS_MAP_ACTION_CONFIG, p_db_class_map_action_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CLASS_MAP_ACTION_CONFIG, p_db_class_map_action_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->class_map_action_config_hash, (void*)p_db_class_map_action_config);
    ctclib_slistnode_delete(p_master->class_map_action_config_list, p_db_class_map_action_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_CLASS_MAP_ACTION_CONFIG, p_db_class_map_action_config);

    return PM_E_NONE;
}

int32
tbl_class_map_action_config_set_class_map_action_config_field_sync(tbl_class_map_action_config_t *p_class_map_action_config, tbl_class_map_action_config_field_id_t field_id, uint32 sync)
{
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;

    /* 1. lookup entry exist */
    p_db_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config(&p_class_map_action_config->key);
    if (NULL == p_db_class_map_action_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG:
        p_db_class_map_action_config->action_flag = p_class_map_action_config->action_flag;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC:
        p_db_class_map_action_config->new_tc = p_class_map_action_config->new_tc;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR:
        p_db_class_map_action_config->new_color = p_class_map_action_config->new_color;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP:
        p_db_class_map_action_config->new_dscp = p_class_map_action_config->new_dscp;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID:
        p_db_class_map_action_config->session_id = p_class_map_action_config->session_id;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER:
        sal_memcpy(p_db_class_map_action_config->policer, p_class_map_action_config->policer, sizeof(p_class_map_action_config->policer));
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID:
        p_db_class_map_action_config->new_svlan_id = p_class_map_action_config->new_svlan_id;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID:
        p_db_class_map_action_config->new_cvlan_id = p_class_map_action_config->new_cvlan_id;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS:
        p_db_class_map_action_config->new_scos = p_class_map_action_config->new_scos;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS:
        p_db_class_map_action_config->new_ccos = p_class_map_action_config->new_ccos;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX:
        p_db_class_map_action_config->redirect_port_ifindex = p_class_map_action_config->redirect_port_ifindex;
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG:
        sal_memcpy(p_db_class_map_action_config->red_unexist_agg, p_class_map_action_config->red_unexist_agg, sizeof(p_class_map_action_config->red_unexist_agg));
        break;

    case TBL_CLASS_MAP_ACTION_CONFIG_FLD_MAX:
        sal_memcpy(p_db_class_map_action_config, p_class_map_action_config, sizeof(tbl_class_map_action_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CLASS_MAP_ACTION_CONFIG, field_id, p_db_class_map_action_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CLASS_MAP_ACTION_CONFIG, field_id, p_db_class_map_action_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_class_map_action_config_add_class_map_action_config(tbl_class_map_action_config_t *p_class_map_action_config)
{
    return tbl_class_map_action_config_add_class_map_action_config_sync(p_class_map_action_config, TRUE);
}

int32
tbl_class_map_action_config_del_class_map_action_config(tbl_class_map_action_config_key_t *p_class_map_action_config_key)
{
    return tbl_class_map_action_config_del_class_map_action_config_sync(p_class_map_action_config_key, TRUE);
}

int32
tbl_class_map_action_config_set_class_map_action_config_field(tbl_class_map_action_config_t *p_class_map_action_config, tbl_class_map_action_config_field_id_t field_id)
{
    return tbl_class_map_action_config_set_class_map_action_config_field_sync(p_class_map_action_config, field_id, TRUE);
}

tbl_class_map_action_config_t*
tbl_class_map_action_config_get_class_map_action_config(tbl_class_map_action_config_key_t *p_class_map_action_config_key)
{
    tbl_class_map_action_config_master_t *p_master = _g_p_tbl_class_map_action_config_master;
    tbl_class_map_action_config_t lkp;

    sal_memcpy(&lkp.key, p_class_map_action_config_key, sizeof(tbl_class_map_action_config_key_t));
    return ctclib_hash_lookup(p_master->class_map_action_config_hash, &lkp);
}

char*
tbl_class_map_action_config_key_val2str(tbl_class_map_action_config_t *p_class_map_action_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_class_map_action_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_class_map_action_config->key));
    return str;
}

int32
tbl_class_map_action_config_key_str2val(char *str, tbl_class_map_action_config_t *p_class_map_action_config)
{
    int32 ret = 0;

    ret = cdb_class_map_action_config_key_str2val(str, &p_class_map_action_config->key);
    return ret;
}

char*
tbl_class_map_action_config_key_name_dump(tbl_class_map_action_config_t *p_class_map_action_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_class_map_action_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_class_map_action_config->key));
    return str;
}

char*
tbl_class_map_action_config_key_value_dump(tbl_class_map_action_config_t *p_class_map_action_config, char *str)
{

    cdb_class_map_action_config_key_val2str(str, MAX_CMD_STR_LEN, &p_class_map_action_config->key);
    return str;
}

char*
tbl_class_map_action_config_field_name_dump(tbl_class_map_action_config_t *p_class_map_action_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CLASS_MAP_ACTION_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_class_map_action_config_field_value_dump(tbl_class_map_action_config_t *p_class_map_action_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY, field_id))
    {
        cdb_class_map_action_config_key_val2str(str, MAX_CMD_STR_LEN, &p_class_map_action_config->key);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->action_flag);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_tc);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR, field_id))
    {
        sal_sprintf(str, "%d", p_class_map_action_config->new_color);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_dscp);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->session_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_class_map_action_config->policer);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_svlan_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_cvlan_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_scos);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->new_ccos);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_class_map_action_config->redirect_port_ifindex);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG, field_id))
    {
        sal_sprintf(str, "%s", p_class_map_action_config->red_unexist_agg);
    }
    return str;
}

char**
tbl_class_map_action_config_table_dump(tbl_class_map_action_config_t *p_class_map_action_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_class_map_action_config_key_name_dump(p_class_map_action_config, buf));
    for(i=1; i<TBL_CLASS_MAP_ACTION_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_class_map_action_config_field_name_dump(p_class_map_action_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_class_map_action_config_field_value_dump(p_class_map_action_config, i, buf));
    }
    return str;
}

int32
tbl_class_map_action_config_field_value_parser(char *str, int32 field_id, tbl_class_map_action_config_t *p_class_map_action_config)
{
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY, field_id))
    {
        cdb_class_map_action_config_key_str2val(str, &p_class_map_action_config->key);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG, field_id))
    {
        int32 ret;
        p_class_map_action_config->action_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_tc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR, field_id))
    {
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID, field_id))
    {
        int32 ret;
        p_class_map_action_config->session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER, field_id))
    {
        sal_strcpy(p_class_map_action_config->policer, str);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_svlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_cvlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_scos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS, field_id))
    {
        int32 ret;
        p_class_map_action_config->new_ccos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_class_map_action_config->redirect_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG, field_id))
    {
        sal_strcpy(p_class_map_action_config->red_unexist_agg, str);
    }
    return PM_E_NONE;
}

int32
tbl_class_map_action_config_table_parser(char** array, char* key_value,tbl_class_map_action_config_t *p_class_map_action_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_class_map_action_config_key_str2val(key_value, p_class_map_action_config));

    for(i=1; i<TBL_CLASS_MAP_ACTION_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_class_map_action_config_field_value_parser( array[j++], i, p_class_map_action_config));
    }

    return PM_E_NONE;
}

int32
tbl_class_map_action_config_dump_one(tbl_class_map_action_config_t *p_class_map_action_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY].name, 
            cdb_class_map_action_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_class_map_action_config->key));
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG].name,
            p_class_map_action_config->action_flag);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC].name,
            p_class_map_action_config->new_tc);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR].name,
            p_class_map_action_config->new_color);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP].name,
            p_class_map_action_config->new_dscp);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID].name,
            p_class_map_action_config->session_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER].name,
            p_class_map_action_config->policer);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID].name,
            p_class_map_action_config->new_svlan_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID].name,
            p_class_map_action_config->new_cvlan_id);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS].name,
            p_class_map_action_config->new_scos);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS].name,
            p_class_map_action_config->new_ccos);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX].name,
            p_class_map_action_config->redirect_port_ifindex);
    }
    if (FLD_MATCH(TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG].name,
            p_class_map_action_config->red_unexist_agg);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_class_map_action_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_class_map_action_config_master_t *p_master = _g_p_tbl_class_map_action_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->class_map_action_config_list, p_db_class_map_action_config, listnode, next)
    {
        rc |= fn(p_db_class_map_action_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_class_map_action_config_master_t*
tbl_class_map_action_config_get_master()
{
    return _g_p_tbl_class_map_action_config_master;
}

tbl_class_map_action_config_master_t*
tbl_class_map_action_config_init_class_map_action_config()
{
    _g_p_tbl_class_map_action_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_class_map_action_config_master_t));
    _g_p_tbl_class_map_action_config_master->class_map_action_config_hash = ctclib_hash_create(_tbl_class_map_action_config_hash_make, _tbl_class_map_action_config_hash_cmp);
    _g_p_tbl_class_map_action_config_master->class_map_action_config_list = ctclib_slist_create(tbl_class_map_action_config_cmp, NULL);
    return _g_p_tbl_class_map_action_config_master;
}


tbl_class_map_action_config_t*
tbl_class_map_action_config_get_class_map_action_config_by_name(const char* policy_map_name, const char* class_map_name)
{
    tbl_class_map_action_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.policy_name, policy_map_name, PMAP_NAME_SIZE);
    sal_strncpy(key.class_name, class_map_name, CMAP_NAME_SIZE);
    return tbl_class_map_action_config_get_class_map_action_config(&key);
}


#include "proto.h"
#include "gen/tbl_fea_acl_policy_action_define.h"
#include "gen/tbl_fea_acl_policy_action.h"
#include "cdb_data_cmp.h"

tbl_fea_acl_policy_action_master_t *_g_p_tbl_fea_acl_policy_action_master = NULL;

static uint32
_tbl_fea_acl_policy_action_hash_make(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fea_acl_policy_action->key;
    for (index = 0; index < sizeof(p_fea_acl_policy_action->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_acl_policy_action_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_acl_policy_action_t *p_fea_acl_policy_action1 = (tbl_fea_acl_policy_action_t*)p_arg1;
    tbl_fea_acl_policy_action_t *p_fea_acl_policy_action2 = (tbl_fea_acl_policy_action_t*)p_arg2;

    if (0 == sal_memcmp(&p_fea_acl_policy_action1->key, &p_fea_acl_policy_action2->key, sizeof(tbl_fea_acl_policy_action_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_acl_policy_action_add_fea_acl_policy_action_sync(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, uint32 sync)
{
    tbl_fea_acl_policy_action_master_t *p_master = _g_p_tbl_fea_acl_policy_action_master;
    tbl_fea_acl_policy_action_t *p_db_fea_acl_policy_action = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_acl_policy_action_get_fea_acl_policy_action(&p_fea_acl_policy_action->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_acl_policy_action = XCALLOC(MEM_TBL_FEA_ACL_POLICY_ACTION, sizeof(tbl_fea_acl_policy_action_t));
    if (NULL == p_db_fea_acl_policy_action)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_acl_policy_action, p_fea_acl_policy_action, sizeof(tbl_fea_acl_policy_action_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fea_acl_policy_action_hash, (void*)p_db_fea_acl_policy_action, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_ACL_POLICY_ACTION, p_db_fea_acl_policy_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_ACL_POLICY_ACTION, p_db_fea_acl_policy_action);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_del_fea_acl_policy_action_sync(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key, uint32 sync)
{
    tbl_fea_acl_policy_action_master_t *p_master = _g_p_tbl_fea_acl_policy_action_master;
    tbl_fea_acl_policy_action_t *p_db_fea_acl_policy_action = NULL;

    /* 1. lookup entry exist */
    p_db_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(p_fea_acl_policy_action_key);
    if (NULL == p_db_fea_acl_policy_action)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_ACL_POLICY_ACTION, p_db_fea_acl_policy_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_ACL_POLICY_ACTION, p_db_fea_acl_policy_action);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fea_acl_policy_action_hash, (void*)p_db_fea_acl_policy_action);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_ACL_POLICY_ACTION, p_db_fea_acl_policy_action);

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_set_fea_acl_policy_action_field_sync(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_fea_acl_policy_action_field_id_t field_id, uint32 sync)
{
    tbl_fea_acl_policy_action_t *p_db_fea_acl_policy_action = NULL;

    /* 1. lookup entry exist */
    p_db_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&p_fea_acl_policy_action->key);
    if (NULL == p_db_fea_acl_policy_action)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_fea_acl_policy_action, p_fea_acl_policy_action, flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS)
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS:
        CDB_FLAG_CHECK_SET(p_db_fea_acl_policy_action, p_fea_acl_policy_action, flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_CLEAR_STATS)
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID:
        p_db_fea_acl_policy_action->policer_id = p_fea_acl_policy_action->policer_id;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME:
        sal_memcpy(p_db_fea_acl_policy_action->policer_name, p_fea_acl_policy_action->policer_name, sizeof(p_fea_acl_policy_action->policer_name));
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF:
        p_db_fea_acl_policy_action->policer_id_ref = p_fea_acl_policy_action->policer_id_ref;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET:
        p_db_fea_acl_policy_action->green_packet = p_fea_acl_policy_action->green_packet;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE:
        p_db_fea_acl_policy_action->green_byte = p_fea_acl_policy_action->green_byte;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET:
        p_db_fea_acl_policy_action->yellow_packet = p_fea_acl_policy_action->yellow_packet;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE:
        p_db_fea_acl_policy_action->yellow_byte = p_fea_acl_policy_action->yellow_byte;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET:
        p_db_fea_acl_policy_action->red_packet = p_fea_acl_policy_action->red_packet;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE:
        p_db_fea_acl_policy_action->red_byte = p_fea_acl_policy_action->red_byte;
        break;

    case TBL_FEA_ACL_POLICY_ACTION_FLD_MAX:
        sal_memcpy(p_db_fea_acl_policy_action, p_fea_acl_policy_action, sizeof(tbl_fea_acl_policy_action_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_ACL_POLICY_ACTION, field_id, p_db_fea_acl_policy_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_ACL_POLICY_ACTION, field_id, p_db_fea_acl_policy_action);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_add_fea_acl_policy_action(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    return tbl_fea_acl_policy_action_add_fea_acl_policy_action_sync(p_fea_acl_policy_action, TRUE);
}

int32
tbl_fea_acl_policy_action_del_fea_acl_policy_action(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key)
{
    return tbl_fea_acl_policy_action_del_fea_acl_policy_action_sync(p_fea_acl_policy_action_key, TRUE);
}

int32
tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_fea_acl_policy_action_field_id_t field_id)
{
    return tbl_fea_acl_policy_action_set_fea_acl_policy_action_field_sync(p_fea_acl_policy_action, field_id, TRUE);
}

tbl_fea_acl_policy_action_t*
tbl_fea_acl_policy_action_get_fea_acl_policy_action(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key)
{
    tbl_fea_acl_policy_action_master_t *p_master = _g_p_tbl_fea_acl_policy_action_master;
    tbl_fea_acl_policy_action_t lkp;

    sal_memcpy(&lkp.key, p_fea_acl_policy_action_key, sizeof(tbl_fea_acl_policy_action_key_t));
    return ctclib_hash_lookup(p_master->fea_acl_policy_action_hash, &lkp);
}

char*
tbl_fea_acl_policy_action_key_val2str(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%"PRIu64, p_fea_acl_policy_action->key.acl_policy_action_id);
    return str;
}

int32
tbl_fea_acl_policy_action_key_str2val(char *str, tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    int32 ret = 0;

    p_fea_acl_policy_action->key.acl_policy_action_id = cdb_uint64_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_acl_policy_action_key_name_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL_POLICY_ACTION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%"PRIu64, p_node->name, p_fea_acl_policy_action->key.acl_policy_action_id);
    return str;
}

char*
tbl_fea_acl_policy_action_key_value_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char *str)
{
    sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->key.acl_policy_action_id);
    return str;
}

char*
tbl_fea_acl_policy_action_field_name_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL_POLICY_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_ACL_POLICY_ACTION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_acl_policy_action_field_value_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->key.acl_policy_action_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS));
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_CLEAR_STATS));
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->policer_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_fea_acl_policy_action->policer_name);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->policer_id_ref);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->green_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->green_byte);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->yellow_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->yellow_byte);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->red_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl_policy_action->red_byte);
    }
    return str;
}

char**
tbl_fea_acl_policy_action_table_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_acl_policy_action_key_name_dump(p_fea_acl_policy_action, buf));
    for(i=1; i<TBL_FEA_ACL_POLICY_ACTION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_acl_policy_action_field_name_dump(p_fea_acl_policy_action, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_acl_policy_action_field_value_dump(p_fea_acl_policy_action, i, buf));
    }
    return str;
}

int32
tbl_fea_acl_policy_action_field_value_parser(char *str, int32 field_id, tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->key.acl_policy_action_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS, field_id))
    {
        GLB_SET_FLAG(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_CLEAR_STATS);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->policer_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME, field_id))
    {
        sal_strcpy(p_fea_acl_policy_action->policer_name, str);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->policer_id_ref = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->green_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->green_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->yellow_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->yellow_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->red_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE, field_id))
    {
        int32 ret;
        p_fea_acl_policy_action->red_byte = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_table_parser(char** array, char* key_value,tbl_fea_acl_policy_action_t *p_fea_acl_policy_action)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL_POLICY_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_acl_policy_action_key_str2val(key_value, p_fea_acl_policy_action));

    for(i=1; i<TBL_FEA_ACL_POLICY_ACTION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_acl_policy_action_field_value_parser( array[j++], i, p_fea_acl_policy_action));
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_dump_one(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL_POLICY_ACTION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_KEY].name,
            p_fea_acl_policy_action->key.acl_policy_action_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS));
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS].name,
            GLB_FLAG_ISSET(p_fea_acl_policy_action->flags, GLB_ACL_POLICY_POLICY_STATS_FLAGS_CLEAR_STATS));
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID].name,
            p_fea_acl_policy_action->policer_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME].name,
            p_fea_acl_policy_action->policer_name);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF].name,
            p_fea_acl_policy_action->policer_id_ref);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET].name,
            p_fea_acl_policy_action->green_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE].name,
            p_fea_acl_policy_action->green_byte);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET].name,
            p_fea_acl_policy_action->yellow_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE].name,
            p_fea_acl_policy_action->yellow_byte);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET].name,
            p_fea_acl_policy_action->red_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE].name,
            p_fea_acl_policy_action->red_byte);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_acl_policy_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_acl_policy_action_master_t *p_master = _g_p_tbl_fea_acl_policy_action_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->fea_acl_policy_action_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fea_acl_policy_action_master_t*
tbl_fea_acl_policy_action_get_master()
{
    return _g_p_tbl_fea_acl_policy_action_master;
}

tbl_fea_acl_policy_action_master_t*
tbl_fea_acl_policy_action_init_fea_acl_policy_action()
{
    _g_p_tbl_fea_acl_policy_action_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_acl_policy_action_master_t));
    _g_p_tbl_fea_acl_policy_action_master->fea_acl_policy_action_hash = ctclib_hash_create(_tbl_fea_acl_policy_action_hash_make, _tbl_fea_acl_policy_action_hash_cmp);
    return _g_p_tbl_fea_acl_policy_action_master;
}


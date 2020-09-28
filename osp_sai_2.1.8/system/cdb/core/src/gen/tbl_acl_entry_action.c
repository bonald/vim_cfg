
#include "proto.h"
#include "gen/tbl_acl_entry_action_define.h"
#include "gen/tbl_acl_entry_action.h"
#include "cdb_data_cmp.h"

tbl_acl_entry_action_master_t *_g_p_tbl_acl_entry_action_master = NULL;

static uint32
_tbl_acl_entry_action_hash_make(tbl_acl_entry_action_t *p_acl_entry_action)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_entry_action->key;
    for (index = 0; index < sizeof(p_acl_entry_action->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_entry_action_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_entry_action_t *p_acl_entry_action1 = (tbl_acl_entry_action_t*)p_arg1;
    tbl_acl_entry_action_t *p_acl_entry_action2 = (tbl_acl_entry_action_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_entry_action1->key, &p_acl_entry_action2->key, sizeof(tbl_acl_entry_action_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_entry_action_add_acl_entry_action_sync(tbl_acl_entry_action_t *p_acl_entry_action, uint32 sync)
{
    tbl_acl_entry_action_master_t *p_master = _g_p_tbl_acl_entry_action_master;
    tbl_acl_entry_action_t *p_db_acl_entry_action = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_entry_action_get_acl_entry_action(&p_acl_entry_action->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_entry_action = XCALLOC(MEM_TBL_ACL_ENTRY_ACTION, sizeof(tbl_acl_entry_action_t));
    if (NULL == p_db_acl_entry_action)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_entry_action, p_acl_entry_action, sizeof(tbl_acl_entry_action_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_entry_action_hash, (void*)p_db_acl_entry_action, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_ENTRY_ACTION, p_db_acl_entry_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_ENTRY_ACTION, p_db_acl_entry_action);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_action_del_acl_entry_action_sync(tbl_acl_entry_action_key_t *p_acl_entry_action_key, uint32 sync)
{
    tbl_acl_entry_action_master_t *p_master = _g_p_tbl_acl_entry_action_master;
    tbl_acl_entry_action_t *p_db_acl_entry_action = NULL;

    /* 1. lookup entry exist */
    p_db_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(p_acl_entry_action_key);
    if (NULL == p_db_acl_entry_action)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_ENTRY_ACTION, p_db_acl_entry_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_ENTRY_ACTION, p_db_acl_entry_action);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_entry_action_hash, (void*)p_db_acl_entry_action);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_ENTRY_ACTION, p_db_acl_entry_action);

    return PM_E_NONE;
}

int32
tbl_acl_entry_action_set_acl_entry_action_field_sync(tbl_acl_entry_action_t *p_acl_entry_action, tbl_acl_entry_action_field_id_t field_id, uint32 sync)
{
    tbl_acl_entry_action_t *p_db_acl_entry_action = NULL;

    /* 1. lookup entry exist */
    p_db_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(&p_acl_entry_action->key);
    if (NULL == p_db_acl_entry_action)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG:
        p_db_acl_entry_action->action_flag = p_acl_entry_action->action_flag;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_DENY:
        p_db_acl_entry_action->deny = p_acl_entry_action->deny;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE:
        p_db_acl_entry_action->stats_enale = p_acl_entry_action->stats_enale;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_POLICER:
        sal_memcpy(p_db_acl_entry_action->policer, p_acl_entry_action->policer, sizeof(p_acl_entry_action->policer));
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP:
        p_db_acl_entry_action->nexthop_group = p_acl_entry_action->nexthop_group;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_LOGEN:
        p_db_acl_entry_action->logen = p_acl_entry_action->logen;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_TC:
        p_db_acl_entry_action->tc = p_acl_entry_action->tc;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_COLOR:
        p_db_acl_entry_action->color = p_acl_entry_action->color;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_DSCP:
        p_db_acl_entry_action->dscp = p_acl_entry_action->dscp;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL:
        p_db_acl_entry_action->snat_cancel = p_acl_entry_action->snat_cancel;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID:
        p_db_acl_entry_action->session_id = p_acl_entry_action->session_id;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID:
        p_db_acl_entry_action->new_svlan_id = p_acl_entry_action->new_svlan_id;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID:
        p_db_acl_entry_action->new_cvlan_id = p_acl_entry_action->new_cvlan_id;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS:
        p_db_acl_entry_action->new_scos = p_acl_entry_action->new_scos;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS:
        p_db_acl_entry_action->new_ccos = p_acl_entry_action->new_ccos;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX:
        p_db_acl_entry_action->redirect_port_ifindex = p_acl_entry_action->redirect_port_ifindex;
        break;

    case TBL_ACL_ENTRY_ACTION_FLD_MAX:
        sal_memcpy(p_db_acl_entry_action, p_acl_entry_action, sizeof(tbl_acl_entry_action_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_ENTRY_ACTION, field_id, p_db_acl_entry_action);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_ENTRY_ACTION, field_id, p_db_acl_entry_action);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_action_add_acl_entry_action(tbl_acl_entry_action_t *p_acl_entry_action)
{
    return tbl_acl_entry_action_add_acl_entry_action_sync(p_acl_entry_action, TRUE);
}

int32
tbl_acl_entry_action_del_acl_entry_action(tbl_acl_entry_action_key_t *p_acl_entry_action_key)
{
    return tbl_acl_entry_action_del_acl_entry_action_sync(p_acl_entry_action_key, TRUE);
}

int32
tbl_acl_entry_action_set_acl_entry_action_field(tbl_acl_entry_action_t *p_acl_entry_action, tbl_acl_entry_action_field_id_t field_id)
{
    return tbl_acl_entry_action_set_acl_entry_action_field_sync(p_acl_entry_action, field_id, TRUE);
}

tbl_acl_entry_action_t*
tbl_acl_entry_action_get_acl_entry_action(tbl_acl_entry_action_key_t *p_acl_entry_action_key)
{
    tbl_acl_entry_action_master_t *p_master = _g_p_tbl_acl_entry_action_master;
    tbl_acl_entry_action_t lkp;

    sal_memcpy(&lkp.key, p_acl_entry_action_key, sizeof(tbl_acl_entry_action_key_t));
    return ctclib_hash_lookup(p_master->acl_entry_action_hash, &lkp);
}

char*
tbl_acl_entry_action_key_val2str(tbl_acl_entry_action_t *p_acl_entry_action, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%"PRIu64, p_acl_entry_action->key.aclid);
    return str;
}

int32
tbl_acl_entry_action_key_str2val(char *str, tbl_acl_entry_action_t *p_acl_entry_action)
{
    int32 ret = 0;

    p_acl_entry_action->key.aclid = cdb_uint64_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_entry_action_key_name_dump(tbl_acl_entry_action_t *p_acl_entry_action, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY_ACTION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%"PRIu64, p_node->name, p_acl_entry_action->key.aclid);
    return str;
}

char*
tbl_acl_entry_action_key_value_dump(tbl_acl_entry_action_t *p_acl_entry_action, char *str)
{
    sal_sprintf(str, "%"PRIu64, p_acl_entry_action->key.aclid);
    return str;
}

char*
tbl_acl_entry_action_field_name_dump(tbl_acl_entry_action_t *p_acl_entry_action, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_ENTRY_ACTION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_entry_action_field_value_dump(tbl_acl_entry_action_t *p_acl_entry_action, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry_action->key.aclid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->action_flag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DENY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->deny);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->stats_enale);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_acl_entry_action->policer);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->nexthop_group);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_LOGEN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->logen);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_TC, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->tc);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_COLOR, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->color);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->snat_cancel);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->session_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->new_svlan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->new_cvlan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->new_scos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->new_ccos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry_action->redirect_port_ifindex);
    }
    return str;
}

char**
tbl_acl_entry_action_table_dump(tbl_acl_entry_action_t *p_acl_entry_action, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_entry_action_key_name_dump(p_acl_entry_action, buf));
    for(i=1; i<TBL_ACL_ENTRY_ACTION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_entry_action_field_name_dump(p_acl_entry_action, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_entry_action_field_value_dump(p_acl_entry_action, i, buf));
    }
    return str;
}

int32
tbl_acl_entry_action_field_value_parser(char *str, int32 field_id, tbl_acl_entry_action_t *p_acl_entry_action)
{
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_entry_action->key.aclid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG, field_id))
    {
        int32 ret;
        p_acl_entry_action->action_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DENY, field_id))
    {
        int32 ret;
        p_acl_entry_action->deny = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE, field_id))
    {
        int32 ret;
        p_acl_entry_action->stats_enale = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_POLICER, field_id))
    {
        sal_strcpy(p_acl_entry_action->policer, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP, field_id))
    {
        int32 ret;
        p_acl_entry_action->nexthop_group = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_LOGEN, field_id))
    {
        int32 ret;
        p_acl_entry_action->logen = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_TC, field_id))
    {
        int32 ret;
        p_acl_entry_action->tc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_COLOR, field_id))
    {
        int32 ret;
        p_acl_entry_action->color = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DSCP, field_id))
    {
        int32 ret;
        p_acl_entry_action->dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL, field_id))
    {
        int32 ret;
        p_acl_entry_action->snat_cancel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID, field_id))
    {
        int32 ret;
        p_acl_entry_action->session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID, field_id))
    {
        int32 ret;
        p_acl_entry_action->new_svlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID, field_id))
    {
        int32 ret;
        p_acl_entry_action->new_cvlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS, field_id))
    {
        int32 ret;
        p_acl_entry_action->new_scos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS, field_id))
    {
        int32 ret;
        p_acl_entry_action->new_ccos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_acl_entry_action->redirect_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_entry_action_table_parser(char** array, char* key_value,tbl_acl_entry_action_t *p_acl_entry_action)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY_ACTION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_entry_action_key_str2val(key_value, p_acl_entry_action));

    for(i=1; i<TBL_ACL_ENTRY_ACTION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_entry_action_field_value_parser( array[j++], i, p_acl_entry_action));
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_action_dump_one(tbl_acl_entry_action_t *p_acl_entry_action, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY_ACTION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_KEY].name,
            p_acl_entry_action->key.aclid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG].name,
            p_acl_entry_action->action_flag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DENY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_DENY].name,
            p_acl_entry_action->deny);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE].name,
            p_acl_entry_action->stats_enale);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_POLICER].name,
            p_acl_entry_action->policer);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP].name,
            p_acl_entry_action->nexthop_group);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_LOGEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_LOGEN].name,
            p_acl_entry_action->logen);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_TC].name,
            p_acl_entry_action->tc);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_COLOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_COLOR].name,
            p_acl_entry_action->color);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_DSCP].name,
            p_acl_entry_action->dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL].name,
            p_acl_entry_action->snat_cancel);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID].name,
            p_acl_entry_action->session_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID].name,
            p_acl_entry_action->new_svlan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID].name,
            p_acl_entry_action->new_cvlan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS].name,
            p_acl_entry_action->new_scos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS].name,
            p_acl_entry_action->new_ccos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX].name,
            p_acl_entry_action->redirect_port_ifindex);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_entry_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_entry_action_master_t *p_master = _g_p_tbl_acl_entry_action_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->acl_entry_action_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_acl_entry_action_master_t*
tbl_acl_entry_action_get_master()
{
    return _g_p_tbl_acl_entry_action_master;
}

tbl_acl_entry_action_master_t*
tbl_acl_entry_action_init_acl_entry_action()
{
    _g_p_tbl_acl_entry_action_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_entry_action_master_t));
    _g_p_tbl_acl_entry_action_master->acl_entry_action_hash = ctclib_hash_create(_tbl_acl_entry_action_hash_make, _tbl_acl_entry_action_hash_cmp);
    return _g_p_tbl_acl_entry_action_master;
}


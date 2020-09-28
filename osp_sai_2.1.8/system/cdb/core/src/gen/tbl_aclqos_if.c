
#include "proto.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "cdb_data_cmp.h"

tbl_aclqos_if_master_t *_g_p_tbl_aclqos_if_master = NULL;

static uint32
_tbl_aclqos_if_hash_make(tbl_aclqos_if_t *p_aclqos_if)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_aclqos_if->key;
    for (index = 0; index < sizeof(p_aclqos_if->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_aclqos_if_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_aclqos_if_t *p_aclqos_if1 = (tbl_aclqos_if_t*)p_arg1;
    tbl_aclqos_if_t *p_aclqos_if2 = (tbl_aclqos_if_t*)p_arg2;

    if (0 == sal_memcmp(&p_aclqos_if1->key, &p_aclqos_if2->key, sizeof(tbl_aclqos_if_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_aclqos_if_add_aclqos_if_sync(tbl_aclqos_if_t *p_aclqos_if, uint32 sync)
{
    tbl_aclqos_if_master_t *p_master = _g_p_tbl_aclqos_if_master;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_aclqos_if_get_aclqos_if(&p_aclqos_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_aclqos_if = XCALLOC(MEM_TBL_ACLQOS_IF, sizeof(tbl_aclqos_if_t));
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_aclqos_if, p_aclqos_if, sizeof(tbl_aclqos_if_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->if_hash, (void*)p_db_aclqos_if, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_aclqos_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACLQOS_IF, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACLQOS_IF, p_db_aclqos_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_del_aclqos_if_sync(tbl_aclqos_if_key_t *p_aclqos_if_key, uint32 sync)
{
    tbl_aclqos_if_master_t *p_master = _g_p_tbl_aclqos_if_master;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;

    /* 1. lookup entry exist */
    p_db_aclqos_if = tbl_aclqos_if_get_aclqos_if(p_aclqos_if_key);
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACLQOS_IF, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACLQOS_IF, p_db_aclqos_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->if_hash, (void*)p_db_aclqos_if);
    ctclib_slistnode_delete(p_master->if_list, p_db_aclqos_if);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACLQOS_IF, p_db_aclqos_if);

    return PM_E_NONE;
}

int32
tbl_aclqos_if_set_aclqos_if_field_sync(tbl_aclqos_if_t *p_aclqos_if, tbl_aclqos_if_field_id_t field_id, uint32 sync)
{
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;

    /* 1. lookup entry exist */
    p_db_aclqos_if = tbl_aclqos_if_get_aclqos_if(&p_aclqos_if->key);
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACLQOS_IF_FLD_IFINDEX:
        p_db_aclqos_if->ifindex = p_aclqos_if->ifindex;
        break;

    case TBL_ACLQOS_IF_FLD_PORT_TYPE:
        p_db_aclqos_if->port_type = p_aclqos_if->port_type;
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_DOMAIN)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_TRUST:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_TRUST)
        break;

    case TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC:
        CDB_FLAG_CHECK_SET(p_db_aclqos_if, p_aclqos_if, flags, GLB_ACLQOS_IF_FLAGS_PORT_TC)
        break;

    case TBL_ACLQOS_IF_FLD_DOMAIN:
        p_db_aclqos_if->domain = p_aclqos_if->domain;
        break;

    case TBL_ACLQOS_IF_FLD_COS_DOMAIN:
        p_db_aclqos_if->cos_domain = p_aclqos_if->cos_domain;
        break;

    case TBL_ACLQOS_IF_FLD_DSCP_DOMAIN:
        p_db_aclqos_if->dscp_domain = p_aclqos_if->dscp_domain;
        break;

    case TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE:
        sal_memcpy(p_db_aclqos_if->port_shape_profile, p_aclqos_if->port_shape_profile, sizeof(p_aclqos_if->port_shape_profile));
        break;

    case TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP:
        sal_memcpy(p_db_aclqos_if->input_policy_map, p_aclqos_if->input_policy_map, sizeof(p_aclqos_if->input_policy_map));
        break;

    case TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP:
        sal_memcpy(p_db_aclqos_if->output_policy_map, p_aclqos_if->output_policy_map, sizeof(p_aclqos_if->output_policy_map));
        break;

    case TBL_ACLQOS_IF_FLD_INPUT_POLICER:
        sal_memcpy(p_db_aclqos_if->input_policer, p_aclqos_if->input_policer, sizeof(p_aclqos_if->input_policer));
        break;

    case TBL_ACLQOS_IF_FLD_OUTPUT_POLICER:
        sal_memcpy(p_db_aclqos_if->output_policer, p_aclqos_if->output_policer, sizeof(p_aclqos_if->output_policer));
        break;

    case TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
        sal_memcpy(p_db_aclqos_if->queue_shape_profile, p_aclqos_if->queue_shape_profile, sizeof(p_aclqos_if->queue_shape_profile));
        break;

    case TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
        sal_memcpy(p_db_aclqos_if->queue_drop_profile, p_aclqos_if->queue_drop_profile, sizeof(p_aclqos_if->queue_drop_profile));
        break;

    case TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
        sal_memcpy(p_db_aclqos_if->queue_drop_mode, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode));
        break;

    case TBL_ACLQOS_IF_FLD_REPLACE_DSCP:
        p_db_aclqos_if->replace_dscp = p_aclqos_if->replace_dscp;
        break;

    case TBL_ACLQOS_IF_FLD_REPLACE_COS:
        p_db_aclqos_if->replace_cos = p_aclqos_if->replace_cos;
        break;

    case TBL_ACLQOS_IF_FLD_TRUST:
        p_db_aclqos_if->trust = p_aclqos_if->trust;
        break;

    case TBL_ACLQOS_IF_FLD_PORT_TC:
        p_db_aclqos_if->port_tc = p_aclqos_if->port_tc;
        break;

    case TBL_ACLQOS_IF_FLD_MAX:
        sal_memcpy(p_db_aclqos_if, p_aclqos_if, sizeof(tbl_aclqos_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACLQOS_IF, field_id, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACLQOS_IF, field_id, p_db_aclqos_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_add_aclqos_if(tbl_aclqos_if_t *p_aclqos_if)
{
    return tbl_aclqos_if_add_aclqos_if_sync(p_aclqos_if, TRUE);
}

int32
tbl_aclqos_if_del_aclqos_if(tbl_aclqos_if_key_t *p_aclqos_if_key)
{
    return tbl_aclqos_if_del_aclqos_if_sync(p_aclqos_if_key, TRUE);
}

int32
tbl_aclqos_if_set_aclqos_if_field(tbl_aclqos_if_t *p_aclqos_if, tbl_aclqos_if_field_id_t field_id)
{
    return tbl_aclqos_if_set_aclqos_if_field_sync(p_aclqos_if, field_id, TRUE);
}

tbl_aclqos_if_t*
tbl_aclqos_if_get_aclqos_if(tbl_aclqos_if_key_t *p_aclqos_if_key)
{
    tbl_aclqos_if_master_t *p_master = _g_p_tbl_aclqos_if_master;
    tbl_aclqos_if_t lkp;

    sal_memcpy(&lkp.key, p_aclqos_if_key, sizeof(tbl_aclqos_if_key_t));
    return ctclib_hash_lookup(p_master->if_hash, &lkp);
}

char*
tbl_aclqos_if_key_val2str(tbl_aclqos_if_t *p_aclqos_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_aclqos_if->key.name);
    return str;
}

int32
tbl_aclqos_if_key_str2val(char *str, tbl_aclqos_if_t *p_aclqos_if)
{
    int32 ret = 0;

    sal_strcpy(p_aclqos_if->key.name, str);
    return ret;
}

char*
tbl_aclqos_if_key_name_dump(tbl_aclqos_if_t *p_aclqos_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_aclqos_if->key.name);
    return str;
}

char*
tbl_aclqos_if_key_value_dump(tbl_aclqos_if_t *p_aclqos_if, char *str)
{
    sal_sprintf(str, "%s", p_aclqos_if->key.name);
    return str;
}

char*
tbl_aclqos_if_field_name_dump(tbl_aclqos_if_t *p_aclqos_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACLQOS_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_aclqos_if_field_value_dump(tbl_aclqos_if_t *p_aclqos_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->key.name);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->ifindex);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->port_type);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_TRUST, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_TRUST));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_TC));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_COS_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->cos_domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DSCP_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->dscp_domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->port_shape_profile);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->input_policy_map);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->output_policy_map);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->input_policer);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->output_policer);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_shape_profile,
        sizeof(p_aclqos_if->queue_shape_profile[0]), sizeof(p_aclqos_if->queue_shape_profile)/sizeof(p_aclqos_if->queue_shape_profile[0]));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_drop_profile,
        sizeof(p_aclqos_if->queue_drop_profile[0]), sizeof(p_aclqos_if->queue_drop_profile)/sizeof(p_aclqos_if->queue_drop_profile[0]));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode)/4);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->replace_dscp);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_COS, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->replace_cos);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_TRUST, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->trust);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TC, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->port_tc);
    }
    return str;
}

char**
tbl_aclqos_if_table_dump(tbl_aclqos_if_t *p_aclqos_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_aclqos_if_key_name_dump(p_aclqos_if, buf));
    for(i=1; i<TBL_ACLQOS_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_aclqos_if_field_name_dump(p_aclqos_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_aclqos_if_field_value_dump(p_aclqos_if, i, buf));
    }
    return str;
}

int32
tbl_aclqos_if_field_value_parser(char *str, int32 field_id, tbl_aclqos_if_t *p_aclqos_if)
{
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_KEY, field_id))
    {
        sal_strcpy(p_aclqos_if->key.name, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_aclqos_if->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TYPE, field_id))
    {
        int32 ret;
        p_aclqos_if->port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_TRUST, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_TRUST);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC, field_id))
    {
        GLB_SET_FLAG(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_TC);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DOMAIN, field_id))
    {
        int32 ret;
        p_aclqos_if->domain = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_COS_DOMAIN, field_id))
    {
        int32 ret;
        p_aclqos_if->cos_domain = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DSCP_DOMAIN, field_id))
    {
        int32 ret;
        p_aclqos_if->dscp_domain = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE, field_id))
    {
        sal_strcpy(p_aclqos_if->port_shape_profile, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP, field_id))
    {
        sal_strcpy(p_aclqos_if->input_policy_map, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, field_id))
    {
        sal_strcpy(p_aclqos_if->output_policy_map, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICER, field_id))
    {
        sal_strcpy(p_aclqos_if->input_policer, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICER, field_id))
    {
        sal_strcpy(p_aclqos_if->output_policer, str);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_aclqos_if->queue_shape_profile));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_aclqos_if->queue_drop_profile));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_drop_mode);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_DSCP, field_id))
    {
        int32 ret;
        p_aclqos_if->replace_dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_COS, field_id))
    {
        int32 ret;
        p_aclqos_if->replace_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_TRUST, field_id))
    {
        int32 ret;
        p_aclqos_if->trust = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TC, field_id))
    {
        int32 ret;
        p_aclqos_if->port_tc = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_aclqos_if_table_parser(char** array, char* key_value,tbl_aclqos_if_t *p_aclqos_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_aclqos_if_key_str2val(key_value, p_aclqos_if));

    for(i=1; i<TBL_ACLQOS_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_aclqos_if_field_value_parser( array[j++], i, p_aclqos_if));
    }

    return PM_E_NONE;
}

int32
tbl_aclqos_if_dump_one(tbl_aclqos_if_t *p_aclqos_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_KEY].name,
            p_aclqos_if->key.name);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_IFINDEX].name,
            p_aclqos_if->ifindex);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_PORT_TYPE].name,
            p_aclqos_if->port_type);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_DOMAIN].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_DOMAIN));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_PORT_SHAPE_PROFILE].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_INPUT_POLICER].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_INPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_OUTPUT_POLICER].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_SHAPE_PROFILE].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_PROFILE].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_QUEUE_DROP_MODE].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_DSCP].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_REPLACE_COS].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_REPLACE_COS));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_TRUST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_TRUST].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_TRUST));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_FLAGS_PORT_TC].name,
            GLB_FLAG_ISSET(p_aclqos_if->flags, GLB_ACLQOS_IF_FLAGS_PORT_TC));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_DOMAIN].name,
            p_aclqos_if->domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_COS_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_COS_DOMAIN].name,
            p_aclqos_if->cos_domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_DSCP_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_DSCP_DOMAIN].name,
            p_aclqos_if->dscp_domain);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE].name,
            p_aclqos_if->port_shape_profile);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_INPUT_POLICY_MAP].name,
            p_aclqos_if->input_policy_map);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP].name,
            p_aclqos_if->output_policy_map);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_INPUT_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_INPUT_POLICER].name,
            p_aclqos_if->input_policer);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_OUTPUT_POLICER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_OUTPUT_POLICER].name,
            p_aclqos_if->output_policer);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE].name, 
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_shape_profile,
            sizeof(p_aclqos_if->queue_shape_profile[0]), sizeof(p_aclqos_if->queue_shape_profile)/sizeof(p_aclqos_if->queue_shape_profile[0])));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE].name, 
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_drop_profile,
            sizeof(p_aclqos_if->queue_drop_profile[0]), sizeof(p_aclqos_if->queue_drop_profile)/sizeof(p_aclqos_if->queue_drop_profile[0])));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode)/4));
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_REPLACE_DSCP].name,
            p_aclqos_if->replace_dscp);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_REPLACE_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_REPLACE_COS].name,
            p_aclqos_if->replace_cos);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_TRUST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_TRUST].name,
            p_aclqos_if->trust);
    }
    if (FLD_MATCH(TBL_ACLQOS_IF_FLD_PORT_TC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACLQOS_IF_FLD_PORT_TC].name,
            p_aclqos_if->port_tc);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_aclqos_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_aclqos_if_master_t *p_master = _g_p_tbl_aclqos_if_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_aclqos_if_t *p_db_aclqos_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_aclqos_if, listnode, next)
    {
        rc |= fn(p_db_aclqos_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_aclqos_if_master_t*
tbl_aclqos_if_get_master()
{
    return _g_p_tbl_aclqos_if_master;
}

tbl_aclqos_if_master_t*
tbl_aclqos_if_init_aclqos_if()
{
    _g_p_tbl_aclqos_if_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_aclqos_if_master_t));
    _g_p_tbl_aclqos_if_master->if_hash = ctclib_hash_create(_tbl_aclqos_if_hash_make, _tbl_aclqos_if_hash_cmp);
    _g_p_tbl_aclqos_if_master->if_list = ctclib_slist_create(tbl_aclqos_if_cmp, NULL);
    return _g_p_tbl_aclqos_if_master;
}

tbl_aclqos_if_t*
tbl_aclqos_if_get_aclqos_if_by_name(const char* ifname)
{
    tbl_aclqos_if_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_aclqos_if_get_aclqos_if(&key);
}

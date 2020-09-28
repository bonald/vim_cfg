
#include "proto.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "cdb_data_cmp.h"

tbl_acl_config_master_t *_g_p_tbl_acl_config_master = NULL;

static uint32
_tbl_acl_config_hash_make(tbl_acl_config_t *p_acl_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_config->key;
    for (index = 0; index < sizeof(p_acl_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_config_t *p_acl_config1 = (tbl_acl_config_t*)p_arg1;
    tbl_acl_config_t *p_acl_config2 = (tbl_acl_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_config1->key, &p_acl_config2->key, sizeof(tbl_acl_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_config_add_acl_config_sync(tbl_acl_config_t *p_acl_config, uint32 sync)
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;
    tbl_acl_config_t *p_db_acl_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_config_get_acl_config(&p_acl_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_config = XCALLOC(MEM_TBL_ACL_CONFIG, sizeof(tbl_acl_config_t));
    if (NULL == p_db_acl_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_config, p_acl_config, sizeof(tbl_acl_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_config_hash, (void*)p_db_acl_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->acl_config_list, p_db_acl_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_CONFIG, p_db_acl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_CONFIG, p_db_acl_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_config_del_acl_config_sync(tbl_acl_config_key_t *p_acl_config_key, uint32 sync)
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;
    tbl_acl_config_t *p_db_acl_config = NULL;

    /* 1. lookup entry exist */
    p_db_acl_config = tbl_acl_config_get_acl_config(p_acl_config_key);
    if (NULL == p_db_acl_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_CONFIG, p_db_acl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_CONFIG, p_db_acl_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_config_hash, (void*)p_db_acl_config);
    ctclib_slistnode_delete(p_master->acl_config_list, p_db_acl_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_CONFIG, p_db_acl_config);

    return PM_E_NONE;
}

int32
tbl_acl_config_set_acl_config_field_sync(tbl_acl_config_t *p_acl_config, tbl_acl_config_field_id_t field_id, uint32 sync)
{
    tbl_acl_config_t *p_db_acl_config = NULL;

    /* 1. lookup entry exist */
    p_db_acl_config = tbl_acl_config_get_acl_config(&p_acl_config->key);
    if (NULL == p_db_acl_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_CONFIG_FLD_ACE_REF:
        p_db_acl_config->ace_ref = p_acl_config->ace_ref;
        break;

    case TBL_ACL_CONFIG_FLD_ACEV6_REF:
        p_db_acl_config->acev6_ref = p_acl_config->acev6_ref;
        break;

    case TBL_ACL_CONFIG_FLD_ACEMAC_REF:
        p_db_acl_config->acemac_ref = p_acl_config->acemac_ref;
        break;

    case TBL_ACL_CONFIG_FLD_INTF_REF_IN:
        p_db_acl_config->intf_ref_in = p_acl_config->intf_ref_in;
        break;

    case TBL_ACL_CONFIG_FLD_INTF_REF_OUT:
        p_db_acl_config->intf_ref_out = p_acl_config->intf_ref_out;
        break;

    case TBL_ACL_CONFIG_FLD_L4_PORT_REF:
        p_db_acl_config->l4_port_ref = p_acl_config->l4_port_ref;
        break;

    case TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF:
        p_db_acl_config->tcp_flags_ref = p_acl_config->tcp_flags_ref;
        break;

    case TBL_ACL_CONFIG_FLD_REMARK:
        sal_memcpy(p_db_acl_config->remark, p_acl_config->remark, sizeof(p_acl_config->remark));
        break;

    case TBL_ACL_CONFIG_FLD_SEQ_REF:
        p_db_acl_config->seq_ref = p_acl_config->seq_ref;
        break;

    case TBL_ACL_CONFIG_FLD_ETHER_REF:
        p_db_acl_config->ether_ref = p_acl_config->ether_ref;
        break;

    case TBL_ACL_CONFIG_FLD_ARP_REF:
        p_db_acl_config->arp_ref = p_acl_config->arp_ref;
        break;

    case TBL_ACL_CONFIG_FLD_TAP_REF:
        p_db_acl_config->tap_ref = p_acl_config->tap_ref;
        break;

    case TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING:
        p_db_acl_config->type_identifying = p_acl_config->type_identifying;
        break;

    case TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT:
        p_db_acl_config->ace_truncation_ref_cnt = p_acl_config->ace_truncation_ref_cnt;
        break;

    case TBL_ACL_CONFIG_FLD_DECAP_EN:
        p_db_acl_config->decap_en = p_acl_config->decap_en;
        break;

    case TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX:
        p_db_acl_config->decap_inner_match_seq_max = p_acl_config->decap_inner_match_seq_max;
        break;

    case TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN:
        p_db_acl_config->decap_normal_seq_min = p_acl_config->decap_normal_seq_min;
        break;

    case TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED:
        p_db_acl_config->service_ipt_attached = p_acl_config->service_ipt_attached;
        break;

    case TBL_ACL_CONFIG_FLD_MAX:
        sal_memcpy(p_db_acl_config, p_acl_config, sizeof(tbl_acl_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_CONFIG, field_id, p_db_acl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_CONFIG, field_id, p_db_acl_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_config_add_acl_config(tbl_acl_config_t *p_acl_config)
{
    return tbl_acl_config_add_acl_config_sync(p_acl_config, TRUE);
}

int32
tbl_acl_config_del_acl_config(tbl_acl_config_key_t *p_acl_config_key)
{
    return tbl_acl_config_del_acl_config_sync(p_acl_config_key, TRUE);
}

int32
tbl_acl_config_set_acl_config_field(tbl_acl_config_t *p_acl_config, tbl_acl_config_field_id_t field_id)
{
    return tbl_acl_config_set_acl_config_field_sync(p_acl_config, field_id, TRUE);
}

tbl_acl_config_t*
tbl_acl_config_get_acl_config(tbl_acl_config_key_t *p_acl_config_key)
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;
    tbl_acl_config_t lkp;

    sal_memcpy(&lkp.key, p_acl_config_key, sizeof(tbl_acl_config_key_t));
    return ctclib_hash_lookup(p_master->acl_config_hash, &lkp);
}

char*
tbl_acl_config_key_val2str(tbl_acl_config_t *p_acl_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_acl_config->key.name);
    return str;
}

int32
tbl_acl_config_key_str2val(char *str, tbl_acl_config_t *p_acl_config)
{
    int32 ret = 0;

    sal_strcpy(p_acl_config->key.name, str);
    return ret;
}

char*
tbl_acl_config_key_name_dump(tbl_acl_config_t *p_acl_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_CONFIG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_acl_config->key.name);
    return str;
}

char*
tbl_acl_config_key_value_dump(tbl_acl_config_t *p_acl_config, char *str)
{
    sal_sprintf(str, "%s", p_acl_config->key.name);
    return str;
}

char*
tbl_acl_config_field_name_dump(tbl_acl_config_t *p_acl_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_config_field_value_dump(tbl_acl_config_t *p_acl_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_acl_config->key.name);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->ace_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEV6_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->acev6_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEMAC_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->acemac_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_IN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->intf_ref_in);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_OUT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->intf_ref_out);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_L4_PORT_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->l4_port_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->tcp_flags_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_REMARK, field_id))
    {
        sal_sprintf(str, "%s", p_acl_config->remark);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SEQ_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->seq_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ETHER_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->ether_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ARP_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->arp_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TAP_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->tap_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->type_identifying);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->ace_truncation_ref_cnt);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->decap_en);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->decap_inner_match_seq_max);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->decap_normal_seq_min);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED, field_id))
    {
        sal_sprintf(str, "%u", p_acl_config->service_ipt_attached);
    }
    return str;
}

char**
tbl_acl_config_table_dump(tbl_acl_config_t *p_acl_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_config_key_name_dump(p_acl_config, buf));
    for(i=1; i<TBL_ACL_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_config_field_name_dump(p_acl_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_config_field_value_dump(p_acl_config, i, buf));
    }
    return str;
}

int32
tbl_acl_config_field_value_parser(char *str, int32 field_id, tbl_acl_config_t *p_acl_config)
{
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_KEY, field_id))
    {
        sal_strcpy(p_acl_config->key.name, str);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_REF, field_id))
    {
        int32 ret;
        p_acl_config->ace_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEV6_REF, field_id))
    {
        int32 ret;
        p_acl_config->acev6_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEMAC_REF, field_id))
    {
        int32 ret;
        p_acl_config->acemac_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_IN, field_id))
    {
        int32 ret;
        p_acl_config->intf_ref_in = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_OUT, field_id))
    {
        int32 ret;
        p_acl_config->intf_ref_out = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_L4_PORT_REF, field_id))
    {
        int32 ret;
        p_acl_config->l4_port_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF, field_id))
    {
        int32 ret;
        p_acl_config->tcp_flags_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_REMARK, field_id))
    {
        sal_strcpy(p_acl_config->remark, str);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SEQ_REF, field_id))
    {
        int32 ret;
        p_acl_config->seq_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ETHER_REF, field_id))
    {
        int32 ret;
        p_acl_config->ether_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ARP_REF, field_id))
    {
        int32 ret;
        p_acl_config->arp_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TAP_REF, field_id))
    {
        int32 ret;
        p_acl_config->tap_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING, field_id))
    {
        int32 ret;
        p_acl_config->type_identifying = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT, field_id))
    {
        int32 ret;
        p_acl_config->ace_truncation_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_EN, field_id))
    {
        int32 ret;
        p_acl_config->decap_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX, field_id))
    {
        int32 ret;
        p_acl_config->decap_inner_match_seq_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN, field_id))
    {
        int32 ret;
        p_acl_config->decap_normal_seq_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED, field_id))
    {
        int32 ret;
        p_acl_config->service_ipt_attached = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_config_table_parser(char** array, char* key_value,tbl_acl_config_t *p_acl_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_config_key_str2val(key_value, p_acl_config));

    for(i=1; i<TBL_ACL_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_config_field_value_parser( array[j++], i, p_acl_config));
    }

    return PM_E_NONE;
}

int32
tbl_acl_config_dump_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_CONFIG_FLD_KEY].name,
            p_acl_config->key.name);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ACE_REF].name,
            p_acl_config->ace_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEV6_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ACEV6_REF].name,
            p_acl_config->acev6_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACEMAC_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ACEMAC_REF].name,
            p_acl_config->acemac_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_INTF_REF_IN].name,
            p_acl_config->intf_ref_in);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_INTF_REF_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_INTF_REF_OUT].name,
            p_acl_config->intf_ref_out);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_L4_PORT_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_L4_PORT_REF].name,
            p_acl_config->l4_port_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF].name,
            p_acl_config->tcp_flags_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_REMARK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_CONFIG_FLD_REMARK].name,
            p_acl_config->remark);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SEQ_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_SEQ_REF].name,
            p_acl_config->seq_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ETHER_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ETHER_REF].name,
            p_acl_config->ether_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ARP_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ARP_REF].name,
            p_acl_config->arp_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TAP_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_TAP_REF].name,
            p_acl_config->tap_ref);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING].name,
            p_acl_config->type_identifying);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT].name,
            p_acl_config->ace_truncation_ref_cnt);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_DECAP_EN].name,
            p_acl_config->decap_en);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX].name,
            p_acl_config->decap_inner_match_seq_max);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN].name,
            p_acl_config->decap_normal_seq_min);
    }
    if (FLD_MATCH(TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED].name,
            p_acl_config->service_ipt_attached);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_config_t *p_db_acl_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_config_list, p_db_acl_config, listnode, next)
    {
        rc |= fn(p_db_acl_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_config_master_t*
tbl_acl_config_get_master()
{
    return _g_p_tbl_acl_config_master;
}

tbl_acl_config_master_t*
tbl_acl_config_init_acl_config()
{
    _g_p_tbl_acl_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_config_master_t));
    _g_p_tbl_acl_config_master->acl_config_hash = ctclib_hash_create(_tbl_acl_config_hash_make, _tbl_acl_config_hash_cmp);
    _g_p_tbl_acl_config_master->acl_config_list = ctclib_slist_create(tbl_acl_config_cmp, NULL);
    return _g_p_tbl_acl_config_master;
}

int32
tbl_acl_config_get_acl_config_count()
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;

    return p_master->acl_config_hash->count;
}

int32
tbl_acl_config_get_l4port_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_config->l4_port_ref * (p_acl_config->intf_ref_in + p_acl_config->intf_ref_out);
    return PM_E_NONE;
}

int32
tbl_acl_config_get_l4port_config_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_acl_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_config_get_l4port_one, &args);
    return num;
}

int32
tbl_acl_config_get_tcp_flags_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_config->tcp_flags_ref * (p_acl_config->intf_ref_in + p_acl_config->intf_ref_out);
    return PM_E_NONE;
}

int32
tbl_acl_config_get_tcp_flags_config_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_acl_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_config_get_tcp_flags_one, &args);
    return num;
}




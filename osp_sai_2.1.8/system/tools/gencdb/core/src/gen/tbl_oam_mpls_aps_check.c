
#include "proto.h"
#include "gen/tbl_oam_mpls_aps_check_define.h"
#include "gen/tbl_oam_mpls_aps_check.h"
#include "cdb_data_cmp.h"

tbl_oam_mpls_aps_check_master_t *_g_p_tbl_oam_mpls_aps_check_master = NULL;

static uint32
_tbl_oam_mpls_aps_check_hash_make(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_oam_mpls_aps_check->key;
    for (index = 0; index < sizeof(p_oam_mpls_aps_check->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_oam_mpls_aps_check_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check1 = (tbl_oam_mpls_aps_check_t*)p_arg1;
    tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check2 = (tbl_oam_mpls_aps_check_t*)p_arg2;

    if (0 == sal_memcmp(&p_oam_mpls_aps_check1->key, &p_oam_mpls_aps_check2->key, sizeof(tbl_oam_mpls_aps_check_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_oam_mpls_aps_check_add_oam_mpls_aps_check_sync(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, uint32 sync)
{
    tbl_oam_mpls_aps_check_master_t *p_master = _g_p_tbl_oam_mpls_aps_check_master;
    tbl_oam_mpls_aps_check_t *p_db_oam_mpls_aps_check = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_oam_mpls_aps_check_get_oam_mpls_aps_check(&p_oam_mpls_aps_check->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_oam_mpls_aps_check = XCALLOC(MEM_TBL_OAM_MPLS_APS_CHECK, sizeof(tbl_oam_mpls_aps_check_t));
    if (NULL == p_db_oam_mpls_aps_check)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_oam_mpls_aps_check, p_oam_mpls_aps_check, sizeof(tbl_oam_mpls_aps_check_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->oam_mpls_aps_check_hash, (void*)p_db_oam_mpls_aps_check, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->oam_mpls_aps_check_list, p_db_oam_mpls_aps_check);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OAM_MPLS_APS_CHECK, p_db_oam_mpls_aps_check);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OAM_MPLS_APS_CHECK, p_db_oam_mpls_aps_check);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_del_oam_mpls_aps_check_sync(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key, uint32 sync)
{
    tbl_oam_mpls_aps_check_master_t *p_master = _g_p_tbl_oam_mpls_aps_check_master;
    tbl_oam_mpls_aps_check_t *p_db_oam_mpls_aps_check = NULL;

    /* 1. lookup entry exist */
    p_db_oam_mpls_aps_check = tbl_oam_mpls_aps_check_get_oam_mpls_aps_check(p_oam_mpls_aps_check_key);
    if (NULL == p_db_oam_mpls_aps_check)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OAM_MPLS_APS_CHECK, p_db_oam_mpls_aps_check);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OAM_MPLS_APS_CHECK, p_db_oam_mpls_aps_check);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->oam_mpls_aps_check_hash, (void*)p_db_oam_mpls_aps_check);
    ctclib_slistnode_delete(p_master->oam_mpls_aps_check_list, p_db_oam_mpls_aps_check);

    /* 4. free db entry */
    XFREE(MEM_TBL_OAM_MPLS_APS_CHECK, p_db_oam_mpls_aps_check);

    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field_sync(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_oam_mpls_aps_check_field_id_t field_id, uint32 sync)
{
    tbl_oam_mpls_aps_check_t *p_db_oam_mpls_aps_check = NULL;

    /* 1. lookup entry exist */
    p_db_oam_mpls_aps_check = tbl_oam_mpls_aps_check_get_oam_mpls_aps_check(&p_oam_mpls_aps_check->key);
    if (NULL == p_db_oam_mpls_aps_check)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL:
        p_db_oam_mpls_aps_check->lsp_label = p_oam_mpls_aps_check->lsp_label;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION:
        p_db_oam_mpls_aps_check->is_aps_action = p_oam_mpls_aps_check->is_aps_action;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT:
        p_db_oam_mpls_aps_check->ref_cnt = p_oam_mpls_aps_check->ref_cnt;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX:
        p_db_oam_mpls_aps_check->ifindex = p_oam_mpls_aps_check->ifindex;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC:
        sal_memcpy(p_db_oam_mpls_aps_check->nexthop_mac, p_oam_mpls_aps_check->nexthop_mac, sizeof(p_oam_mpls_aps_check->nexthop_mac));
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_VID:
        p_db_oam_mpls_aps_check->vid = p_oam_mpls_aps_check->vid;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP:
        p_db_oam_mpls_aps_check->outlabel_exp = p_oam_mpls_aps_check->outlabel_exp;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET:
        p_db_oam_mpls_aps_check->outlabel_exp_is_set = p_oam_mpls_aps_check->outlabel_exp_is_set;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL:
        p_db_oam_mpls_aps_check->outlabel_ttl = p_oam_mpls_aps_check->outlabel_ttl;
        break;

    case TBL_OAM_MPLS_APS_CHECK_FLD_MAX:
        sal_memcpy(p_db_oam_mpls_aps_check, p_oam_mpls_aps_check, sizeof(tbl_oam_mpls_aps_check_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OAM_MPLS_APS_CHECK, field_id, p_db_oam_mpls_aps_check);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OAM_MPLS_APS_CHECK, field_id, p_db_oam_mpls_aps_check);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_add_oam_mpls_aps_check(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check)
{
    return tbl_oam_mpls_aps_check_add_oam_mpls_aps_check_sync(p_oam_mpls_aps_check, TRUE);
}

int32
tbl_oam_mpls_aps_check_del_oam_mpls_aps_check(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key)
{
    return tbl_oam_mpls_aps_check_del_oam_mpls_aps_check_sync(p_oam_mpls_aps_check_key, TRUE);
}

int32
tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_oam_mpls_aps_check_field_id_t field_id)
{
    return tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field_sync(p_oam_mpls_aps_check, field_id, TRUE);
}

tbl_oam_mpls_aps_check_t*
tbl_oam_mpls_aps_check_get_oam_mpls_aps_check(tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key)
{
    tbl_oam_mpls_aps_check_master_t *p_master = _g_p_tbl_oam_mpls_aps_check_master;
    tbl_oam_mpls_aps_check_t lkp;

    sal_memcpy(&lkp.key, p_oam_mpls_aps_check_key, sizeof(tbl_oam_mpls_aps_check_key_t));
    return ctclib_hash_lookup(p_master->oam_mpls_aps_check_hash, &lkp);
}

char*
tbl_oam_mpls_aps_check_key_val2str(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_oam_mpls_aps_check->key.name);
    return str;
}

int32
tbl_oam_mpls_aps_check_key_str2val(char *str, tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check)
{
    int32 ret = 0;

    sal_strcpy(p_oam_mpls_aps_check->key.name, str);
    return ret;
}

char*
tbl_oam_mpls_aps_check_key_name_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_MPLS_APS_CHECK);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_oam_mpls_aps_check->key.name);
    return str;
}

char*
tbl_oam_mpls_aps_check_key_value_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char *str)
{
    sal_sprintf(str, "%s", p_oam_mpls_aps_check->key.name);
    return str;
}

char*
tbl_oam_mpls_aps_check_field_name_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_MPLS_APS_CHECK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OAM_MPLS_APS_CHECK_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_oam_mpls_aps_check_field_value_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_oam_mpls_aps_check->key.name);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->lsp_label);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->is_aps_action);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->ref_cnt);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->ifindex);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_oam_mpls_aps_check->nexthop_mac);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->vid);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->outlabel_exp);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_oam_mpls_aps_check->outlabel_ttl);
    }
    return str;
}

char**
tbl_oam_mpls_aps_check_table_dump(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_oam_mpls_aps_check_key_name_dump(p_oam_mpls_aps_check, buf));
    for(i=1; i<TBL_OAM_MPLS_APS_CHECK_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_oam_mpls_aps_check_field_name_dump(p_oam_mpls_aps_check, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_oam_mpls_aps_check_field_value_dump(p_oam_mpls_aps_check, i, buf));
    }
    return str;
}

int32
tbl_oam_mpls_aps_check_field_value_parser(char *str, int32 field_id, tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check)
{
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_KEY, field_id))
    {
        sal_strcpy(p_oam_mpls_aps_check->key.name, str);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->lsp_label = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->is_aps_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_oam_mpls_aps_check->nexthop_mac, str);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_VID, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->outlabel_exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->outlabel_exp_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL, field_id))
    {
        int32 ret;
        p_oam_mpls_aps_check->outlabel_ttl = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_table_parser(char** array, char* key_value,tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_MPLS_APS_CHECK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_oam_mpls_aps_check_key_str2val(key_value, p_oam_mpls_aps_check));

    for(i=1; i<TBL_OAM_MPLS_APS_CHECK_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_oam_mpls_aps_check_field_value_parser( array[j++], i, p_oam_mpls_aps_check));
    }

    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_dump_one(tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_MPLS_APS_CHECK);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_KEY].name,
            p_oam_mpls_aps_check->key.name);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL].name,
            p_oam_mpls_aps_check->lsp_label);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION].name,
            p_oam_mpls_aps_check->is_aps_action);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT].name,
            p_oam_mpls_aps_check->ref_cnt);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX].name,
            p_oam_mpls_aps_check->ifindex);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_oam_mpls_aps_check->nexthop_mac));
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_VID].name,
            p_oam_mpls_aps_check->vid);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP].name,
            p_oam_mpls_aps_check->outlabel_exp);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET].name,
            p_oam_mpls_aps_check->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL].name,
            p_oam_mpls_aps_check->outlabel_ttl);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_oam_mpls_aps_check_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_oam_mpls_aps_check_master_t *p_master = _g_p_tbl_oam_mpls_aps_check_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_oam_mpls_aps_check_t *p_db_oam_mpls_aps_check = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->oam_mpls_aps_check_list, p_db_oam_mpls_aps_check, listnode, next)
    {
        rc |= fn(p_db_oam_mpls_aps_check, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_oam_mpls_aps_check_master_t*
tbl_oam_mpls_aps_check_get_master()
{
    return _g_p_tbl_oam_mpls_aps_check_master;
}

tbl_oam_mpls_aps_check_master_t*
tbl_oam_mpls_aps_check_init_oam_mpls_aps_check()
{
    _g_p_tbl_oam_mpls_aps_check_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_oam_mpls_aps_check_master_t));
    _g_p_tbl_oam_mpls_aps_check_master->oam_mpls_aps_check_hash = ctclib_hash_create(_tbl_oam_mpls_aps_check_hash_make, _tbl_oam_mpls_aps_check_hash_cmp);
    _g_p_tbl_oam_mpls_aps_check_master->oam_mpls_aps_check_list = ctclib_slist_create(tbl_oam_mpls_aps_check_cmp, NULL);
    return _g_p_tbl_oam_mpls_aps_check_master;
}



#include "proto.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "cdb_data_cmp.h"

tbl_lsp_pe_master_t *_g_p_tbl_lsp_pe_master = NULL;

static uint32
_tbl_lsp_pe_hash_make(tbl_lsp_pe_t *p_lsp_pe)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_lsp_pe->key;
    for (index = 0; index < sizeof(p_lsp_pe->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_lsp_pe_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_lsp_pe_t *p_lsp_pe1 = (tbl_lsp_pe_t*)p_arg1;
    tbl_lsp_pe_t *p_lsp_pe2 = (tbl_lsp_pe_t*)p_arg2;

    if (0 == sal_memcmp(&p_lsp_pe1->key, &p_lsp_pe2->key, sizeof(tbl_lsp_pe_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_lsp_pe_add_lsp_pe_sync(tbl_lsp_pe_t *p_lsp_pe, uint32 sync)
{
    tbl_lsp_pe_master_t *p_master = _g_p_tbl_lsp_pe_master;
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_lsp_pe_get_lsp_pe(&p_lsp_pe->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lsp_pe = XCALLOC(MEM_TBL_LSP_PE, sizeof(tbl_lsp_pe_t));
    if (NULL == p_db_lsp_pe)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lsp_pe, p_lsp_pe, sizeof(tbl_lsp_pe_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->lsp_pe_hash, (void*)p_db_lsp_pe, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->lsp_pe_list, p_db_lsp_pe);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LSP_PE, p_db_lsp_pe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LSP_PE, p_db_lsp_pe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lsp_pe_del_lsp_pe_sync(tbl_lsp_pe_key_t *p_lsp_pe_key, uint32 sync)
{
    tbl_lsp_pe_master_t *p_master = _g_p_tbl_lsp_pe_master;
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;

    /* 1. lookup entry exist */
    p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(p_lsp_pe_key);
    if (NULL == p_db_lsp_pe)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LSP_PE, p_db_lsp_pe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LSP_PE, p_db_lsp_pe);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->lsp_pe_hash, (void*)p_db_lsp_pe);
    ctclib_slistnode_delete(p_master->lsp_pe_list, p_db_lsp_pe);

    /* 4. free db entry */
    XFREE(MEM_TBL_LSP_PE, p_db_lsp_pe);

    return PM_E_NONE;
}

int32
tbl_lsp_pe_set_lsp_pe_field_sync(tbl_lsp_pe_t *p_lsp_pe, tbl_lsp_pe_field_id_t field_id, uint32 sync)
{
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;

    /* 1. lookup entry exist */
    p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&p_lsp_pe->key);
    if (NULL == p_db_lsp_pe)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LSP_PE_FLD_REF_CNT:
        p_db_lsp_pe->ref_cnt = p_lsp_pe->ref_cnt;
        break;

    case TBL_LSP_PE_FLD_INLABEL:
        p_db_lsp_pe->inlabel = p_lsp_pe->inlabel;
        break;

    case TBL_LSP_PE_FLD_OUTLABEL:
        p_db_lsp_pe->outlabel = p_lsp_pe->outlabel;
        break;

    case TBL_LSP_PE_FLD_IFINDEX:
        p_db_lsp_pe->ifindex = p_lsp_pe->ifindex;
        break;

    case TBL_LSP_PE_FLD_VID:
        p_db_lsp_pe->vid = p_lsp_pe->vid;
        break;

    case TBL_LSP_PE_FLD_OUTLABEL_EXP:
        p_db_lsp_pe->outlabel_exp = p_lsp_pe->outlabel_exp;
        break;

    case TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET:
        p_db_lsp_pe->outlabel_exp_is_set = p_lsp_pe->outlabel_exp_is_set;
        break;

    case TBL_LSP_PE_FLD_OUTLABEL_TTL:
        p_db_lsp_pe->outlabel_ttl = p_lsp_pe->outlabel_ttl;
        break;

    case TBL_LSP_PE_FLD_OAM_SESSION_ID:
        p_db_lsp_pe->oam_session_id = p_lsp_pe->oam_session_id;
        break;

    case TBL_LSP_PE_FLD_LSP_APS_GROUP_ID:
        p_db_lsp_pe->lsp_aps_group_id = p_lsp_pe->lsp_aps_group_id;
        break;

    case TBL_LSP_PE_FLD_NEXTHOP_MAC:
        sal_memcpy(p_db_lsp_pe->nexthop_mac, p_lsp_pe->nexthop_mac, sizeof(p_lsp_pe->nexthop_mac));
        break;

    case TBL_LSP_PE_FLD_MAX:
        sal_memcpy(p_db_lsp_pe, p_lsp_pe, sizeof(tbl_lsp_pe_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LSP_PE, field_id, p_db_lsp_pe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LSP_PE, field_id, p_db_lsp_pe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lsp_pe_add_lsp_pe(tbl_lsp_pe_t *p_lsp_pe)
{
    return tbl_lsp_pe_add_lsp_pe_sync(p_lsp_pe, TRUE);
}

int32
tbl_lsp_pe_del_lsp_pe(tbl_lsp_pe_key_t *p_lsp_pe_key)
{
    return tbl_lsp_pe_del_lsp_pe_sync(p_lsp_pe_key, TRUE);
}

int32
tbl_lsp_pe_set_lsp_pe_field(tbl_lsp_pe_t *p_lsp_pe, tbl_lsp_pe_field_id_t field_id)
{
    return tbl_lsp_pe_set_lsp_pe_field_sync(p_lsp_pe, field_id, TRUE);
}

tbl_lsp_pe_t*
tbl_lsp_pe_get_lsp_pe(tbl_lsp_pe_key_t *p_lsp_pe_key)
{
    tbl_lsp_pe_master_t *p_master = _g_p_tbl_lsp_pe_master;
    tbl_lsp_pe_t lkp;

    sal_memcpy(&lkp.key, p_lsp_pe_key, sizeof(tbl_lsp_pe_key_t));
    return ctclib_hash_lookup(p_master->lsp_pe_hash, &lkp);
}

char*
tbl_lsp_pe_key_val2str(tbl_lsp_pe_t *p_lsp_pe, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_lsp_pe->key.name);
    return str;
}

int32
tbl_lsp_pe_key_str2val(char *str, tbl_lsp_pe_t *p_lsp_pe)
{
    int32 ret = 0;

    sal_strcpy(p_lsp_pe->key.name, str);
    return ret;
}

char*
tbl_lsp_pe_key_name_dump(tbl_lsp_pe_t *p_lsp_pe, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_PE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_lsp_pe->key.name);
    return str;
}

char*
tbl_lsp_pe_key_value_dump(tbl_lsp_pe_t *p_lsp_pe, char *str)
{
    sal_sprintf(str, "%s", p_lsp_pe->key.name);
    return str;
}

char*
tbl_lsp_pe_field_name_dump(tbl_lsp_pe_t *p_lsp_pe, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_PE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LSP_PE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lsp_pe_field_value_dump(tbl_lsp_pe_t *p_lsp_pe, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LSP_PE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_lsp_pe->key.name);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->ref_cnt);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_INLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->inlabel);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->outlabel);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->ifindex);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->vid);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OAM_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->oam_session_id);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_LSP_APS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_pe->lsp_aps_group_id);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lsp_pe->nexthop_mac);
    }
    return str;
}

char**
tbl_lsp_pe_table_dump(tbl_lsp_pe_t *p_lsp_pe, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lsp_pe_key_name_dump(p_lsp_pe, buf));
    for(i=1; i<TBL_LSP_PE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lsp_pe_field_name_dump(p_lsp_pe, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lsp_pe_field_value_dump(p_lsp_pe, i, buf));
    }
    return str;
}

int32
tbl_lsp_pe_field_value_parser(char *str, int32 field_id, tbl_lsp_pe_t *p_lsp_pe)
{
    if (FLD_MATCH(TBL_LSP_PE_FLD_KEY, field_id))
    {
        sal_strcpy(p_lsp_pe->key.name, str);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_lsp_pe->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_INLABEL, field_id))
    {
        int32 ret;
        p_lsp_pe->inlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL, field_id))
    {
        int32 ret;
        p_lsp_pe->outlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_lsp_pe->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_VID, field_id))
    {
        int32 ret;
        p_lsp_pe->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP, field_id))
    {
        int32 ret;
        p_lsp_pe->outlabel_exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        int32 ret;
        p_lsp_pe->outlabel_exp_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_TTL, field_id))
    {
        int32 ret;
        p_lsp_pe->outlabel_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OAM_SESSION_ID, field_id))
    {
        int32 ret;
        p_lsp_pe->oam_session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_LSP_APS_GROUP_ID, field_id))
    {
        int32 ret;
        p_lsp_pe->lsp_aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_lsp_pe->nexthop_mac, str);
    }
    return PM_E_NONE;
}

int32
tbl_lsp_pe_table_parser(char** array, char* key_value,tbl_lsp_pe_t *p_lsp_pe)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_PE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lsp_pe_key_str2val(key_value, p_lsp_pe));

    for(i=1; i<TBL_LSP_PE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lsp_pe_field_value_parser( array[j++], i, p_lsp_pe));
    }

    return PM_E_NONE;
}

int32
tbl_lsp_pe_dump_one(tbl_lsp_pe_t *p_lsp_pe, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_PE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LSP_PE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LSP_PE_FLD_KEY].name,
            p_lsp_pe->key.name);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_REF_CNT].name,
            p_lsp_pe->ref_cnt);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_INLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_INLABEL].name,
            p_lsp_pe->inlabel);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_OUTLABEL].name,
            p_lsp_pe->outlabel);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_IFINDEX].name,
            p_lsp_pe->ifindex);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_VID].name,
            p_lsp_pe->vid);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_OUTLABEL_EXP].name,
            p_lsp_pe->outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET].name,
            p_lsp_pe->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OUTLABEL_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_OUTLABEL_TTL].name,
            p_lsp_pe->outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_OAM_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_OAM_SESSION_ID].name,
            p_lsp_pe->oam_session_id);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_LSP_APS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_PE_FLD_LSP_APS_GROUP_ID].name,
            p_lsp_pe->lsp_aps_group_id);
    }
    if (FLD_MATCH(TBL_LSP_PE_FLD_NEXTHOP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LSP_PE_FLD_NEXTHOP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lsp_pe->nexthop_mac));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lsp_pe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lsp_pe_master_t *p_master = _g_p_tbl_lsp_pe_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->lsp_pe_list, p_db_lsp_pe, listnode, next)
    {
        rc |= fn(p_db_lsp_pe, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lsp_pe_master_t*
tbl_lsp_pe_get_master()
{
    return _g_p_tbl_lsp_pe_master;
}

tbl_lsp_pe_master_t*
tbl_lsp_pe_init_lsp_pe()
{
    _g_p_tbl_lsp_pe_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_lsp_pe_master_t));
    _g_p_tbl_lsp_pe_master->lsp_pe_hash = ctclib_hash_create(_tbl_lsp_pe_hash_make, _tbl_lsp_pe_hash_cmp);
    _g_p_tbl_lsp_pe_master->lsp_pe_list = ctclib_slist_create(tbl_lsp_pe_cmp, NULL);
    return _g_p_tbl_lsp_pe_master;
}


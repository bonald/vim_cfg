
#include "proto.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "cdb_data_cmp.h"

tbl_pw_tpe_master_t *_g_p_tbl_pw_tpe_master = NULL;

static uint32
_tbl_pw_tpe_hash_make(tbl_pw_tpe_t *p_pw_tpe)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pw_tpe->key;
    for (index = 0; index < sizeof(p_pw_tpe->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pw_tpe_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pw_tpe_t *p_pw_tpe1 = (tbl_pw_tpe_t*)p_arg1;
    tbl_pw_tpe_t *p_pw_tpe2 = (tbl_pw_tpe_t*)p_arg2;

    if (0 == sal_memcmp(&p_pw_tpe1->key, &p_pw_tpe2->key, sizeof(tbl_pw_tpe_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pw_tpe_add_pw_tpe_sync(tbl_pw_tpe_t *p_pw_tpe, uint32 sync)
{
    tbl_pw_tpe_master_t *p_master = _g_p_tbl_pw_tpe_master;
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pw_tpe_get_pw_tpe(&p_pw_tpe->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pw_tpe = XCALLOC(MEM_TBL_PW_TPE, sizeof(tbl_pw_tpe_t));
    if (NULL == p_db_pw_tpe)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pw_tpe, p_pw_tpe, sizeof(tbl_pw_tpe_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pw_tpe_hash, (void*)p_db_pw_tpe, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pw_tpe_list, p_db_pw_tpe);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PW_TPE, p_db_pw_tpe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PW_TPE, p_db_pw_tpe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pw_tpe_del_pw_tpe_sync(tbl_pw_tpe_key_t *p_pw_tpe_key, uint32 sync)
{
    tbl_pw_tpe_master_t *p_master = _g_p_tbl_pw_tpe_master;
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;

    /* 1. lookup entry exist */
    p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(p_pw_tpe_key);
    if (NULL == p_db_pw_tpe)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PW_TPE, p_db_pw_tpe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PW_TPE, p_db_pw_tpe);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pw_tpe_hash, (void*)p_db_pw_tpe);
    ctclib_slistnode_delete(p_master->pw_tpe_list, p_db_pw_tpe);

    /* 4. free db entry */
    XFREE(MEM_TBL_PW_TPE, p_db_pw_tpe);

    return PM_E_NONE;
}

int32
tbl_pw_tpe_set_pw_tpe_field_sync(tbl_pw_tpe_t *p_pw_tpe, tbl_pw_tpe_field_id_t field_id, uint32 sync)
{
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;

    /* 1. lookup entry exist */
    p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&p_pw_tpe->key);
    if (NULL == p_db_pw_tpe)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PW_TPE_FLD_REF_CNT:
        p_db_pw_tpe->ref_cnt = p_pw_tpe->ref_cnt;
        break;

    case TBL_PW_TPE_FLD_INLABEL:
        p_db_pw_tpe->inlabel = p_pw_tpe->inlabel;
        break;

    case TBL_PW_TPE_FLD_OUTLABEL:
        p_db_pw_tpe->outlabel = p_pw_tpe->outlabel;
        break;

    case TBL_PW_TPE_FLD_OUTLABEL_EXP:
        p_db_pw_tpe->outlabel_exp = p_pw_tpe->outlabel_exp;
        break;

    case TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET:
        p_db_pw_tpe->outlabel_exp_is_set = p_pw_tpe->outlabel_exp_is_set;
        break;

    case TBL_PW_TPE_FLD_OUTLABEL_TTL:
        p_db_pw_tpe->outlabel_ttl = p_pw_tpe->outlabel_ttl;
        break;

    case TBL_PW_TPE_FLD_TUNNEL_TYPE:
        p_db_pw_tpe->tunnel_type = p_pw_tpe->tunnel_type;
        break;

    case TBL_PW_TPE_FLD_LSP_NAME:
        sal_memcpy(p_db_pw_tpe->lsp_name, p_pw_tpe->lsp_name, sizeof(p_pw_tpe->lsp_name));
        break;

    case TBL_PW_TPE_FLD_LSP_APS_GROUP_ID:
        p_db_pw_tpe->lsp_aps_group_id = p_pw_tpe->lsp_aps_group_id;
        break;

    case TBL_PW_TPE_FLD_PW_APS_GROUP_ID:
        p_db_pw_tpe->pw_aps_group_id = p_pw_tpe->pw_aps_group_id;
        break;

    case TBL_PW_TPE_FLD_OAM_SESSION_ID:
        p_db_pw_tpe->oam_session_id = p_pw_tpe->oam_session_id;
        break;

    case TBL_PW_TPE_FLD_MAX:
        sal_memcpy(p_db_pw_tpe, p_pw_tpe, sizeof(tbl_pw_tpe_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PW_TPE, field_id, p_db_pw_tpe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PW_TPE, field_id, p_db_pw_tpe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pw_tpe_add_pw_tpe(tbl_pw_tpe_t *p_pw_tpe)
{
    return tbl_pw_tpe_add_pw_tpe_sync(p_pw_tpe, TRUE);
}

int32
tbl_pw_tpe_del_pw_tpe(tbl_pw_tpe_key_t *p_pw_tpe_key)
{
    return tbl_pw_tpe_del_pw_tpe_sync(p_pw_tpe_key, TRUE);
}

int32
tbl_pw_tpe_set_pw_tpe_field(tbl_pw_tpe_t *p_pw_tpe, tbl_pw_tpe_field_id_t field_id)
{
    return tbl_pw_tpe_set_pw_tpe_field_sync(p_pw_tpe, field_id, TRUE);
}

tbl_pw_tpe_t*
tbl_pw_tpe_get_pw_tpe(tbl_pw_tpe_key_t *p_pw_tpe_key)
{
    tbl_pw_tpe_master_t *p_master = _g_p_tbl_pw_tpe_master;
    tbl_pw_tpe_t lkp;

    sal_memcpy(&lkp.key, p_pw_tpe_key, sizeof(tbl_pw_tpe_key_t));
    return ctclib_hash_lookup(p_master->pw_tpe_hash, &lkp);
}

char*
tbl_pw_tpe_key_val2str(tbl_pw_tpe_t *p_pw_tpe, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_pw_tpe->key.name);
    return str;
}

int32
tbl_pw_tpe_key_str2val(char *str, tbl_pw_tpe_t *p_pw_tpe)
{
    int32 ret = 0;

    sal_strcpy(p_pw_tpe->key.name, str);
    return ret;
}

char*
tbl_pw_tpe_key_name_dump(tbl_pw_tpe_t *p_pw_tpe, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_TPE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_pw_tpe->key.name);
    return str;
}

char*
tbl_pw_tpe_key_value_dump(tbl_pw_tpe_t *p_pw_tpe, char *str)
{
    sal_sprintf(str, "%s", p_pw_tpe->key.name);
    return str;
}

char*
tbl_pw_tpe_field_name_dump(tbl_pw_tpe_t *p_pw_tpe, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_TPE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PW_TPE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pw_tpe_field_value_dump(tbl_pw_tpe_t *p_pw_tpe, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PW_TPE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_pw_tpe->key.name);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->ref_cnt);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_INLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->inlabel);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->outlabel);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->outlabel_exp);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->outlabel_ttl);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_TUNNEL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->tunnel_type);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_pw_tpe->lsp_name);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_APS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->lsp_aps_group_id);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_PW_APS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->pw_aps_group_id);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OAM_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_pw_tpe->oam_session_id);
    }
    return str;
}

char**
tbl_pw_tpe_table_dump(tbl_pw_tpe_t *p_pw_tpe, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pw_tpe_key_name_dump(p_pw_tpe, buf));
    for(i=1; i<TBL_PW_TPE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pw_tpe_field_name_dump(p_pw_tpe, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pw_tpe_field_value_dump(p_pw_tpe, i, buf));
    }
    return str;
}

int32
tbl_pw_tpe_field_value_parser(char *str, int32 field_id, tbl_pw_tpe_t *p_pw_tpe)
{
    if (FLD_MATCH(TBL_PW_TPE_FLD_KEY, field_id))
    {
        sal_strcpy(p_pw_tpe->key.name, str);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_pw_tpe->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_INLABEL, field_id))
    {
        int32 ret;
        p_pw_tpe->inlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL, field_id))
    {
        int32 ret;
        p_pw_tpe->outlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP, field_id))
    {
        int32 ret;
        p_pw_tpe->outlabel_exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET, field_id))
    {
        int32 ret;
        p_pw_tpe->outlabel_exp_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_TTL, field_id))
    {
        int32 ret;
        p_pw_tpe->outlabel_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_TUNNEL_TYPE, field_id))
    {
        int32 ret;
        p_pw_tpe->tunnel_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_NAME, field_id))
    {
        sal_strcpy(p_pw_tpe->lsp_name, str);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_APS_GROUP_ID, field_id))
    {
        int32 ret;
        p_pw_tpe->lsp_aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_PW_APS_GROUP_ID, field_id))
    {
        int32 ret;
        p_pw_tpe->pw_aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OAM_SESSION_ID, field_id))
    {
        int32 ret;
        p_pw_tpe->oam_session_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_pw_tpe_table_parser(char** array, char* key_value,tbl_pw_tpe_t *p_pw_tpe)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_TPE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pw_tpe_key_str2val(key_value, p_pw_tpe));

    for(i=1; i<TBL_PW_TPE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pw_tpe_field_value_parser( array[j++], i, p_pw_tpe));
    }

    return PM_E_NONE;
}

int32
tbl_pw_tpe_dump_one(tbl_pw_tpe_t *p_pw_tpe, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_TPE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PW_TPE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PW_TPE_FLD_KEY].name,
            p_pw_tpe->key.name);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_REF_CNT].name,
            p_pw_tpe->ref_cnt);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_INLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_INLABEL].name,
            p_pw_tpe->inlabel);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_OUTLABEL].name,
            p_pw_tpe->outlabel);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_OUTLABEL_EXP].name,
            p_pw_tpe->outlabel_exp);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET].name,
            p_pw_tpe->outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OUTLABEL_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_OUTLABEL_TTL].name,
            p_pw_tpe->outlabel_ttl);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_TUNNEL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_TUNNEL_TYPE].name,
            p_pw_tpe->tunnel_type);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PW_TPE_FLD_LSP_NAME].name,
            p_pw_tpe->lsp_name);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_LSP_APS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_LSP_APS_GROUP_ID].name,
            p_pw_tpe->lsp_aps_group_id);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_PW_APS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_PW_APS_GROUP_ID].name,
            p_pw_tpe->pw_aps_group_id);
    }
    if (FLD_MATCH(TBL_PW_TPE_FLD_OAM_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_TPE_FLD_OAM_SESSION_ID].name,
            p_pw_tpe->oam_session_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pw_tpe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pw_tpe_master_t *p_master = _g_p_tbl_pw_tpe_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pw_tpe_list, p_db_pw_tpe, listnode, next)
    {
        rc |= fn(p_db_pw_tpe, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pw_tpe_master_t*
tbl_pw_tpe_get_master()
{
    return _g_p_tbl_pw_tpe_master;
}

tbl_pw_tpe_master_t*
tbl_pw_tpe_init_pw_tpe()
{
    _g_p_tbl_pw_tpe_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pw_tpe_master_t));
    _g_p_tbl_pw_tpe_master->pw_tpe_hash = ctclib_hash_create(_tbl_pw_tpe_hash_make, _tbl_pw_tpe_hash_cmp);
    _g_p_tbl_pw_tpe_master->pw_tpe_list = ctclib_slist_create(tbl_pw_tpe_cmp, NULL);
    return _g_p_tbl_pw_tpe_master;
}



#include "proto.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "cdb_data_cmp.h"

tbl_lsp_p_master_t *_g_p_tbl_lsp_p_master = NULL;

static uint32
_tbl_lsp_p_hash_make(tbl_lsp_p_t *p_lsp_p)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_lsp_p->key;
    for (index = 0; index < sizeof(p_lsp_p->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_lsp_p_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_lsp_p_t *p_lsp_p1 = (tbl_lsp_p_t*)p_arg1;
    tbl_lsp_p_t *p_lsp_p2 = (tbl_lsp_p_t*)p_arg2;

    if (0 == sal_memcmp(&p_lsp_p1->key, &p_lsp_p2->key, sizeof(tbl_lsp_p_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_lsp_p_add_lsp_p_sync(tbl_lsp_p_t *p_lsp_p, uint32 sync)
{
    tbl_lsp_p_master_t *p_master = _g_p_tbl_lsp_p_master;
    tbl_lsp_p_t *p_db_lsp_p = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_lsp_p_get_lsp_p(&p_lsp_p->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lsp_p = XCALLOC(MEM_TBL_LSP_P, sizeof(tbl_lsp_p_t));
    if (NULL == p_db_lsp_p)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lsp_p, p_lsp_p, sizeof(tbl_lsp_p_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->lsp_p_hash, (void*)p_db_lsp_p, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->lsp_p_list, p_db_lsp_p);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LSP_P, p_db_lsp_p);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LSP_P, p_db_lsp_p);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lsp_p_del_lsp_p_sync(tbl_lsp_p_key_t *p_lsp_p_key, uint32 sync)
{
    tbl_lsp_p_master_t *p_master = _g_p_tbl_lsp_p_master;
    tbl_lsp_p_t *p_db_lsp_p = NULL;

    /* 1. lookup entry exist */
    p_db_lsp_p = tbl_lsp_p_get_lsp_p(p_lsp_p_key);
    if (NULL == p_db_lsp_p)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LSP_P, p_db_lsp_p);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LSP_P, p_db_lsp_p);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->lsp_p_hash, (void*)p_db_lsp_p);
    ctclib_slistnode_delete(p_master->lsp_p_list, p_db_lsp_p);

    /* 4. free db entry */
    XFREE(MEM_TBL_LSP_P, p_db_lsp_p);

    return PM_E_NONE;
}

int32
tbl_lsp_p_set_lsp_p_field_sync(tbl_lsp_p_t *p_lsp_p, tbl_lsp_p_field_id_t field_id, uint32 sync)
{
    tbl_lsp_p_t *p_db_lsp_p = NULL;

    /* 1. lookup entry exist */
    p_db_lsp_p = tbl_lsp_p_get_lsp_p(&p_lsp_p->key);
    if (NULL == p_db_lsp_p)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LSP_P_FLD_REF_CNT:
        p_db_lsp_p->ref_cnt = p_lsp_p->ref_cnt;
        break;

    case TBL_LSP_P_FLD_WEST_INLABEL:
        p_db_lsp_p->west_inlabel = p_lsp_p->west_inlabel;
        break;

    case TBL_LSP_P_FLD_WEST_OUTLABEL:
        p_db_lsp_p->west_outlabel = p_lsp_p->west_outlabel;
        break;

    case TBL_LSP_P_FLD_WEST_IFINDEX:
        p_db_lsp_p->west_ifindex = p_lsp_p->west_ifindex;
        break;

    case TBL_LSP_P_FLD_WEST_VID:
        p_db_lsp_p->west_vid = p_lsp_p->west_vid;
        break;

    case TBL_LSP_P_FLD_WEST_NEXTHOP_MAC:
        sal_memcpy(p_db_lsp_p->west_nexthop_mac, p_lsp_p->west_nexthop_mac, sizeof(p_lsp_p->west_nexthop_mac));
        break;

    case TBL_LSP_P_FLD_WEST_OUTLABEL_EXP:
        p_db_lsp_p->west_outlabel_exp = p_lsp_p->west_outlabel_exp;
        break;

    case TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET:
        p_db_lsp_p->west_outlabel_exp_is_set = p_lsp_p->west_outlabel_exp_is_set;
        break;

    case TBL_LSP_P_FLD_WEST_OUTLABEL_TTL:
        p_db_lsp_p->west_outlabel_ttl = p_lsp_p->west_outlabel_ttl;
        break;

    case TBL_LSP_P_FLD_EAST_INLABEL:
        p_db_lsp_p->east_inlabel = p_lsp_p->east_inlabel;
        break;

    case TBL_LSP_P_FLD_EAST_OUTLABEL:
        p_db_lsp_p->east_outlabel = p_lsp_p->east_outlabel;
        break;

    case TBL_LSP_P_FLD_EAST_IFINDEX:
        p_db_lsp_p->east_ifindex = p_lsp_p->east_ifindex;
        break;

    case TBL_LSP_P_FLD_EAST_VID:
        p_db_lsp_p->east_vid = p_lsp_p->east_vid;
        break;

    case TBL_LSP_P_FLD_EAST_OUTLABEL_EXP:
        p_db_lsp_p->east_outlabel_exp = p_lsp_p->east_outlabel_exp;
        break;

    case TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET:
        p_db_lsp_p->east_outlabel_exp_is_set = p_lsp_p->east_outlabel_exp_is_set;
        break;

    case TBL_LSP_P_FLD_EAST_OUTLABEL_TTL:
        p_db_lsp_p->east_outlabel_ttl = p_lsp_p->east_outlabel_ttl;
        break;

    case TBL_LSP_P_FLD_EAST_NEXTHOP_MAC:
        sal_memcpy(p_db_lsp_p->east_nexthop_mac, p_lsp_p->east_nexthop_mac, sizeof(p_lsp_p->east_nexthop_mac));
        break;

    case TBL_LSP_P_FLD_OAM_SESSION_ID:
        p_db_lsp_p->oam_session_id = p_lsp_p->oam_session_id;
        break;

    case TBL_LSP_P_FLD_MAX:
        sal_memcpy(p_db_lsp_p, p_lsp_p, sizeof(tbl_lsp_p_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LSP_P, field_id, p_db_lsp_p);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LSP_P, field_id, p_db_lsp_p);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lsp_p_add_lsp_p(tbl_lsp_p_t *p_lsp_p)
{
    return tbl_lsp_p_add_lsp_p_sync(p_lsp_p, TRUE);
}

int32
tbl_lsp_p_del_lsp_p(tbl_lsp_p_key_t *p_lsp_p_key)
{
    return tbl_lsp_p_del_lsp_p_sync(p_lsp_p_key, TRUE);
}

int32
tbl_lsp_p_set_lsp_p_field(tbl_lsp_p_t *p_lsp_p, tbl_lsp_p_field_id_t field_id)
{
    return tbl_lsp_p_set_lsp_p_field_sync(p_lsp_p, field_id, TRUE);
}

tbl_lsp_p_t*
tbl_lsp_p_get_lsp_p(tbl_lsp_p_key_t *p_lsp_p_key)
{
    tbl_lsp_p_master_t *p_master = _g_p_tbl_lsp_p_master;
    tbl_lsp_p_t lkp;

    sal_memcpy(&lkp.key, p_lsp_p_key, sizeof(tbl_lsp_p_key_t));
    return ctclib_hash_lookup(p_master->lsp_p_hash, &lkp);
}

char*
tbl_lsp_p_key_val2str(tbl_lsp_p_t *p_lsp_p, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_lsp_p->key.name);
    return str;
}

int32
tbl_lsp_p_key_str2val(char *str, tbl_lsp_p_t *p_lsp_p)
{
    int32 ret = 0;

    sal_strcpy(p_lsp_p->key.name, str);
    return ret;
}

char*
tbl_lsp_p_key_name_dump(tbl_lsp_p_t *p_lsp_p, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_P);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_lsp_p->key.name);
    return str;
}

char*
tbl_lsp_p_key_value_dump(tbl_lsp_p_t *p_lsp_p, char *str)
{
    sal_sprintf(str, "%s", p_lsp_p->key.name);
    return str;
}

char*
tbl_lsp_p_field_name_dump(tbl_lsp_p_t *p_lsp_p, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_P);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LSP_P_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lsp_p_field_value_dump(tbl_lsp_p_t *p_lsp_p, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LSP_P_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_lsp_p->key.name);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->ref_cnt);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_INLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_inlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_outlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_ifindex);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_VID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_vid);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lsp_p->west_nexthop_mac);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->west_outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_INLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_inlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_outlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_ifindex);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_VID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_vid);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->east_outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lsp_p->east_nexthop_mac);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_OAM_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lsp_p->oam_session_id);
    }
    return str;
}

char**
tbl_lsp_p_table_dump(tbl_lsp_p_t *p_lsp_p, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lsp_p_key_name_dump(p_lsp_p, buf));
    for(i=1; i<TBL_LSP_P_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lsp_p_field_name_dump(p_lsp_p, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lsp_p_field_value_dump(p_lsp_p, i, buf));
    }
    return str;
}

int32
tbl_lsp_p_field_value_parser(char *str, int32 field_id, tbl_lsp_p_t *p_lsp_p)
{
    if (FLD_MATCH(TBL_LSP_P_FLD_KEY, field_id))
    {
        sal_strcpy(p_lsp_p->key.name, str);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_lsp_p->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_INLABEL, field_id))
    {
        int32 ret;
        p_lsp_p->west_inlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL, field_id))
    {
        int32 ret;
        p_lsp_p->west_outlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_IFINDEX, field_id))
    {
        int32 ret;
        p_lsp_p->west_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_VID, field_id))
    {
        int32 ret;
        p_lsp_p->west_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_lsp_p->west_nexthop_mac, str);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP, field_id))
    {
        int32 ret;
        p_lsp_p->west_outlabel_exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET, field_id))
    {
        int32 ret;
        p_lsp_p->west_outlabel_exp_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_TTL, field_id))
    {
        int32 ret;
        p_lsp_p->west_outlabel_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_INLABEL, field_id))
    {
        int32 ret;
        p_lsp_p->east_inlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL, field_id))
    {
        int32 ret;
        p_lsp_p->east_outlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_IFINDEX, field_id))
    {
        int32 ret;
        p_lsp_p->east_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_VID, field_id))
    {
        int32 ret;
        p_lsp_p->east_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP, field_id))
    {
        int32 ret;
        p_lsp_p->east_outlabel_exp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET, field_id))
    {
        int32 ret;
        p_lsp_p->east_outlabel_exp_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_TTL, field_id))
    {
        int32 ret;
        p_lsp_p->east_outlabel_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_NEXTHOP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_lsp_p->east_nexthop_mac, str);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_OAM_SESSION_ID, field_id))
    {
        int32 ret;
        p_lsp_p->oam_session_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_lsp_p_table_parser(char** array, char* key_value,tbl_lsp_p_t *p_lsp_p)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_P);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lsp_p_key_str2val(key_value, p_lsp_p));

    for(i=1; i<TBL_LSP_P_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lsp_p_field_value_parser( array[j++], i, p_lsp_p));
    }

    return PM_E_NONE;
}

int32
tbl_lsp_p_dump_one(tbl_lsp_p_t *p_lsp_p, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LSP_P);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LSP_P_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LSP_P_FLD_KEY].name,
            p_lsp_p->key.name);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_REF_CNT].name,
            p_lsp_p->ref_cnt);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_INLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_INLABEL].name,
            p_lsp_p->west_inlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_OUTLABEL].name,
            p_lsp_p->west_outlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_IFINDEX].name,
            p_lsp_p->west_ifindex);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_VID].name,
            p_lsp_p->west_vid);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_NEXTHOP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LSP_P_FLD_WEST_NEXTHOP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lsp_p->west_nexthop_mac));
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_OUTLABEL_EXP].name,
            p_lsp_p->west_outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET].name,
            p_lsp_p->west_outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_WEST_OUTLABEL_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_WEST_OUTLABEL_TTL].name,
            p_lsp_p->west_outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_INLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_INLABEL].name,
            p_lsp_p->east_inlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_OUTLABEL].name,
            p_lsp_p->east_outlabel);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_IFINDEX].name,
            p_lsp_p->east_ifindex);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_VID].name,
            p_lsp_p->east_vid);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_OUTLABEL_EXP].name,
            p_lsp_p->east_outlabel_exp);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET].name,
            p_lsp_p->east_outlabel_exp_is_set);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_OUTLABEL_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_EAST_OUTLABEL_TTL].name,
            p_lsp_p->east_outlabel_ttl);
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_EAST_NEXTHOP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LSP_P_FLD_EAST_NEXTHOP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lsp_p->east_nexthop_mac));
    }
    if (FLD_MATCH(TBL_LSP_P_FLD_OAM_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LSP_P_FLD_OAM_SESSION_ID].name,
            p_lsp_p->oam_session_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lsp_p_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lsp_p_master_t *p_master = _g_p_tbl_lsp_p_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_lsp_p_t *p_db_lsp_p = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->lsp_p_list, p_db_lsp_p, listnode, next)
    {
        rc |= fn(p_db_lsp_p, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lsp_p_master_t*
tbl_lsp_p_get_master()
{
    return _g_p_tbl_lsp_p_master;
}

tbl_lsp_p_master_t*
tbl_lsp_p_init_lsp_p()
{
    _g_p_tbl_lsp_p_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_lsp_p_master_t));
    _g_p_tbl_lsp_p_master->lsp_p_hash = ctclib_hash_create(_tbl_lsp_p_hash_make, _tbl_lsp_p_hash_cmp);
    _g_p_tbl_lsp_p_master->lsp_p_list = ctclib_slist_create(tbl_lsp_p_cmp, NULL);
    return _g_p_tbl_lsp_p_master;
}


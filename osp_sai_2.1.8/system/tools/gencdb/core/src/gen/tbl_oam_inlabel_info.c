
#include "proto.h"
#include "gen/tbl_oam_inlabel_info_define.h"
#include "gen/tbl_oam_inlabel_info.h"
#include "cdb_data_cmp.h"

tbl_oam_inlabel_info_master_t *_g_p_tbl_oam_inlabel_info_master = NULL;

static uint32
_tbl_oam_inlabel_info_hash_make(tbl_oam_inlabel_info_t *p_oam_inlabel_info)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_oam_inlabel_info->key;
    for (index = 0; index < sizeof(p_oam_inlabel_info->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_oam_inlabel_info_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_oam_inlabel_info_t *p_oam_inlabel_info1 = (tbl_oam_inlabel_info_t*)p_arg1;
    tbl_oam_inlabel_info_t *p_oam_inlabel_info2 = (tbl_oam_inlabel_info_t*)p_arg2;

    if (0 == sal_memcmp(&p_oam_inlabel_info1->key, &p_oam_inlabel_info2->key, sizeof(tbl_oam_inlabel_info_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_oam_inlabel_info_add_oam_inlabel_info_sync(tbl_oam_inlabel_info_t *p_oam_inlabel_info, uint32 sync)
{
    tbl_oam_inlabel_info_master_t *p_master = _g_p_tbl_oam_inlabel_info_master;
    tbl_oam_inlabel_info_t *p_db_oam_inlabel_info = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_oam_inlabel_info_get_oam_inlabel_info(&p_oam_inlabel_info->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_oam_inlabel_info = XCALLOC(MEM_TBL_OAM_INLABEL_INFO, sizeof(tbl_oam_inlabel_info_t));
    if (NULL == p_db_oam_inlabel_info)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_oam_inlabel_info, p_oam_inlabel_info, sizeof(tbl_oam_inlabel_info_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->oam_inlabel_info_hash, (void*)p_db_oam_inlabel_info, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->oam_inlabel_info_list, p_db_oam_inlabel_info);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OAM_INLABEL_INFO, p_db_oam_inlabel_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OAM_INLABEL_INFO, p_db_oam_inlabel_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_del_oam_inlabel_info_sync(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key, uint32 sync)
{
    tbl_oam_inlabel_info_master_t *p_master = _g_p_tbl_oam_inlabel_info_master;
    tbl_oam_inlabel_info_t *p_db_oam_inlabel_info = NULL;

    /* 1. lookup entry exist */
    p_db_oam_inlabel_info = tbl_oam_inlabel_info_get_oam_inlabel_info(p_oam_inlabel_info_key);
    if (NULL == p_db_oam_inlabel_info)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OAM_INLABEL_INFO, p_db_oam_inlabel_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OAM_INLABEL_INFO, p_db_oam_inlabel_info);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->oam_inlabel_info_hash, (void*)p_db_oam_inlabel_info);
    ctclib_slistnode_delete(p_master->oam_inlabel_info_list, p_db_oam_inlabel_info);

    /* 4. free db entry */
    XFREE(MEM_TBL_OAM_INLABEL_INFO, p_db_oam_inlabel_info);

    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_set_oam_inlabel_info_field_sync(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_oam_inlabel_info_field_id_t field_id, uint32 sync)
{
    tbl_oam_inlabel_info_t *p_db_oam_inlabel_info = NULL;

    /* 1. lookup entry exist */
    p_db_oam_inlabel_info = tbl_oam_inlabel_info_get_oam_inlabel_info(&p_oam_inlabel_info->key);
    if (NULL == p_db_oam_inlabel_info)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OAM_INLABEL_INFO_FLD_REF_CNT:
        p_db_oam_inlabel_info->ref_cnt = p_oam_inlabel_info->ref_cnt;
        break;

    case TBL_OAM_INLABEL_INFO_FLD_MAX:
        sal_memcpy(p_db_oam_inlabel_info, p_oam_inlabel_info, sizeof(tbl_oam_inlabel_info_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OAM_INLABEL_INFO, field_id, p_db_oam_inlabel_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OAM_INLABEL_INFO, field_id, p_db_oam_inlabel_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_add_oam_inlabel_info(tbl_oam_inlabel_info_t *p_oam_inlabel_info)
{
    return tbl_oam_inlabel_info_add_oam_inlabel_info_sync(p_oam_inlabel_info, TRUE);
}

int32
tbl_oam_inlabel_info_del_oam_inlabel_info(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key)
{
    return tbl_oam_inlabel_info_del_oam_inlabel_info_sync(p_oam_inlabel_info_key, TRUE);
}

int32
tbl_oam_inlabel_info_set_oam_inlabel_info_field(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_oam_inlabel_info_field_id_t field_id)
{
    return tbl_oam_inlabel_info_set_oam_inlabel_info_field_sync(p_oam_inlabel_info, field_id, TRUE);
}

tbl_oam_inlabel_info_t*
tbl_oam_inlabel_info_get_oam_inlabel_info(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key)
{
    tbl_oam_inlabel_info_master_t *p_master = _g_p_tbl_oam_inlabel_info_master;
    tbl_oam_inlabel_info_t lkp;

    sal_memcpy(&lkp.key, p_oam_inlabel_info_key, sizeof(tbl_oam_inlabel_info_key_t));
    return ctclib_hash_lookup(p_master->oam_inlabel_info_hash, &lkp);
}

char*
tbl_oam_inlabel_info_key_val2str(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_oam_inlabel_info->key.inlabel);
    return str;
}

int32
tbl_oam_inlabel_info_key_str2val(char *str, tbl_oam_inlabel_info_t *p_oam_inlabel_info)
{
    int32 ret = 0;

    p_oam_inlabel_info->key.inlabel = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_oam_inlabel_info_key_name_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_INLABEL_INFO);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_oam_inlabel_info->key.inlabel);
    return str;
}

char*
tbl_oam_inlabel_info_key_value_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char *str)
{
    sal_sprintf(str, "%u", p_oam_inlabel_info->key.inlabel);
    return str;
}

char*
tbl_oam_inlabel_info_field_name_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_INLABEL_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OAM_INLABEL_INFO_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_oam_inlabel_info_field_value_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_oam_inlabel_info->key.inlabel);
    }
    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_oam_inlabel_info->ref_cnt);
    }
    return str;
}

char**
tbl_oam_inlabel_info_table_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_oam_inlabel_info_key_name_dump(p_oam_inlabel_info, buf));
    for(i=1; i<TBL_OAM_INLABEL_INFO_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_oam_inlabel_info_field_name_dump(p_oam_inlabel_info, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_oam_inlabel_info_field_value_dump(p_oam_inlabel_info, i, buf));
    }
    return str;
}

int32
tbl_oam_inlabel_info_field_value_parser(char *str, int32 field_id, tbl_oam_inlabel_info_t *p_oam_inlabel_info)
{
    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_KEY, field_id))
    {
        int32 ret;
        p_oam_inlabel_info->key.inlabel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_oam_inlabel_info->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_table_parser(char** array, char* key_value,tbl_oam_inlabel_info_t *p_oam_inlabel_info)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_INLABEL_INFO);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_oam_inlabel_info_key_str2val(key_value, p_oam_inlabel_info));

    for(i=1; i<TBL_OAM_INLABEL_INFO_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_oam_inlabel_info_field_value_parser( array[j++], i, p_oam_inlabel_info));
    }

    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_dump_one(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_INLABEL_INFO);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_INLABEL_INFO_FLD_KEY].name,
            p_oam_inlabel_info->key.inlabel);
    }
    if (FLD_MATCH(TBL_OAM_INLABEL_INFO_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_INLABEL_INFO_FLD_REF_CNT].name,
            p_oam_inlabel_info->ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_oam_inlabel_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_oam_inlabel_info_master_t *p_master = _g_p_tbl_oam_inlabel_info_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_oam_inlabel_info_t *p_db_oam_inlabel_info = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->oam_inlabel_info_list, p_db_oam_inlabel_info, listnode, next)
    {
        rc |= fn(p_db_oam_inlabel_info, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_oam_inlabel_info_master_t*
tbl_oam_inlabel_info_get_master()
{
    return _g_p_tbl_oam_inlabel_info_master;
}

tbl_oam_inlabel_info_master_t*
tbl_oam_inlabel_info_init_oam_inlabel_info()
{
    _g_p_tbl_oam_inlabel_info_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_oam_inlabel_info_master_t));
    _g_p_tbl_oam_inlabel_info_master->oam_inlabel_info_hash = ctclib_hash_create(_tbl_oam_inlabel_info_hash_make, _tbl_oam_inlabel_info_hash_cmp);
    _g_p_tbl_oam_inlabel_info_master->oam_inlabel_info_list = ctclib_slist_create(tbl_oam_inlabel_info_cmp, NULL);
    return _g_p_tbl_oam_inlabel_info_master;
}


#include "proto.h"
#include "gen/tbl_pim_sm_rp_gp_define.h"
#include "gen/tbl_pim_sm_rp_gp.h"
#include "cdb_data_cmp.h"

tbl_pim_sm_rp_gp_master_t *_g_p_tbl_pim_sm_rp_gp_master = NULL;

static uint32
_tbl_pim_sm_rp_gp_hash_make(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pim_sm_rp_gp->key;
    for (index = 0; index < sizeof(p_pim_sm_rp_gp->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pim_sm_rp_gp_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp1 = (tbl_pim_sm_rp_gp_t*)p_arg1;
    tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp2 = (tbl_pim_sm_rp_gp_t*)p_arg2;

    if (0 == sal_memcmp(&p_pim_sm_rp_gp1->key, &p_pim_sm_rp_gp2->key, sizeof(tbl_pim_sm_rp_gp_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pim_sm_rp_gp_add_pim_sm_rp_gp_sync(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, uint32 sync)
{
    tbl_pim_sm_rp_gp_master_t *p_master = _g_p_tbl_pim_sm_rp_gp_master;
    tbl_pim_sm_rp_gp_t *p_db_pim_sm_rp_gp = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(&p_pim_sm_rp_gp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pim_sm_rp_gp = XCALLOC(MEM_TBL_PIM_SM_RP_GP, sizeof(tbl_pim_sm_rp_gp_t));
    if (NULL == p_db_pim_sm_rp_gp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pim_sm_rp_gp, p_pim_sm_rp_gp, sizeof(tbl_pim_sm_rp_gp_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pim_sm_rp_gp_hash, (void*)p_db_pim_sm_rp_gp, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pim_sm_rp_gp_list, p_db_pim_sm_rp_gp);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PIM_SM_RP_GP, p_db_pim_sm_rp_gp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PIM_SM_RP_GP, p_db_pim_sm_rp_gp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_del_pim_sm_rp_gp_sync(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key, uint32 sync)
{
    tbl_pim_sm_rp_gp_master_t *p_master = _g_p_tbl_pim_sm_rp_gp_master;
    tbl_pim_sm_rp_gp_t *p_db_pim_sm_rp_gp = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm_rp_gp = tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(p_pim_sm_rp_gp_key);
    if (NULL == p_db_pim_sm_rp_gp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PIM_SM_RP_GP, p_db_pim_sm_rp_gp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PIM_SM_RP_GP, p_db_pim_sm_rp_gp);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pim_sm_rp_gp_hash, (void*)p_db_pim_sm_rp_gp);
    ctclib_slistnode_delete(p_master->pim_sm_rp_gp_list, p_db_pim_sm_rp_gp);

    /* 4. free db entry */
    XFREE(MEM_TBL_PIM_SM_RP_GP, p_db_pim_sm_rp_gp);

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field_sync(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_pim_sm_rp_gp_field_id_t field_id, uint32 sync)
{
    tbl_pim_sm_rp_gp_t *p_db_pim_sm_rp_gp = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm_rp_gp = tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(&p_pim_sm_rp_gp->key);
    if (NULL == p_db_pim_sm_rp_gp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX:
        sal_memcpy(&p_db_pim_sm_rp_gp->group_prefix, &p_pim_sm_rp_gp->group_prefix, sizeof(p_pim_sm_rp_gp->group_prefix));
        break;

    case TBL_PIM_SM_RP_GP_FLD_MAX:
        sal_memcpy(p_db_pim_sm_rp_gp, p_pim_sm_rp_gp, sizeof(tbl_pim_sm_rp_gp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PIM_SM_RP_GP, field_id, p_db_pim_sm_rp_gp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PIM_SM_RP_GP, field_id, p_db_pim_sm_rp_gp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_add_pim_sm_rp_gp(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp)
{
    return tbl_pim_sm_rp_gp_add_pim_sm_rp_gp_sync(p_pim_sm_rp_gp, TRUE);
}

int32
tbl_pim_sm_rp_gp_del_pim_sm_rp_gp(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key)
{
    return tbl_pim_sm_rp_gp_del_pim_sm_rp_gp_sync(p_pim_sm_rp_gp_key, TRUE);
}

int32
tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_pim_sm_rp_gp_field_id_t field_id)
{
    return tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field_sync(p_pim_sm_rp_gp, field_id, TRUE);
}

tbl_pim_sm_rp_gp_t*
tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key)
{
    tbl_pim_sm_rp_gp_master_t *p_master = _g_p_tbl_pim_sm_rp_gp_master;
    tbl_pim_sm_rp_gp_t lkp;

    sal_memcpy(&lkp.key, p_pim_sm_rp_gp_key, sizeof(tbl_pim_sm_rp_gp_key_t));
    return ctclib_hash_lookup(p_master->pim_sm_rp_gp_hash, &lkp);
}

char*
tbl_pim_sm_rp_gp_key_val2str(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_pim_sm_rp_gp->key.id);
    return str;
}

int32
tbl_pim_sm_rp_gp_key_str2val(char *str, tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp)
{
    int32 ret = 0;

    p_pim_sm_rp_gp->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_pim_sm_rp_gp_key_name_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP_GP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_pim_sm_rp_gp->key.id);
    return str;
}

char*
tbl_pim_sm_rp_gp_key_value_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char *str)
{
    sal_sprintf(str, "%u", p_pim_sm_rp_gp->key.id);
    return str;
}

char*
tbl_pim_sm_rp_gp_field_name_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP_GP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PIM_SM_RP_GP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pim_sm_rp_gp_field_value_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm_rp_gp->key.id);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX, field_id))
    {
        cdb_pim_sm_gprefix_val2str(str, MAX_CMD_STR_LEN, &p_pim_sm_rp_gp->group_prefix);
    }
    return str;
}

char**
tbl_pim_sm_rp_gp_table_dump(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pim_sm_rp_gp_key_name_dump(p_pim_sm_rp_gp, buf));
    for(i=1; i<TBL_PIM_SM_RP_GP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_rp_gp_field_name_dump(p_pim_sm_rp_gp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_rp_gp_field_value_dump(p_pim_sm_rp_gp, i, buf));
    }
    return str;
}

int32
tbl_pim_sm_rp_gp_field_value_parser(char *str, int32 field_id, tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp)
{
    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_KEY, field_id))
    {
        int32 ret;
        p_pim_sm_rp_gp->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX, field_id))
    {
        cdb_pim_sm_gprefix_str2val(str, &p_pim_sm_rp_gp->group_prefix);
    }
    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_table_parser(char** array, char* key_value,tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP_GP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pim_sm_rp_gp_key_str2val(key_value, p_pim_sm_rp_gp));

    for(i=1; i<TBL_PIM_SM_RP_GP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pim_sm_rp_gp_field_value_parser( array[j++], i, p_pim_sm_rp_gp));
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_dump_one(tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP_GP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_RP_GP_FLD_KEY].name,
            p_pim_sm_rp_gp->key.id);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX].name, 
            cdb_pim_sm_gprefix_val2str(buf, MAX_CMD_STR_LEN, &p_pim_sm_rp_gp->group_prefix));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_gp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pim_sm_rp_gp_master_t *p_master = _g_p_tbl_pim_sm_rp_gp_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pim_sm_rp_gp_t *p_db_pim_sm_rp_gp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_gp_list, p_db_pim_sm_rp_gp, listnode, next)
    {
        rc |= fn(p_db_pim_sm_rp_gp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pim_sm_rp_gp_master_t*
tbl_pim_sm_rp_gp_get_master()
{
    return _g_p_tbl_pim_sm_rp_gp_master;
}

tbl_pim_sm_rp_gp_master_t*
tbl_pim_sm_rp_gp_init_pim_sm_rp_gp()
{
    _g_p_tbl_pim_sm_rp_gp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pim_sm_rp_gp_master_t));
    _g_p_tbl_pim_sm_rp_gp_master->pim_sm_rp_gp_hash = ctclib_hash_create(_tbl_pim_sm_rp_gp_hash_make, _tbl_pim_sm_rp_gp_hash_cmp);
    _g_p_tbl_pim_sm_rp_gp_master->pim_sm_rp_gp_list = ctclib_slist_create(tbl_pim_sm_rp_gp_cmp, NULL);
    return _g_p_tbl_pim_sm_rp_gp_master;
}



#include "proto.h"
#include "gen/tbl_pim_sm_rp_define.h"
#include "gen/tbl_pim_sm_rp.h"
#include "cdb_data_cmp.h"

tbl_pim_sm_rp_master_t *_g_p_tbl_pim_sm_rp_master = NULL;

static uint32
_tbl_pim_sm_rp_hash_make(tbl_pim_sm_rp_t *p_pim_sm_rp)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pim_sm_rp->key;
    for (index = 0; index < sizeof(p_pim_sm_rp->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pim_sm_rp_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pim_sm_rp_t *p_pim_sm_rp1 = (tbl_pim_sm_rp_t*)p_arg1;
    tbl_pim_sm_rp_t *p_pim_sm_rp2 = (tbl_pim_sm_rp_t*)p_arg2;

    if (0 == sal_memcmp(&p_pim_sm_rp1->key, &p_pim_sm_rp2->key, sizeof(tbl_pim_sm_rp_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pim_sm_rp_add_pim_sm_rp_sync(tbl_pim_sm_rp_t *p_pim_sm_rp, uint32 sync)
{
    tbl_pim_sm_rp_master_t *p_master = _g_p_tbl_pim_sm_rp_master;
    tbl_pim_sm_rp_t *p_db_pim_sm_rp = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pim_sm_rp_get_pim_sm_rp(&p_pim_sm_rp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pim_sm_rp = XCALLOC(MEM_TBL_PIM_SM_RP, sizeof(tbl_pim_sm_rp_t));
    if (NULL == p_db_pim_sm_rp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pim_sm_rp, p_pim_sm_rp, sizeof(tbl_pim_sm_rp_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pim_sm_rp_hash, (void*)p_db_pim_sm_rp, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pim_sm_rp_list, p_db_pim_sm_rp);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PIM_SM_RP, p_db_pim_sm_rp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PIM_SM_RP, p_db_pim_sm_rp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_del_pim_sm_rp_sync(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key, uint32 sync)
{
    tbl_pim_sm_rp_master_t *p_master = _g_p_tbl_pim_sm_rp_master;
    tbl_pim_sm_rp_t *p_db_pim_sm_rp = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm_rp = tbl_pim_sm_rp_get_pim_sm_rp(p_pim_sm_rp_key);
    if (NULL == p_db_pim_sm_rp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PIM_SM_RP, p_db_pim_sm_rp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PIM_SM_RP, p_db_pim_sm_rp);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pim_sm_rp_hash, (void*)p_db_pim_sm_rp);
    ctclib_slistnode_delete(p_master->pim_sm_rp_list, p_db_pim_sm_rp);

    /* 4. free db entry */
    XFREE(MEM_TBL_PIM_SM_RP, p_db_pim_sm_rp);

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_set_pim_sm_rp_field_sync(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_pim_sm_rp_field_id_t field_id, uint32 sync)
{
    tbl_pim_sm_rp_t *p_db_pim_sm_rp = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm_rp = tbl_pim_sm_rp_get_pim_sm_rp(&p_pim_sm_rp->key);
    if (NULL == p_db_pim_sm_rp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PIM_SM_RP_FLD_PRIORITY:
        p_db_pim_sm_rp->priority = p_pim_sm_rp->priority;
        break;

    case TBL_PIM_SM_RP_FLD_INTERVAL:
        p_db_pim_sm_rp->interval = p_pim_sm_rp->interval;
        break;

    case TBL_PIM_SM_RP_FLD_MAX:
        sal_memcpy(p_db_pim_sm_rp, p_pim_sm_rp, sizeof(tbl_pim_sm_rp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PIM_SM_RP, field_id, p_db_pim_sm_rp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PIM_SM_RP, field_id, p_db_pim_sm_rp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_add_pim_sm_rp(tbl_pim_sm_rp_t *p_pim_sm_rp)
{
    return tbl_pim_sm_rp_add_pim_sm_rp_sync(p_pim_sm_rp, TRUE);
}

int32
tbl_pim_sm_rp_del_pim_sm_rp(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key)
{
    return tbl_pim_sm_rp_del_pim_sm_rp_sync(p_pim_sm_rp_key, TRUE);
}

int32
tbl_pim_sm_rp_set_pim_sm_rp_field(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_pim_sm_rp_field_id_t field_id)
{
    return tbl_pim_sm_rp_set_pim_sm_rp_field_sync(p_pim_sm_rp, field_id, TRUE);
}

tbl_pim_sm_rp_t*
tbl_pim_sm_rp_get_pim_sm_rp(tbl_pim_sm_rp_key_t *p_pim_sm_rp_key)
{
    tbl_pim_sm_rp_master_t *p_master = _g_p_tbl_pim_sm_rp_master;
    tbl_pim_sm_rp_t lkp;

    sal_memcpy(&lkp.key, p_pim_sm_rp_key, sizeof(tbl_pim_sm_rp_key_t));
    return ctclib_hash_lookup(p_master->pim_sm_rp_hash, &lkp);
}

char*
tbl_pim_sm_rp_key_val2str(tbl_pim_sm_rp_t *p_pim_sm_rp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_pim_sm_rp->key.name);
    return str;
}

int32
tbl_pim_sm_rp_key_str2val(char *str, tbl_pim_sm_rp_t *p_pim_sm_rp)
{
    int32 ret = 0;

    sal_strcpy(p_pim_sm_rp->key.name, str);
    return ret;
}

char*
tbl_pim_sm_rp_key_name_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_pim_sm_rp->key.name);
    return str;
}

char*
tbl_pim_sm_rp_key_value_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char *str)
{
    sal_sprintf(str, "%s", p_pim_sm_rp->key.name);
    return str;
}

char*
tbl_pim_sm_rp_field_name_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PIM_SM_RP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pim_sm_rp_field_value_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_pim_sm_rp->key.name);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm_rp->priority);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm_rp->interval);
    }
    return str;
}

char**
tbl_pim_sm_rp_table_dump(tbl_pim_sm_rp_t *p_pim_sm_rp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pim_sm_rp_key_name_dump(p_pim_sm_rp, buf));
    for(i=1; i<TBL_PIM_SM_RP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_rp_field_name_dump(p_pim_sm_rp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_rp_field_value_dump(p_pim_sm_rp, i, buf));
    }
    return str;
}

int32
tbl_pim_sm_rp_field_value_parser(char *str, int32 field_id, tbl_pim_sm_rp_t *p_pim_sm_rp)
{
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_KEY, field_id))
    {
        sal_strcpy(p_pim_sm_rp->key.name, str);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_pim_sm_rp->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_INTERVAL, field_id))
    {
        int32 ret;
        p_pim_sm_rp->interval = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_table_parser(char** array, char* key_value,tbl_pim_sm_rp_t *p_pim_sm_rp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pim_sm_rp_key_str2val(key_value, p_pim_sm_rp));

    for(i=1; i<TBL_PIM_SM_RP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pim_sm_rp_field_value_parser( array[j++], i, p_pim_sm_rp));
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_dump_one(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM_RP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PIM_SM_RP_FLD_KEY].name,
            p_pim_sm_rp->key.name);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_RP_FLD_PRIORITY].name,
            p_pim_sm_rp->priority);
    }
    if (FLD_MATCH(TBL_PIM_SM_RP_FLD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_RP_FLD_INTERVAL].name,
            p_pim_sm_rp->interval);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pim_sm_rp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pim_sm_rp_master_t *p_master = _g_p_tbl_pim_sm_rp_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pim_sm_rp_t *p_db_pim_sm_rp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_list, p_db_pim_sm_rp, listnode, next)
    {
        rc |= fn(p_db_pim_sm_rp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pim_sm_rp_master_t*
tbl_pim_sm_rp_get_master()
{
    return _g_p_tbl_pim_sm_rp_master;
}

tbl_pim_sm_rp_master_t*
tbl_pim_sm_rp_init_pim_sm_rp()
{
    _g_p_tbl_pim_sm_rp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pim_sm_rp_master_t));
    _g_p_tbl_pim_sm_rp_master->pim_sm_rp_hash = ctclib_hash_create(_tbl_pim_sm_rp_hash_make, _tbl_pim_sm_rp_hash_cmp);
    _g_p_tbl_pim_sm_rp_master->pim_sm_rp_list = ctclib_slist_create(tbl_pim_sm_rp_cmp, NULL);
    return _g_p_tbl_pim_sm_rp_master;
}


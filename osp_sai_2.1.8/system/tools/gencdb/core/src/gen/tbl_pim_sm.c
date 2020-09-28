
#include "proto.h"
#include "gen/tbl_pim_sm_define.h"
#include "gen/tbl_pim_sm.h"
#include "cdb_data_cmp.h"

tbl_pim_sm_master_t *_g_p_tbl_pim_sm_master = NULL;

static uint32
_tbl_pim_sm_hash_make(tbl_pim_sm_t *p_pim_sm)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pim_sm->key;
    for (index = 0; index < sizeof(p_pim_sm->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pim_sm_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pim_sm_t *p_pim_sm1 = (tbl_pim_sm_t*)p_arg1;
    tbl_pim_sm_t *p_pim_sm2 = (tbl_pim_sm_t*)p_arg2;

    if (0 == sal_memcmp(&p_pim_sm1->key, &p_pim_sm2->key, sizeof(tbl_pim_sm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pim_sm_add_pim_sm_sync(tbl_pim_sm_t *p_pim_sm, uint32 sync)
{
    tbl_pim_sm_master_t *p_master = _g_p_tbl_pim_sm_master;
    tbl_pim_sm_t *p_db_pim_sm = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pim_sm_get_pim_sm(&p_pim_sm->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pim_sm = XCALLOC(MEM_TBL_PIM_SM, sizeof(tbl_pim_sm_t));
    if (NULL == p_db_pim_sm)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pim_sm, p_pim_sm, sizeof(tbl_pim_sm_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pim_sm_hash, (void*)p_db_pim_sm, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pim_sm_list, p_db_pim_sm);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PIM_SM, p_db_pim_sm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PIM_SM, p_db_pim_sm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_del_pim_sm_sync(tbl_pim_sm_key_t *p_pim_sm_key, uint32 sync)
{
    tbl_pim_sm_master_t *p_master = _g_p_tbl_pim_sm_master;
    tbl_pim_sm_t *p_db_pim_sm = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm = tbl_pim_sm_get_pim_sm(p_pim_sm_key);
    if (NULL == p_db_pim_sm)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PIM_SM, p_db_pim_sm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PIM_SM, p_db_pim_sm);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pim_sm_hash, (void*)p_db_pim_sm);
    ctclib_slistnode_delete(p_master->pim_sm_list, p_db_pim_sm);

    /* 4. free db entry */
    XFREE(MEM_TBL_PIM_SM, p_db_pim_sm);

    return PM_E_NONE;
}

int32
tbl_pim_sm_set_pim_sm_field_sync(tbl_pim_sm_t *p_pim_sm, tbl_pim_sm_field_id_t field_id, uint32 sync)
{
    tbl_pim_sm_t *p_db_pim_sm = NULL;

    /* 1. lookup entry exist */
    p_db_pim_sm = tbl_pim_sm_get_pim_sm(&p_pim_sm->key);
    if (NULL == p_db_pim_sm)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PIM_SM_FLD_RP_ADDR:
        sal_memcpy(&p_db_pim_sm->rp_addr, &p_pim_sm->rp_addr, sizeof(p_pim_sm->rp_addr));
        break;

    case TBL_PIM_SM_FLD_HELLO_TIME:
        p_db_pim_sm->hello_time = p_pim_sm->hello_time;
        break;

    case TBL_PIM_SM_FLD_SPT_INFINITY:
        p_db_pim_sm->spt_infinity = p_pim_sm->spt_infinity;
        break;

    case TBL_PIM_SM_FLD_JP_TIMER:
        p_db_pim_sm->jp_timer = p_pim_sm->jp_timer;
        break;

    case TBL_PIM_SM_FLD_MAX:
        sal_memcpy(p_db_pim_sm, p_pim_sm, sizeof(tbl_pim_sm_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PIM_SM, field_id, p_db_pim_sm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PIM_SM, field_id, p_db_pim_sm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_add_pim_sm(tbl_pim_sm_t *p_pim_sm)
{
    return tbl_pim_sm_add_pim_sm_sync(p_pim_sm, TRUE);
}

int32
tbl_pim_sm_del_pim_sm(tbl_pim_sm_key_t *p_pim_sm_key)
{
    return tbl_pim_sm_del_pim_sm_sync(p_pim_sm_key, TRUE);
}

int32
tbl_pim_sm_set_pim_sm_field(tbl_pim_sm_t *p_pim_sm, tbl_pim_sm_field_id_t field_id)
{
    return tbl_pim_sm_set_pim_sm_field_sync(p_pim_sm, field_id, TRUE);
}

tbl_pim_sm_t*
tbl_pim_sm_get_pim_sm(tbl_pim_sm_key_t *p_pim_sm_key)
{
    tbl_pim_sm_master_t *p_master = _g_p_tbl_pim_sm_master;
    tbl_pim_sm_t lkp;

    sal_memcpy(&lkp.key, p_pim_sm_key, sizeof(tbl_pim_sm_key_t));
    return ctclib_hash_lookup(p_master->pim_sm_hash, &lkp);
}

char*
tbl_pim_sm_key_val2str(tbl_pim_sm_t *p_pim_sm, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_pim_sm->key.id);
    return str;
}

int32
tbl_pim_sm_key_str2val(char *str, tbl_pim_sm_t *p_pim_sm)
{
    int32 ret = 0;

    p_pim_sm->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_pim_sm_key_name_dump(tbl_pim_sm_t *p_pim_sm, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_pim_sm->key.id);
    return str;
}

char*
tbl_pim_sm_key_value_dump(tbl_pim_sm_t *p_pim_sm, char *str)
{
    sal_sprintf(str, "%u", p_pim_sm->key.id);
    return str;
}

char*
tbl_pim_sm_field_name_dump(tbl_pim_sm_t *p_pim_sm, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PIM_SM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pim_sm_field_value_dump(tbl_pim_sm_t *p_pim_sm, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PIM_SM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm->key.id);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_RP_ADDR, field_id))
    {
        cdb_pim_static_rp_addr_val2str(str, MAX_CMD_STR_LEN, &p_pim_sm->rp_addr);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_HELLO_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm->hello_time);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_SPT_INFINITY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm->spt_infinity);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_JP_TIMER, field_id))
    {
        sal_sprintf(str, "%u", p_pim_sm->jp_timer);
    }
    return str;
}

char**
tbl_pim_sm_table_dump(tbl_pim_sm_t *p_pim_sm, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pim_sm_key_name_dump(p_pim_sm, buf));
    for(i=1; i<TBL_PIM_SM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_field_name_dump(p_pim_sm, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pim_sm_field_value_dump(p_pim_sm, i, buf));
    }
    return str;
}

int32
tbl_pim_sm_field_value_parser(char *str, int32 field_id, tbl_pim_sm_t *p_pim_sm)
{
    if (FLD_MATCH(TBL_PIM_SM_FLD_KEY, field_id))
    {
        int32 ret;
        p_pim_sm->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_RP_ADDR, field_id))
    {
        cdb_pim_static_rp_addr_str2val(str, &p_pim_sm->rp_addr);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_HELLO_TIME, field_id))
    {
        int32 ret;
        p_pim_sm->hello_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_SPT_INFINITY, field_id))
    {
        int32 ret;
        p_pim_sm->spt_infinity = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_JP_TIMER, field_id))
    {
        int32 ret;
        p_pim_sm->jp_timer = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_pim_sm_table_parser(char** array, char* key_value,tbl_pim_sm_t *p_pim_sm)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pim_sm_key_str2val(key_value, p_pim_sm));

    for(i=1; i<TBL_PIM_SM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pim_sm_field_value_parser( array[j++], i, p_pim_sm));
    }

    return PM_E_NONE;
}

int32
tbl_pim_sm_dump_one(tbl_pim_sm_t *p_pim_sm, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_SM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PIM_SM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_FLD_KEY].name,
            p_pim_sm->key.id);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_RP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PIM_SM_FLD_RP_ADDR].name, 
            cdb_pim_static_rp_addr_val2str(buf, MAX_CMD_STR_LEN, &p_pim_sm->rp_addr));
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_HELLO_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_FLD_HELLO_TIME].name,
            p_pim_sm->hello_time);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_SPT_INFINITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_FLD_SPT_INFINITY].name,
            p_pim_sm->spt_infinity);
    }
    if (FLD_MATCH(TBL_PIM_SM_FLD_JP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_SM_FLD_JP_TIMER].name,
            p_pim_sm->jp_timer);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pim_sm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pim_sm_master_t *p_master = _g_p_tbl_pim_sm_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pim_sm_t *p_db_pim_sm = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_list, p_db_pim_sm, listnode, next)
    {
        rc |= fn(p_db_pim_sm, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pim_sm_master_t*
tbl_pim_sm_get_master()
{
    return _g_p_tbl_pim_sm_master;
}

tbl_pim_sm_master_t*
tbl_pim_sm_init_pim_sm()
{
    _g_p_tbl_pim_sm_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pim_sm_master_t));
    _g_p_tbl_pim_sm_master->pim_sm_hash = ctclib_hash_create(_tbl_pim_sm_hash_make, _tbl_pim_sm_hash_cmp);
    _g_p_tbl_pim_sm_master->pim_sm_list = ctclib_slist_create(tbl_pim_sm_cmp, NULL);
    return _g_p_tbl_pim_sm_master;
}


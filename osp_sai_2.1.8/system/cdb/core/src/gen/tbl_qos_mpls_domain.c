
#include "proto.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"
#include "cdb_data_cmp.h"

tbl_qos_mpls_domain_master_t *_g_p_tbl_qos_mpls_domain_master = NULL;

static uint32
_tbl_qos_mpls_domain_hash_make(tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_qos_mpls_domain->key;
    for (index = 0; index < sizeof(p_qos_mpls_domain->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_qos_mpls_domain_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_qos_mpls_domain_t *p_qos_mpls_domain1 = (tbl_qos_mpls_domain_t*)p_arg1;
    tbl_qos_mpls_domain_t *p_qos_mpls_domain2 = (tbl_qos_mpls_domain_t*)p_arg2;

    if (0 == sal_memcmp(&p_qos_mpls_domain1->key, &p_qos_mpls_domain2->key, sizeof(tbl_qos_mpls_domain_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_qos_mpls_domain_add_qos_mpls_domain_sync(tbl_qos_mpls_domain_t *p_qos_mpls_domain, uint32 sync)
{
    tbl_qos_mpls_domain_master_t *p_master = _g_p_tbl_qos_mpls_domain_master;
    tbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_mpls_domain_get_qos_mpls_domain(&p_qos_mpls_domain->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_mpls_domain = XCALLOC(MEM_TBL_QOS_MPLS_DOMAIN, sizeof(tbl_qos_mpls_domain_t));
    if (NULL == p_db_qos_mpls_domain)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_mpls_domain, p_qos_mpls_domain, sizeof(tbl_qos_mpls_domain_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->qos_mpls_domain_hash, (void*)p_db_qos_mpls_domain, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->qos_mpls_domain_list, p_db_qos_mpls_domain);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_del_qos_mpls_domain_sync(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key, uint32 sync)
{
    tbl_qos_mpls_domain_master_t *p_master = _g_p_tbl_qos_mpls_domain_master;
    tbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_domain = tbl_qos_mpls_domain_get_qos_mpls_domain(p_qos_mpls_domain_key);
    if (NULL == p_db_qos_mpls_domain)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->qos_mpls_domain_hash, (void*)p_db_qos_mpls_domain);
    ctclib_slistnode_delete(p_master->qos_mpls_domain_list, p_db_qos_mpls_domain);

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);

    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_qos_mpls_domain_field_id_t field_id, uint32 sync)
{
    tbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_domain = tbl_qos_mpls_domain_get_qos_mpls_domain(&p_qos_mpls_domain->key);
    if (NULL == p_db_qos_mpls_domain)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN:
        p_db_qos_mpls_domain->exp_domain = p_qos_mpls_domain->exp_domain;
        break;

    case TBL_QOS_MPLS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_qos_mpls_domain, p_qos_mpls_domain, sizeof(tbl_qos_mpls_domain_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_MPLS_DOMAIN, field_id, p_db_qos_mpls_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_MPLS_DOMAIN, field_id, p_db_qos_mpls_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_add_qos_mpls_domain(tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    return tbl_qos_mpls_domain_add_qos_mpls_domain_sync(p_qos_mpls_domain, TRUE);
}

int32
tbl_qos_mpls_domain_del_qos_mpls_domain(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key)
{
    return tbl_qos_mpls_domain_del_qos_mpls_domain_sync(p_qos_mpls_domain_key, TRUE);
}

int32
tbl_qos_mpls_domain_set_qos_mpls_domain_field(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_qos_mpls_domain_field_id_t field_id)
{
    return tbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(p_qos_mpls_domain, field_id, TRUE);
}

tbl_qos_mpls_domain_t*
tbl_qos_mpls_domain_get_qos_mpls_domain(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key)
{
    tbl_qos_mpls_domain_master_t *p_master = _g_p_tbl_qos_mpls_domain_master;
    tbl_qos_mpls_domain_t lkp;

    sal_memcpy(&lkp.key, p_qos_mpls_domain_key, sizeof(tbl_qos_mpls_domain_key_t));
    return ctclib_hash_lookup(p_master->qos_mpls_domain_hash, &lkp);
}

char*
tbl_qos_mpls_domain_key_val2str(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_mpls_domain->key.mpls_label);
    return str;
}

int32
tbl_qos_mpls_domain_key_str2val(char *str, tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    int32 ret = 0;

    p_qos_mpls_domain->key.mpls_label = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_qos_mpls_domain_key_name_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_MPLS_DOMAIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_qos_mpls_domain->key.mpls_label);
    return str;
}

char*
tbl_qos_mpls_domain_key_value_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str)
{
    sal_sprintf(str, "%u", p_qos_mpls_domain->key.mpls_label);
    return str;
}

char*
tbl_qos_mpls_domain_field_name_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_MPLS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_MPLS_DOMAIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_mpls_domain_field_value_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_qos_mpls_domain->key.mpls_label);
    }
    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_qos_mpls_domain->exp_domain);
    }
    return str;
}

char**
tbl_qos_mpls_domain_table_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_mpls_domain_key_name_dump(p_qos_mpls_domain, buf));
    for(i=1; i<TBL_QOS_MPLS_DOMAIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_mpls_domain_field_name_dump(p_qos_mpls_domain, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_mpls_domain_field_value_dump(p_qos_mpls_domain, i, buf));
    }
    return str;
}

int32
tbl_qos_mpls_domain_field_value_parser(char *str, int32 field_id, tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_KEY, field_id))
    {
        int32 ret;
        p_qos_mpls_domain->key.mpls_label = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN, field_id))
    {
        int32 ret;
        p_qos_mpls_domain->exp_domain = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_table_parser(char** array, char* key_value,tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_MPLS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_mpls_domain_key_str2val(key_value, p_qos_mpls_domain));

    for(i=1; i<TBL_QOS_MPLS_DOMAIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_mpls_domain_field_value_parser( array[j++], i, p_qos_mpls_domain));
    }

    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_dump_one(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_MPLS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_MPLS_DOMAIN_FLD_KEY].name,
            p_qos_mpls_domain->key.mpls_label);
    }
    if (FLD_MATCH(TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN].name,
            p_qos_mpls_domain->exp_domain);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_mpls_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_mpls_domain_master_t *p_master = _g_p_tbl_qos_mpls_domain_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->qos_mpls_domain_list, p_db_qos_mpls_domain, listnode, next)
    {
        rc |= fn(p_db_qos_mpls_domain, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_qos_mpls_domain_master_t*
tbl_qos_mpls_domain_get_master()
{
    return _g_p_tbl_qos_mpls_domain_master;
}

tbl_qos_mpls_domain_master_t*
tbl_qos_mpls_domain_init_qos_mpls_domain()
{
    _g_p_tbl_qos_mpls_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_mpls_domain_master_t));
    _g_p_tbl_qos_mpls_domain_master->qos_mpls_domain_hash = ctclib_hash_create(_tbl_qos_mpls_domain_hash_make, _tbl_qos_mpls_domain_hash_cmp);
    _g_p_tbl_qos_mpls_domain_master->qos_mpls_domain_list = ctclib_slist_create(tbl_qos_mpls_domain_cmp, NULL);
    return _g_p_tbl_qos_mpls_domain_master;
}


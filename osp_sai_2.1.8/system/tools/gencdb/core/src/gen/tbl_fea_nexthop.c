
#include "proto.h"
#include "gen/tbl_fea_nexthop_define.h"
#include "gen/tbl_fea_nexthop.h"
#include "cdb_data_cmp.h"

tbl_fea_nexthop_master_t *_g_p_tbl_fea_nexthop_master = NULL;

static uint32
_tbl_fea_nexthop_hash_make(tbl_fea_nexthop_t *p_nh)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nh->key;
    for (index = 0; index < sizeof(p_nh->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_nexthop_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_nexthop_t *p_nh1 = (tbl_fea_nexthop_t*)p_arg1;
    tbl_fea_nexthop_t *p_nh2 = (tbl_fea_nexthop_t*)p_arg2;

    if (0 == sal_memcmp(&p_nh1->key, &p_nh2->key, sizeof(tbl_fea_nexthop_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_nexthop_add_fea_nexthop_sync(tbl_fea_nexthop_t *p_nh, uint32 sync)
{
    tbl_fea_nexthop_master_t *p_master = _g_p_tbl_fea_nexthop_master;
    tbl_fea_nexthop_t *p_db_nh = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_nexthop_get_fea_nexthop(&p_nh->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nh = XCALLOC(MEM_TBL_FEA_NEXTHOP, sizeof(tbl_fea_nexthop_t));
    if (NULL == p_db_nh)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nh, p_nh, sizeof(tbl_fea_nexthop_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nh_hash, (void*)p_db_nh, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_NEXTHOP, p_db_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_NEXTHOP, p_db_nh);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_nexthop_del_fea_nexthop_sync(tbl_fea_nexthop_key_t *p_nh_key, uint32 sync)
{
    tbl_fea_nexthop_master_t *p_master = _g_p_tbl_fea_nexthop_master;
    tbl_fea_nexthop_t *p_db_nh = NULL;

    /* 1. lookup entry exist */
    p_db_nh = tbl_fea_nexthop_get_fea_nexthop(p_nh_key);
    if (NULL == p_db_nh)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_NEXTHOP, p_db_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_NEXTHOP, p_db_nh);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nh_hash, (void*)p_db_nh);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_NEXTHOP, p_db_nh);

    return PM_E_NONE;
}

int32
tbl_fea_nexthop_set_fea_nexthop_field_sync(tbl_fea_nexthop_t *p_nh, tbl_fea_nexthop_field_id_t field_id, uint32 sync)
{
    tbl_fea_nexthop_t *p_db_nh = NULL;

    /* 1. lookup entry exist */
    p_db_nh = tbl_fea_nexthop_get_fea_nexthop(&p_nh->key);
    if (NULL == p_db_nh)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_NEXTHOP_FLD_NHID:
        p_db_nh->nhid = p_nh->nhid;
        break;

    case TBL_FEA_NEXTHOP_FLD_MAX:
        sal_memcpy(p_db_nh, p_nh, sizeof(tbl_fea_nexthop_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_NEXTHOP, field_id, p_db_nh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_NEXTHOP, field_id, p_db_nh);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_nexthop_add_fea_nexthop(tbl_fea_nexthop_t *p_nh)
{
    return tbl_fea_nexthop_add_fea_nexthop_sync(p_nh, TRUE);
}

int32
tbl_fea_nexthop_del_fea_nexthop(tbl_fea_nexthop_key_t *p_nh_key)
{
    return tbl_fea_nexthop_del_fea_nexthop_sync(p_nh_key, TRUE);
}

int32
tbl_fea_nexthop_set_fea_nexthop_field(tbl_fea_nexthop_t *p_nh, tbl_fea_nexthop_field_id_t field_id)
{
    return tbl_fea_nexthop_set_fea_nexthop_field_sync(p_nh, field_id, TRUE);
}

tbl_fea_nexthop_t*
tbl_fea_nexthop_get_fea_nexthop(tbl_fea_nexthop_key_t *p_nh_key)
{
    tbl_fea_nexthop_master_t *p_master = _g_p_tbl_fea_nexthop_master;
    tbl_fea_nexthop_t lkp;

    sal_memcpy(&lkp.key, p_nh_key, sizeof(tbl_fea_nexthop_key_t));
    return ctclib_hash_lookup(p_master->nh_hash, &lkp);
}

char*
tbl_fea_nexthop_key_val2str(tbl_fea_nexthop_t *p_nh, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nexthop_key_val2str(buf, MAX_CMD_STR_LEN, &p_nh->key));
    return str;
}

int32
tbl_fea_nexthop_key_str2val(char *str, tbl_fea_nexthop_t *p_nh)
{
    int32 ret = 0;

    ret = cdb_nexthop_key_str2val(str, &p_nh->key);
    return ret;
}

char*
tbl_fea_nexthop_key_name_dump(tbl_fea_nexthop_t *p_nh, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_NEXTHOP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nexthop_key_val2str(buf, MAX_CMD_STR_LEN, &p_nh->key));
    return str;
}

char*
tbl_fea_nexthop_key_value_dump(tbl_fea_nexthop_t *p_nh, char *str)
{

    cdb_nexthop_key_val2str(str, MAX_CMD_STR_LEN, &p_nh->key);
    return str;
}

char*
tbl_fea_nexthop_field_name_dump(tbl_fea_nexthop_t *p_nh, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_NEXTHOP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_NEXTHOP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_nexthop_field_value_dump(tbl_fea_nexthop_t *p_nh, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_KEY, field_id))
    {
        cdb_nexthop_key_val2str(str, MAX_CMD_STR_LEN, &p_nh->key);
    }
    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_NHID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nh->nhid);
    }
    return str;
}

char**
tbl_fea_nexthop_table_dump(tbl_fea_nexthop_t *p_nh, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_nexthop_key_name_dump(p_nh, buf));
    for(i=1; i<TBL_FEA_NEXTHOP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_nexthop_field_name_dump(p_nh, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_nexthop_field_value_dump(p_nh, i, buf));
    }
    return str;
}

int32
tbl_fea_nexthop_field_value_parser(char *str, int32 field_id, tbl_fea_nexthop_t *p_nh)
{
    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_KEY, field_id))
    {
        cdb_nexthop_key_str2val(str, &p_nh->key);
    }
    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_NHID, field_id))
    {
        int32 ret;
        p_nh->nhid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_nexthop_table_parser(char** array, char* key_value,tbl_fea_nexthop_t *p_nh)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_NEXTHOP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_nexthop_key_str2val(key_value, p_nh));

    for(i=1; i<TBL_FEA_NEXTHOP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_nexthop_field_value_parser( array[j++], i, p_nh));
    }

    return PM_E_NONE;
}

int32
tbl_fea_nexthop_dump_one(tbl_fea_nexthop_t *p_nh, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_NEXTHOP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_NEXTHOP_FLD_KEY].name, 
            cdb_nexthop_key_val2str(buf, MAX_CMD_STR_LEN, &p_nh->key));
    }
    if (FLD_MATCH(TBL_FEA_NEXTHOP_FLD_NHID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_NEXTHOP_FLD_NHID].name,
            p_nh->nhid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_nexthop_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_nexthop_master_t *p_master = _g_p_tbl_fea_nexthop_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->nh_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fea_nexthop_master_t*
tbl_fea_nexthop_get_master()
{
    return _g_p_tbl_fea_nexthop_master;
}

tbl_fea_nexthop_master_t*
tbl_fea_nexthop_init_fea_nexthop()
{
    _g_p_tbl_fea_nexthop_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_nexthop_master_t));
    _g_p_tbl_fea_nexthop_master->nh_hash = ctclib_hash_create(_tbl_fea_nexthop_hash_make, _tbl_fea_nexthop_hash_cmp);
    return _g_p_tbl_fea_nexthop_master;
}


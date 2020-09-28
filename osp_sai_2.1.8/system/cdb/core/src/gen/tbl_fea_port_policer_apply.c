
#include "proto.h"
#include "gen/tbl_fea_port_policer_apply_define.h"
#include "gen/tbl_fea_port_policer_apply.h"
#include "cdb_data_cmp.h"

tbl_fea_port_policer_apply_master_t *_g_p_tbl_fea_port_policer_apply_master = NULL;

static uint32
_tbl_fea_port_policer_apply_hash_make(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fea_port_policer_apply->key;
    for (index = 0; index < sizeof(p_fea_port_policer_apply->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_port_policer_apply_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_port_policer_apply_t *p_fea_port_policer_apply1 = (tbl_fea_port_policer_apply_t*)p_arg1;
    tbl_fea_port_policer_apply_t *p_fea_port_policer_apply2 = (tbl_fea_port_policer_apply_t*)p_arg2;

    if (0 == sal_memcmp(&p_fea_port_policer_apply1->key, &p_fea_port_policer_apply2->key, sizeof(tbl_fea_port_policer_apply_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_port_policer_apply_add_fea_port_policer_apply_sync(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, uint32 sync)
{
    tbl_fea_port_policer_apply_master_t *p_master = _g_p_tbl_fea_port_policer_apply_master;
    tbl_fea_port_policer_apply_t *p_db_fea_port_policer_apply = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_port_policer_apply_get_fea_port_policer_apply(&p_fea_port_policer_apply->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_port_policer_apply = XCALLOC(MEM_TBL_FEA_PORT_POLICER_APPLY, sizeof(tbl_fea_port_policer_apply_t));
    if (NULL == p_db_fea_port_policer_apply)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_port_policer_apply, p_fea_port_policer_apply, sizeof(tbl_fea_port_policer_apply_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->if_hash, (void*)p_db_fea_port_policer_apply, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_fea_port_policer_apply);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_PORT_POLICER_APPLY, p_db_fea_port_policer_apply);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_PORT_POLICER_APPLY, p_db_fea_port_policer_apply);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_del_fea_port_policer_apply_sync(tbl_fea_port_policer_apply_key_t *p_fea_port_policer_apply_key, uint32 sync)
{
    tbl_fea_port_policer_apply_master_t *p_master = _g_p_tbl_fea_port_policer_apply_master;
    tbl_fea_port_policer_apply_t *p_db_fea_port_policer_apply = NULL;

    /* 1. lookup entry exist */
    p_db_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply(p_fea_port_policer_apply_key);
    if (NULL == p_db_fea_port_policer_apply)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_PORT_POLICER_APPLY, p_db_fea_port_policer_apply);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_PORT_POLICER_APPLY, p_db_fea_port_policer_apply);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->if_hash, (void*)p_db_fea_port_policer_apply);
    ctclib_slistnode_delete(p_master->if_list, p_db_fea_port_policer_apply);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_PORT_POLICER_APPLY, p_db_fea_port_policer_apply);

    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_set_fea_port_policer_apply_field_sync(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, tbl_fea_port_policer_apply_field_id_t field_id, uint32 sync)
{
    tbl_fea_port_policer_apply_t *p_db_fea_port_policer_apply = NULL;

    /* 1. lookup entry exist */
    p_db_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply(&p_fea_port_policer_apply->key);
    if (NULL == p_db_fea_port_policer_apply)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID:
        p_db_fea_port_policer_apply->in_policer_id = p_fea_port_policer_apply->in_policer_id;
        break;

    case TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID:
        p_db_fea_port_policer_apply->out_policer_id = p_fea_port_policer_apply->out_policer_id;
        break;

    case TBL_FEA_PORT_POLICER_APPLY_FLD_MAX:
        sal_memcpy(p_db_fea_port_policer_apply, p_fea_port_policer_apply, sizeof(tbl_fea_port_policer_apply_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_PORT_POLICER_APPLY, field_id, p_db_fea_port_policer_apply);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_PORT_POLICER_APPLY, field_id, p_db_fea_port_policer_apply);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_add_fea_port_policer_apply(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply)
{
    return tbl_fea_port_policer_apply_add_fea_port_policer_apply_sync(p_fea_port_policer_apply, TRUE);
}

int32
tbl_fea_port_policer_apply_del_fea_port_policer_apply(tbl_fea_port_policer_apply_key_t *p_fea_port_policer_apply_key)
{
    return tbl_fea_port_policer_apply_del_fea_port_policer_apply_sync(p_fea_port_policer_apply_key, TRUE);
}

int32
tbl_fea_port_policer_apply_set_fea_port_policer_apply_field(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, tbl_fea_port_policer_apply_field_id_t field_id)
{
    return tbl_fea_port_policer_apply_set_fea_port_policer_apply_field_sync(p_fea_port_policer_apply, field_id, TRUE);
}

tbl_fea_port_policer_apply_t*
tbl_fea_port_policer_apply_get_fea_port_policer_apply(tbl_fea_port_policer_apply_key_t *p_fea_port_policer_apply_key)
{
    tbl_fea_port_policer_apply_master_t *p_master = _g_p_tbl_fea_port_policer_apply_master;
    tbl_fea_port_policer_apply_t lkp;

    sal_memcpy(&lkp.key, p_fea_port_policer_apply_key, sizeof(tbl_fea_port_policer_apply_key_t));
    return ctclib_hash_lookup(p_master->if_hash, &lkp);
}

char*
tbl_fea_port_policer_apply_key_val2str(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_fea_port_policer_apply->key.name);
    return str;
}

int32
tbl_fea_port_policer_apply_key_str2val(char *str, tbl_fea_port_policer_apply_t *p_fea_port_policer_apply)
{
    int32 ret = 0;

    sal_strcpy(p_fea_port_policer_apply->key.name, str);
    return ret;
}

char*
tbl_fea_port_policer_apply_key_name_dump(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_POLICER_APPLY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_fea_port_policer_apply->key.name);
    return str;
}

char*
tbl_fea_port_policer_apply_key_value_dump(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, char *str)
{
    sal_sprintf(str, "%s", p_fea_port_policer_apply->key.name);
    return str;
}

char*
tbl_fea_port_policer_apply_field_name_dump(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_POLICER_APPLY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_PORT_POLICER_APPLY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_port_policer_apply_field_value_dump(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_fea_port_policer_apply->key.name);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_port_policer_apply->in_policer_id);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_port_policer_apply->out_policer_id);
    }
    return str;
}

char**
tbl_fea_port_policer_apply_table_dump(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_port_policer_apply_key_name_dump(p_fea_port_policer_apply, buf));
    for(i=1; i<TBL_FEA_PORT_POLICER_APPLY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_port_policer_apply_field_name_dump(p_fea_port_policer_apply, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_port_policer_apply_field_value_dump(p_fea_port_policer_apply, i, buf));
    }
    return str;
}

int32
tbl_fea_port_policer_apply_field_value_parser(char *str, int32 field_id, tbl_fea_port_policer_apply_t *p_fea_port_policer_apply)
{
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_KEY, field_id))
    {
        sal_strcpy(p_fea_port_policer_apply->key.name, str);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID, field_id))
    {
        int32 ret;
        p_fea_port_policer_apply->in_policer_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID, field_id))
    {
        int32 ret;
        p_fea_port_policer_apply->out_policer_id = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_table_parser(char** array, char* key_value,tbl_fea_port_policer_apply_t *p_fea_port_policer_apply)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_POLICER_APPLY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_port_policer_apply_key_str2val(key_value, p_fea_port_policer_apply));

    for(i=1; i<TBL_FEA_PORT_POLICER_APPLY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_port_policer_apply_field_value_parser( array[j++], i, p_fea_port_policer_apply));
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_dump_one(tbl_fea_port_policer_apply_t *p_fea_port_policer_apply, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_POLICER_APPLY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_PORT_POLICER_APPLY_FLD_KEY].name,
            p_fea_port_policer_apply->key.name);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID].name,
            p_fea_port_policer_apply->in_policer_id);
    }
    if (FLD_MATCH(TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID].name,
            p_fea_port_policer_apply->out_policer_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_port_policer_apply_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_port_policer_apply_master_t *p_master = _g_p_tbl_fea_port_policer_apply_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fea_port_policer_apply_t *p_db_fea_port_policer_apply = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_fea_port_policer_apply, listnode, next)
    {
        rc |= fn(p_db_fea_port_policer_apply, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_port_policer_apply_master_t*
tbl_fea_port_policer_apply_get_master()
{
    return _g_p_tbl_fea_port_policer_apply_master;
}

tbl_fea_port_policer_apply_master_t*
tbl_fea_port_policer_apply_init_fea_port_policer_apply()
{
    _g_p_tbl_fea_port_policer_apply_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_port_policer_apply_master_t));
    _g_p_tbl_fea_port_policer_apply_master->if_hash = ctclib_hash_create(_tbl_fea_port_policer_apply_hash_make, _tbl_fea_port_policer_apply_hash_cmp);
    _g_p_tbl_fea_port_policer_apply_master->if_list = ctclib_slist_create(tbl_fea_port_policer_apply_cmp, NULL);
    return _g_p_tbl_fea_port_policer_apply_master;
}

tbl_fea_port_policer_apply_t*
tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(const char* ifname)
{
    tbl_fea_port_policer_apply_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_fea_port_policer_apply_get_fea_port_policer_apply(&key);
}

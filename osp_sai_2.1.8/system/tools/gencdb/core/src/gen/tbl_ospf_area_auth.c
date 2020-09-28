
#include "proto.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "cdb_data_cmp.h"

tbl_ospf_area_auth_master_t *_g_p_tbl_ospf_area_auth_master = NULL;

static uint32
_tbl_ospf_area_auth_hash_make(tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf_area_auth->key;
    for (index = 0; index < sizeof(p_ospf_area_auth->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf_area_auth_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf_area_auth_t *p_ospf_area_auth1 = (tbl_ospf_area_auth_t*)p_arg1;
    tbl_ospf_area_auth_t *p_ospf_area_auth2 = (tbl_ospf_area_auth_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf_area_auth1->key, &p_ospf_area_auth2->key, sizeof(tbl_ospf_area_auth_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf_area_auth_add_ospf_area_auth_sync(tbl_ospf_area_auth_t *p_ospf_area_auth, uint32 sync)
{
    tbl_ospf_area_auth_master_t *p_master = _g_p_tbl_ospf_area_auth_master;
    tbl_ospf_area_auth_t *p_db_ospf_area_auth = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf_area_auth_get_ospf_area_auth(&p_ospf_area_auth->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf_area_auth = XCALLOC(MEM_TBL_OSPF_AREA_AUTH, sizeof(tbl_ospf_area_auth_t));
    if (NULL == p_db_ospf_area_auth)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf_area_auth, p_ospf_area_auth, sizeof(tbl_ospf_area_auth_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf_area_auth_hash, (void*)p_db_ospf_area_auth, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf_area_auth_list, p_db_ospf_area_auth);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF_AREA_AUTH, p_db_ospf_area_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF_AREA_AUTH, p_db_ospf_area_auth);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_del_ospf_area_auth_sync(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key, uint32 sync)
{
    tbl_ospf_area_auth_master_t *p_master = _g_p_tbl_ospf_area_auth_master;
    tbl_ospf_area_auth_t *p_db_ospf_area_auth = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_area_auth = tbl_ospf_area_auth_get_ospf_area_auth(p_ospf_area_auth_key);
    if (NULL == p_db_ospf_area_auth)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF_AREA_AUTH, p_db_ospf_area_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF_AREA_AUTH, p_db_ospf_area_auth);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf_area_auth_hash, (void*)p_db_ospf_area_auth);
    ctclib_slistnode_delete(p_master->ospf_area_auth_list, p_db_ospf_area_auth);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF_AREA_AUTH, p_db_ospf_area_auth);

    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_set_ospf_area_auth_field_sync(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_ospf_area_auth_field_id_t field_id, uint32 sync)
{
    tbl_ospf_area_auth_t *p_db_ospf_area_auth = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_area_auth = tbl_ospf_area_auth_get_ospf_area_auth(&p_ospf_area_auth->key);
    if (NULL == p_db_ospf_area_auth)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE:
        p_db_ospf_area_auth->auth_type = p_ospf_area_auth->auth_type;
        break;

    case TBL_OSPF_AREA_AUTH_FLD_MAX:
        sal_memcpy(p_db_ospf_area_auth, p_ospf_area_auth, sizeof(tbl_ospf_area_auth_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF_AREA_AUTH, field_id, p_db_ospf_area_auth);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF_AREA_AUTH, field_id, p_db_ospf_area_auth);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_add_ospf_area_auth(tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    return tbl_ospf_area_auth_add_ospf_area_auth_sync(p_ospf_area_auth, TRUE);
}

int32
tbl_ospf_area_auth_del_ospf_area_auth(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key)
{
    return tbl_ospf_area_auth_del_ospf_area_auth_sync(p_ospf_area_auth_key, TRUE);
}

int32
tbl_ospf_area_auth_set_ospf_area_auth_field(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_ospf_area_auth_field_id_t field_id)
{
    return tbl_ospf_area_auth_set_ospf_area_auth_field_sync(p_ospf_area_auth, field_id, TRUE);
}

tbl_ospf_area_auth_t*
tbl_ospf_area_auth_get_ospf_area_auth(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key)
{
    tbl_ospf_area_auth_master_t *p_master = _g_p_tbl_ospf_area_auth_master;
    tbl_ospf_area_auth_t lkp;

    sal_memcpy(&lkp.key, p_ospf_area_auth_key, sizeof(tbl_ospf_area_auth_key_t));
    return ctclib_hash_lookup(p_master->ospf_area_auth_hash, &lkp);
}

char*
tbl_ospf_area_auth_key_val2str(tbl_ospf_area_auth_t *p_ospf_area_auth, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ospf_area_auth->key.areaid);
    return str;
}

int32
tbl_ospf_area_auth_key_str2val(char *str, tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    int32 ret = 0;

    p_ospf_area_auth->key.areaid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ospf_area_auth_key_name_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_AUTH);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ospf_area_auth->key.areaid);
    return str;
}

char*
tbl_ospf_area_auth_key_value_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char *str)
{
    sal_sprintf(str, "%u", p_ospf_area_auth->key.areaid);
    return str;
}

char*
tbl_ospf_area_auth_field_name_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_AUTH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF_AREA_AUTH_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf_area_auth_field_value_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_auth->key.areaid);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_auth->auth_type);
    }
    return str;
}

char**
tbl_ospf_area_auth_table_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf_area_auth_key_name_dump(p_ospf_area_auth, buf));
    for(i=1; i<TBL_OSPF_AREA_AUTH_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf_area_auth_field_name_dump(p_ospf_area_auth, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf_area_auth_field_value_dump(p_ospf_area_auth, i, buf));
    }
    return str;
}

int32
tbl_ospf_area_auth_field_value_parser(char *str, int32 field_id, tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_KEY, field_id))
    {
        int32 ret;
        p_ospf_area_auth->key.areaid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE, field_id))
    {
        int32 ret;
        p_ospf_area_auth->auth_type = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_table_parser(char** array, char* key_value,tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_AUTH);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf_area_auth_key_str2val(key_value, p_ospf_area_auth));

    for(i=1; i<TBL_OSPF_AREA_AUTH_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf_area_auth_field_value_parser( array[j++], i, p_ospf_area_auth));
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_dump_one(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_AUTH);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_AUTH_FLD_KEY].name,
            p_ospf_area_auth->key.areaid);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE].name,
            p_ospf_area_auth->auth_type);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf_area_auth_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf_area_auth_master_t *p_master = _g_p_tbl_ospf_area_auth_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf_area_auth_t *p_db_ospf_area_auth = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf_area_auth_list, p_db_ospf_area_auth, listnode, next)
    {
        rc |= fn(p_db_ospf_area_auth, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf_area_auth_master_t*
tbl_ospf_area_auth_get_master()
{
    return _g_p_tbl_ospf_area_auth_master;
}

tbl_ospf_area_auth_master_t*
tbl_ospf_area_auth_init_ospf_area_auth()
{
    _g_p_tbl_ospf_area_auth_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf_area_auth_master_t));
    _g_p_tbl_ospf_area_auth_master->ospf_area_auth_hash = ctclib_hash_create(_tbl_ospf_area_auth_hash_make, _tbl_ospf_area_auth_hash_cmp);
    _g_p_tbl_ospf_area_auth_master->ospf_area_auth_list = ctclib_slist_create(tbl_ospf_area_auth_cmp, NULL);
    return _g_p_tbl_ospf_area_auth_master;
}


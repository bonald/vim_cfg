
#include "proto.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt_define.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt.h"
#include "cdb_data_cmp.h"

tbl_openflow_tunnel_local_ip_cnt_master_t *_g_p_tbl_openflow_tunnel_local_ip_cnt_master = NULL;

static uint32
_tbl_openflow_tunnel_local_ip_cnt_hash_make(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_openflow_tunnel_local_ip->key;
    for (index = 0; index < sizeof(p_openflow_tunnel_local_ip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_openflow_tunnel_local_ip_cnt_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip1 = (tbl_openflow_tunnel_local_ip_cnt_t*)p_arg1;
    tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip2 = (tbl_openflow_tunnel_local_ip_cnt_t*)p_arg2;

    if (0 == sal_memcmp(&p_openflow_tunnel_local_ip1->key, &p_openflow_tunnel_local_ip2->key, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt_sync(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, uint32 sync)
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
    tbl_openflow_tunnel_local_ip_cnt_t *p_db_openflow_tunnel_local_ip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&p_openflow_tunnel_local_ip->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_openflow_tunnel_local_ip = XCALLOC(MEM_TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
    if (NULL == p_db_openflow_tunnel_local_ip)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_openflow_tunnel_local_ip, p_openflow_tunnel_local_ip, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->openflow_tunnel_local_ip_hash, (void*)p_db_openflow_tunnel_local_ip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, p_db_openflow_tunnel_local_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, p_db_openflow_tunnel_local_ip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt_sync(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key, uint32 sync)
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
    tbl_openflow_tunnel_local_ip_cnt_t *p_db_openflow_tunnel_local_ip = NULL;

    /* 1. lookup entry exist */
    p_db_openflow_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(p_openflow_tunnel_local_ip_key);
    if (NULL == p_db_openflow_tunnel_local_ip)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, p_db_openflow_tunnel_local_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, p_db_openflow_tunnel_local_ip);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->openflow_tunnel_local_ip_hash, (void*)p_db_openflow_tunnel_local_ip);

    /* 4. free db entry */
    XFREE(MEM_TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, p_db_openflow_tunnel_local_ip);

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field_sync(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_openflow_tunnel_local_ip_cnt_field_id_t field_id, uint32 sync)
{
    tbl_openflow_tunnel_local_ip_cnt_t *p_db_openflow_tunnel_local_ip = NULL;

    /* 1. lookup entry exist */
    p_db_openflow_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&p_openflow_tunnel_local_ip->key);
    if (NULL == p_db_openflow_tunnel_local_ip)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF:
        p_db_openflow_tunnel_local_ip->local_ip_ref = p_openflow_tunnel_local_ip->local_ip_ref;
        break;

    case TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_MAX:
        sal_memcpy(p_db_openflow_tunnel_local_ip, p_openflow_tunnel_local_ip, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, field_id, p_db_openflow_tunnel_local_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT, field_id, p_db_openflow_tunnel_local_ip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip)
{
    return tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt_sync(p_openflow_tunnel_local_ip, TRUE);
}

int32
tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key)
{
    return tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt_sync(p_openflow_tunnel_local_ip_key, TRUE);
}

int32
tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_openflow_tunnel_local_ip_cnt_field_id_t field_id)
{
    return tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field_sync(p_openflow_tunnel_local_ip, field_id, TRUE);
}

tbl_openflow_tunnel_local_ip_cnt_t*
tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key)
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
    tbl_openflow_tunnel_local_ip_cnt_t lkp;

    sal_memcpy(&lkp.key, p_openflow_tunnel_local_ip_key, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
    return ctclib_hash_lookup(p_master->openflow_tunnel_local_ip_hash, &lkp);
}

char*
tbl_openflow_tunnel_local_ip_cnt_key_val2str(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_openflow_tunnel_local_ip->key.local_ip);
    return str;
}

int32
tbl_openflow_tunnel_local_ip_cnt_key_str2val(char *str, tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip)
{
    int32 ret = 0;

    sal_strcpy(p_openflow_tunnel_local_ip->key.local_ip, str);
    return ret;
}

char*
tbl_openflow_tunnel_local_ip_cnt_key_name_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_openflow_tunnel_local_ip->key.local_ip);
    return str;
}

char*
tbl_openflow_tunnel_local_ip_cnt_key_value_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char *str)
{
    sal_sprintf(str, "%s", p_openflow_tunnel_local_ip->key.local_ip);
    return str;
}

char*
tbl_openflow_tunnel_local_ip_cnt_field_name_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_tunnel_local_ip_cnt_field_value_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_openflow_tunnel_local_ip->key.local_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF, field_id))
    {
        sal_sprintf(str, "%u", p_openflow_tunnel_local_ip->local_ip_ref);
    }
    return str;
}

char**
tbl_openflow_tunnel_local_ip_cnt_table_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_tunnel_local_ip_cnt_key_name_dump(p_openflow_tunnel_local_ip, buf));
    for(i=1; i<TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_tunnel_local_ip_cnt_field_name_dump(p_openflow_tunnel_local_ip, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_tunnel_local_ip_cnt_field_value_dump(p_openflow_tunnel_local_ip, i, buf));
    }
    return str;
}

int32
tbl_openflow_tunnel_local_ip_cnt_field_value_parser(char *str, int32 field_id, tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip)
{
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_KEY, field_id))
    {
        sal_strcpy(p_openflow_tunnel_local_ip->key.local_ip, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF, field_id))
    {
        int32 ret;
        p_openflow_tunnel_local_ip->local_ip_ref = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_table_parser(char** array, char* key_value,tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_tunnel_local_ip_cnt_key_str2val(key_value, p_openflow_tunnel_local_ip));

    for(i=1; i<TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_tunnel_local_ip_cnt_field_value_parser( array[j++], i, p_openflow_tunnel_local_ip));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_dump_one(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_KEY].name,
            p_openflow_tunnel_local_ip->key.local_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF].name,
            p_openflow_tunnel_local_ip->local_ip_ref);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_local_ip_cnt_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->openflow_tunnel_local_ip_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_openflow_tunnel_local_ip_cnt_master_t*
tbl_openflow_tunnel_local_ip_cnt_get_master()
{
    return _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
}

tbl_openflow_tunnel_local_ip_cnt_master_t*
tbl_openflow_tunnel_local_ip_cnt_init_openflow_tunnel_local_ip_cnt()
{
    _g_p_tbl_openflow_tunnel_local_ip_cnt_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_openflow_tunnel_local_ip_cnt_master_t));
    _g_p_tbl_openflow_tunnel_local_ip_cnt_master->openflow_tunnel_local_ip_hash = ctclib_hash_create(_tbl_openflow_tunnel_local_ip_cnt_hash_make, _tbl_openflow_tunnel_local_ip_cnt_hash_cmp);
    return _g_p_tbl_openflow_tunnel_local_ip_cnt_master;
}

int32
tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt_count()
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;

    return p_master->openflow_tunnel_local_ip_hash->count;
}



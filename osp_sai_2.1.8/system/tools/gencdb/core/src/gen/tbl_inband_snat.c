
#include "proto.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "cdb_data_cmp.h"

tbl_inband_snat_master_t *_g_p_tbl_inband_snat_master = NULL;

static uint32
_tbl_inband_snat_hash_make(tbl_inband_snat_t *p_inband_snat)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_inband_snat->key;
    for (index = 0; index < sizeof(p_inband_snat->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_inband_snat_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_inband_snat_t *p_inband_snat1 = (tbl_inband_snat_t*)p_arg1;
    tbl_inband_snat_t *p_inband_snat2 = (tbl_inband_snat_t*)p_arg2;

    if (0 == sal_memcmp(&p_inband_snat1->key, &p_inband_snat2->key, sizeof(tbl_inband_snat_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_inband_snat_add_inband_snat_sync(tbl_inband_snat_t *p_inband_snat, uint32 sync)
{
    tbl_inband_snat_master_t *p_master = _g_p_tbl_inband_snat_master;
    tbl_inband_snat_t *p_db_inband_snat = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_inband_snat_get_inband_snat(&p_inband_snat->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_inband_snat = XCALLOC(MEM_TBL_INBAND_SNAT, sizeof(tbl_inband_snat_t));
    if (NULL == p_db_inband_snat)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_inband_snat, p_inband_snat, sizeof(tbl_inband_snat_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->inband_snat_hash, (void*)p_db_inband_snat, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->inband_snat_list, p_db_inband_snat);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_INBAND_SNAT, p_db_inband_snat);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_INBAND_SNAT, p_db_inband_snat);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_inband_snat_del_inband_snat_sync(tbl_inband_snat_key_t *p_inband_snat_key, uint32 sync)
{
    tbl_inband_snat_master_t *p_master = _g_p_tbl_inband_snat_master;
    tbl_inband_snat_t *p_db_inband_snat = NULL;

    /* 1. lookup entry exist */
    p_db_inband_snat = tbl_inband_snat_get_inband_snat(p_inband_snat_key);
    if (NULL == p_db_inband_snat)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_INBAND_SNAT, p_db_inband_snat);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_INBAND_SNAT, p_db_inband_snat);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->inband_snat_hash, (void*)p_db_inband_snat);
    ctclib_slistnode_delete(p_master->inband_snat_list, p_db_inband_snat);

    /* 4. free db entry */
    XFREE(MEM_TBL_INBAND_SNAT, p_db_inband_snat);

    return PM_E_NONE;
}

int32
tbl_inband_snat_set_inband_snat_field_sync(tbl_inband_snat_t *p_inband_snat, tbl_inband_snat_field_id_t field_id, uint32 sync)
{
    tbl_inband_snat_t *p_db_inband_snat = NULL;

    /* 1. lookup entry exist */
    p_db_inband_snat = tbl_inband_snat_get_inband_snat(&p_inband_snat->key);
    if (NULL == p_db_inband_snat)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_INBAND_SNAT_FLD_IFNAME:
        sal_memcpy(p_db_inband_snat->ifname, p_inband_snat->ifname, sizeof(p_inband_snat->ifname));
        break;

    case TBL_INBAND_SNAT_FLD_IP:
        sal_memcpy(&p_db_inband_snat->ip, &p_inband_snat->ip, sizeof(p_inband_snat->ip));
        break;

    case TBL_INBAND_SNAT_FLD_TYPE:
        p_db_inband_snat->type = p_inband_snat->type;
        break;

    case TBL_INBAND_SNAT_FLD_NFT_HANDLE:
        p_db_inband_snat->nft_handle = p_inband_snat->nft_handle;
        break;

    case TBL_INBAND_SNAT_FLD_SNAT_TIMER:
        p_db_inband_snat->snat_timer = p_inband_snat->snat_timer;
        break;

    case TBL_INBAND_SNAT_FLD_MAX:
        sal_memcpy(p_db_inband_snat, p_inband_snat, sizeof(tbl_inband_snat_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_INBAND_SNAT, field_id, p_db_inband_snat);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_INBAND_SNAT, field_id, p_db_inband_snat);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_inband_snat_add_inband_snat(tbl_inband_snat_t *p_inband_snat)
{
    return tbl_inband_snat_add_inband_snat_sync(p_inband_snat, TRUE);
}

int32
tbl_inband_snat_del_inband_snat(tbl_inband_snat_key_t *p_inband_snat_key)
{
    return tbl_inband_snat_del_inband_snat_sync(p_inband_snat_key, TRUE);
}

int32
tbl_inband_snat_set_inband_snat_field(tbl_inband_snat_t *p_inband_snat, tbl_inband_snat_field_id_t field_id)
{
    return tbl_inband_snat_set_inband_snat_field_sync(p_inband_snat, field_id, TRUE);
}

tbl_inband_snat_t*
tbl_inband_snat_get_inband_snat(tbl_inband_snat_key_t *p_inband_snat_key)
{
    tbl_inband_snat_master_t *p_master = _g_p_tbl_inband_snat_master;
    tbl_inband_snat_t lkp;

    sal_memcpy(&lkp.key, p_inband_snat_key, sizeof(tbl_inband_snat_key_t));
    return ctclib_hash_lookup(p_master->inband_snat_hash, &lkp);
}

char*
tbl_inband_snat_key_val2str(tbl_inband_snat_t *p_inband_snat, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_inband_snat_key_val2str(buf, MAX_CMD_STR_LEN, &p_inband_snat->key));
    return str;
}

int32
tbl_inband_snat_key_str2val(char *str, tbl_inband_snat_t *p_inband_snat)
{
    int32 ret = 0;

    ret = cdb_inband_snat_key_str2val(str, &p_inband_snat->key);
    return ret;
}

char*
tbl_inband_snat_key_name_dump(tbl_inband_snat_t *p_inband_snat, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_INBAND_SNAT);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_inband_snat_key_val2str(buf, MAX_CMD_STR_LEN, &p_inband_snat->key));
    return str;
}

char*
tbl_inband_snat_key_value_dump(tbl_inband_snat_t *p_inband_snat, char *str)
{

    cdb_inband_snat_key_val2str(str, MAX_CMD_STR_LEN, &p_inband_snat->key);
    return str;
}

char*
tbl_inband_snat_field_name_dump(tbl_inband_snat_t *p_inband_snat, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_INBAND_SNAT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_INBAND_SNAT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_inband_snat_field_value_dump(tbl_inband_snat_t *p_inband_snat, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_KEY, field_id))
    {
        cdb_inband_snat_key_val2str(str, MAX_CMD_STR_LEN, &p_inband_snat->key);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_inband_snat->ifname);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IP, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_inband_snat->ip);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_inband_snat->type);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_NFT_HANDLE, field_id))
    {
        sal_sprintf(str, "%u", p_inband_snat->nft_handle);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_SNAT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_inband_snat->snat_timer);
    }
    return str;
}

char**
tbl_inband_snat_table_dump(tbl_inband_snat_t *p_inband_snat, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_inband_snat_key_name_dump(p_inband_snat, buf));
    for(i=1; i<TBL_INBAND_SNAT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_inband_snat_field_name_dump(p_inband_snat, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_inband_snat_field_value_dump(p_inband_snat, i, buf));
    }
    return str;
}

int32
tbl_inband_snat_field_value_parser(char *str, int32 field_id, tbl_inband_snat_t *p_inband_snat)
{
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_KEY, field_id))
    {
        cdb_inband_snat_key_str2val(str, &p_inband_snat->key);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_inband_snat->ifname, str);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IP, field_id))
    {
        cdb_addr_ipv4_str2val(&p_inband_snat->ip, str, 0);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_TYPE, field_id))
    {
        int32 ret;
        p_inband_snat->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_NFT_HANDLE, field_id))
    {
        int32 ret;
        p_inband_snat->nft_handle = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_SNAT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_inband_snat->snat_timer);
    }
    return PM_E_NONE;
}

int32
tbl_inband_snat_table_parser(char** array, char* key_value,tbl_inband_snat_t *p_inband_snat)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_INBAND_SNAT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_inband_snat_key_str2val(key_value, p_inband_snat));

    for(i=1; i<TBL_INBAND_SNAT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_inband_snat_field_value_parser( array[j++], i, p_inband_snat));
    }

    return PM_E_NONE;
}

int32
tbl_inband_snat_dump_one(tbl_inband_snat_t *p_inband_snat, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_INBAND_SNAT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INBAND_SNAT_FLD_KEY].name, 
            cdb_inband_snat_key_val2str(buf, MAX_CMD_STR_LEN, &p_inband_snat->key));
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INBAND_SNAT_FLD_IFNAME].name,
            p_inband_snat->ifname);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INBAND_SNAT_FLD_IP].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_inband_snat->ip));
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INBAND_SNAT_FLD_TYPE].name,
            p_inband_snat->type);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_NFT_HANDLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INBAND_SNAT_FLD_NFT_HANDLE].name,
            p_inband_snat->nft_handle);
    }
    if (FLD_MATCH(TBL_INBAND_SNAT_FLD_SNAT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INBAND_SNAT_FLD_SNAT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_inband_snat->snat_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_inband_snat_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_inband_snat_master_t *p_master = _g_p_tbl_inband_snat_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_inband_snat_t *p_db_inband_snat = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->inband_snat_list, p_db_inband_snat, listnode, next)
    {
        rc |= fn(p_db_inband_snat, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_inband_snat_master_t*
tbl_inband_snat_get_master()
{
    return _g_p_tbl_inband_snat_master;
}

tbl_inband_snat_master_t*
tbl_inband_snat_init_inband_snat()
{
    _g_p_tbl_inband_snat_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_inband_snat_master_t));
    _g_p_tbl_inband_snat_master->inband_snat_hash = ctclib_hash_create(_tbl_inband_snat_hash_make, _tbl_inband_snat_hash_cmp);
    _g_p_tbl_inband_snat_master->inband_snat_list = ctclib_slist_create(tbl_inband_snat_cmp, NULL);
    return _g_p_tbl_inband_snat_master;
}


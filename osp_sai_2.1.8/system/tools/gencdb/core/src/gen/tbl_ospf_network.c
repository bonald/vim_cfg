
#include "proto.h"
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
#include "cdb_data_cmp.h"

tbl_ospf_network_master_t *_g_p_tbl_ospf_network_master = NULL;

static uint32
_tbl_ospf_network_hash_make(tbl_ospf_network_t *p_ospf_network)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf_network->key;
    for (index = 0; index < sizeof(p_ospf_network->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf_network_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf_network_t *p_ospf_network1 = (tbl_ospf_network_t*)p_arg1;
    tbl_ospf_network_t *p_ospf_network2 = (tbl_ospf_network_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf_network1->key, &p_ospf_network2->key, sizeof(tbl_ospf_network_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf_network_add_ospf_network_sync(tbl_ospf_network_t *p_ospf_network, uint32 sync)
{
    tbl_ospf_network_master_t *p_master = _g_p_tbl_ospf_network_master;
    tbl_ospf_network_t *p_db_ospf_network = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf_network_get_ospf_network(&p_ospf_network->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf_network = XCALLOC(MEM_TBL_OSPF_NETWORK, sizeof(tbl_ospf_network_t));
    if (NULL == p_db_ospf_network)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf_network, p_ospf_network, sizeof(tbl_ospf_network_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf_network_hash, (void*)p_db_ospf_network, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf_network_list, p_db_ospf_network);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF_NETWORK, p_db_ospf_network);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF_NETWORK, p_db_ospf_network);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_network_del_ospf_network_sync(tbl_ospf_network_key_t *p_ospf_network_key, uint32 sync)
{
    tbl_ospf_network_master_t *p_master = _g_p_tbl_ospf_network_master;
    tbl_ospf_network_t *p_db_ospf_network = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_network = tbl_ospf_network_get_ospf_network(p_ospf_network_key);
    if (NULL == p_db_ospf_network)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF_NETWORK, p_db_ospf_network);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF_NETWORK, p_db_ospf_network);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf_network_hash, (void*)p_db_ospf_network);
    ctclib_slistnode_delete(p_master->ospf_network_list, p_db_ospf_network);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF_NETWORK, p_db_ospf_network);

    return PM_E_NONE;
}

int32
tbl_ospf_network_set_ospf_network_field_sync(tbl_ospf_network_t *p_ospf_network, tbl_ospf_network_field_id_t field_id, uint32 sync)
{
    tbl_ospf_network_t *p_db_ospf_network = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_network = tbl_ospf_network_get_ospf_network(&p_ospf_network->key);
    if (NULL == p_db_ospf_network)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF_NETWORK_FLD_AREA_ID:
        p_db_ospf_network->area_id = p_ospf_network->area_id;
        break;

    case TBL_OSPF_NETWORK_FLD_MAX:
        sal_memcpy(p_db_ospf_network, p_ospf_network, sizeof(tbl_ospf_network_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF_NETWORK, field_id, p_db_ospf_network);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF_NETWORK, field_id, p_db_ospf_network);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_network_add_ospf_network(tbl_ospf_network_t *p_ospf_network)
{
    return tbl_ospf_network_add_ospf_network_sync(p_ospf_network, TRUE);
}

int32
tbl_ospf_network_del_ospf_network(tbl_ospf_network_key_t *p_ospf_network_key)
{
    return tbl_ospf_network_del_ospf_network_sync(p_ospf_network_key, TRUE);
}

int32
tbl_ospf_network_set_ospf_network_field(tbl_ospf_network_t *p_ospf_network, tbl_ospf_network_field_id_t field_id)
{
    return tbl_ospf_network_set_ospf_network_field_sync(p_ospf_network, field_id, TRUE);
}

tbl_ospf_network_t*
tbl_ospf_network_get_ospf_network(tbl_ospf_network_key_t *p_ospf_network_key)
{
    tbl_ospf_network_master_t *p_master = _g_p_tbl_ospf_network_master;
    tbl_ospf_network_t lkp;

    sal_memcpy(&lkp.key, p_ospf_network_key, sizeof(tbl_ospf_network_key_t));
    return ctclib_hash_lookup(p_master->ospf_network_hash, &lkp);
}

char*
tbl_ospf_network_key_val2str(tbl_ospf_network_t *p_ospf_network, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_network->key));
    return str;
}

int32
tbl_ospf_network_key_str2val(char *str, tbl_ospf_network_t *p_ospf_network)
{
    int32 ret = 0;

    ret = cdb_prefix_str2val_wo_masklen(str, &p_ospf_network->key);
    return ret;
}

char*
tbl_ospf_network_key_name_dump(tbl_ospf_network_t *p_ospf_network, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_NETWORK);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_network->key));
    return str;
}

char*
tbl_ospf_network_key_value_dump(tbl_ospf_network_t *p_ospf_network, char *str)
{

    cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ospf_network->key);
    return str;
}

char*
tbl_ospf_network_field_name_dump(tbl_ospf_network_t *p_ospf_network, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_NETWORK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF_NETWORK_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf_network_field_value_dump(tbl_ospf_network_t *p_ospf_network, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_KEY, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ospf_network->key);
    }
    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_AREA_ID, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_network->area_id);
    }
    return str;
}

char**
tbl_ospf_network_table_dump(tbl_ospf_network_t *p_ospf_network, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf_network_key_name_dump(p_ospf_network, buf));
    for(i=1; i<TBL_OSPF_NETWORK_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf_network_field_name_dump(p_ospf_network, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf_network_field_value_dump(p_ospf_network, i, buf));
    }
    return str;
}

int32
tbl_ospf_network_field_value_parser(char *str, int32 field_id, tbl_ospf_network_t *p_ospf_network)
{
    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_KEY, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ospf_network->key);
    }
    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_AREA_ID, field_id))
    {
        int32 ret;
        p_ospf_network->area_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ospf_network_table_parser(char** array, char* key_value,tbl_ospf_network_t *p_ospf_network)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_NETWORK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf_network_key_str2val(key_value, p_ospf_network));

    for(i=1; i<TBL_OSPF_NETWORK_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf_network_field_value_parser( array[j++], i, p_ospf_network));
    }

    return PM_E_NONE;
}

int32
tbl_ospf_network_dump_one(tbl_ospf_network_t *p_ospf_network, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_NETWORK);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_NETWORK_FLD_KEY].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_network->key));
    }
    if (FLD_MATCH(TBL_OSPF_NETWORK_FLD_AREA_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_NETWORK_FLD_AREA_ID].name,
            p_ospf_network->area_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf_network_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf_network_master_t *p_master = _g_p_tbl_ospf_network_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf_network_t *p_db_ospf_network = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf_network_list, p_db_ospf_network, listnode, next)
    {
        rc |= fn(p_db_ospf_network, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf_network_master_t*
tbl_ospf_network_get_master()
{
    return _g_p_tbl_ospf_network_master;
}

tbl_ospf_network_master_t*
tbl_ospf_network_init_ospf_network()
{
    _g_p_tbl_ospf_network_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf_network_master_t));
    _g_p_tbl_ospf_network_master->ospf_network_hash = ctclib_hash_create(_tbl_ospf_network_hash_make, _tbl_ospf_network_hash_cmp);
    _g_p_tbl_ospf_network_master->ospf_network_list = ctclib_slist_create(tbl_ospf_network_cmp, NULL);
    return _g_p_tbl_ospf_network_master;
}


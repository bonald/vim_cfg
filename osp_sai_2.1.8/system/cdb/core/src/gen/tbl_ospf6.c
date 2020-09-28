
#include "proto.h"
#include "gen/tbl_ospf6_define.h"
#include "gen/tbl_ospf6.h"
#include "cdb_data_cmp.h"

tbl_ospf6_master_t *_g_p_tbl_ospf6_master = NULL;

static uint32
_tbl_ospf6_hash_make(tbl_ospf6_t *p_ospf6)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf6->key;
    for (index = 0; index < sizeof(p_ospf6->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf6_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf6_t *p_ospf61 = (tbl_ospf6_t*)p_arg1;
    tbl_ospf6_t *p_ospf62 = (tbl_ospf6_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf61->key, &p_ospf62->key, sizeof(tbl_ospf6_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf6_add_ospf6_sync(tbl_ospf6_t *p_ospf6, uint32 sync)
{
    tbl_ospf6_master_t *p_master = _g_p_tbl_ospf6_master;
    tbl_ospf6_t *p_db_ospf6 = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf6_get_ospf6(&p_ospf6->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf6 = XCALLOC(MEM_TBL_OSPF6, sizeof(tbl_ospf6_t));
    if (NULL == p_db_ospf6)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf6, p_ospf6, sizeof(tbl_ospf6_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf6_hash, (void*)p_db_ospf6, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf6_list, p_db_ospf6);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF6, p_db_ospf6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF6, p_db_ospf6);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_del_ospf6_sync(tbl_ospf6_key_t *p_ospf6_key, uint32 sync)
{
    tbl_ospf6_master_t *p_master = _g_p_tbl_ospf6_master;
    tbl_ospf6_t *p_db_ospf6 = NULL;

    /* 1. lookup entry exist */
    p_db_ospf6 = tbl_ospf6_get_ospf6(p_ospf6_key);
    if (NULL == p_db_ospf6)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF6, p_db_ospf6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF6, p_db_ospf6);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf6_hash, (void*)p_db_ospf6);
    ctclib_slistnode_delete(p_master->ospf6_list, p_db_ospf6);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF6, p_db_ospf6);

    return PM_E_NONE;
}

int32
tbl_ospf6_set_ospf6_field_sync(tbl_ospf6_t *p_ospf6, tbl_ospf6_field_id_t field_id, uint32 sync)
{
    tbl_ospf6_t *p_db_ospf6 = NULL;

    /* 1. lookup entry exist */
    p_db_ospf6 = tbl_ospf6_get_ospf6(&p_ospf6->key);
    if (NULL == p_db_ospf6)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF6_FLD_ROUTER_ID:
        sal_memcpy(&p_db_ospf6->router_id, &p_ospf6->router_id, sizeof(p_ospf6->router_id));
        break;

    case TBL_OSPF6_FLD_DEFAULT_ORIGINATE:
        p_db_ospf6->default_originate = p_ospf6->default_originate;
        break;

    case TBL_OSPF6_FLD_REDISTRIBUTE:
        p_db_ospf6->redistribute = p_ospf6->redistribute;
        break;

    case TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_CONNECTED_TIMER:
        p_db_ospf6->t_ospf6_redistribute_connected_timer = p_ospf6->t_ospf6_redistribute_connected_timer;
        break;

    case TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_STATIC_TIMER:
        p_db_ospf6->t_ospf6_redistribute_static_timer = p_ospf6->t_ospf6_redistribute_static_timer;
        break;

    case TBL_OSPF6_FLD_MAX:
        sal_memcpy(p_db_ospf6, p_ospf6, sizeof(tbl_ospf6_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF6, field_id, p_db_ospf6);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF6, field_id, p_db_ospf6);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_add_ospf6(tbl_ospf6_t *p_ospf6)
{
    return tbl_ospf6_add_ospf6_sync(p_ospf6, TRUE);
}

int32
tbl_ospf6_del_ospf6(tbl_ospf6_key_t *p_ospf6_key)
{
    return tbl_ospf6_del_ospf6_sync(p_ospf6_key, TRUE);
}

int32
tbl_ospf6_set_ospf6_field(tbl_ospf6_t *p_ospf6, tbl_ospf6_field_id_t field_id)
{
    return tbl_ospf6_set_ospf6_field_sync(p_ospf6, field_id, TRUE);
}

tbl_ospf6_t*
tbl_ospf6_get_ospf6(tbl_ospf6_key_t *p_ospf6_key)
{
    tbl_ospf6_master_t *p_master = _g_p_tbl_ospf6_master;
    tbl_ospf6_t lkp;

    sal_memcpy(&lkp.key, p_ospf6_key, sizeof(tbl_ospf6_key_t));
    return ctclib_hash_lookup(p_master->ospf6_hash, &lkp);
}

char*
tbl_ospf6_key_val2str(tbl_ospf6_t *p_ospf6, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ospf6->key.id);
    return str;
}

int32
tbl_ospf6_key_str2val(char *str, tbl_ospf6_t *p_ospf6)
{
    int32 ret = 0;

    p_ospf6->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ospf6_key_name_dump(tbl_ospf6_t *p_ospf6, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ospf6->key.id);
    return str;
}

char*
tbl_ospf6_key_value_dump(tbl_ospf6_t *p_ospf6, char *str)
{
    sal_sprintf(str, "%u", p_ospf6->key.id);
    return str;
}

char*
tbl_ospf6_field_name_dump(tbl_ospf6_t *p_ospf6, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF6_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf6_field_value_dump(tbl_ospf6_t *p_ospf6, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF6_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ospf6->key.id);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_ospf6->router_id);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_DEFAULT_ORIGINATE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf6->default_originate);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf6->redistribute);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_CONNECTED_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ospf6->t_ospf6_redistribute_connected_timer);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_STATIC_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ospf6->t_ospf6_redistribute_static_timer);
    }
    return str;
}

char**
tbl_ospf6_table_dump(tbl_ospf6_t *p_ospf6, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf6_key_name_dump(p_ospf6, buf));
    for(i=1; i<TBL_OSPF6_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf6_field_name_dump(p_ospf6, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf6_field_value_dump(p_ospf6, i, buf));
    }
    return str;
}

int32
tbl_ospf6_field_value_parser(char *str, int32 field_id, tbl_ospf6_t *p_ospf6)
{
    if (FLD_MATCH(TBL_OSPF6_FLD_KEY, field_id))
    {
        int32 ret;
        p_ospf6->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_str2val(&p_ospf6->router_id, str, 0);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_DEFAULT_ORIGINATE, field_id))
    {
        int32 ret;
        p_ospf6->default_originate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_REDISTRIBUTE, field_id))
    {
        int32 ret;
        p_ospf6->redistribute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_CONNECTED_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ospf6->t_ospf6_redistribute_connected_timer);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_STATIC_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ospf6->t_ospf6_redistribute_static_timer);
    }
    return PM_E_NONE;
}

int32
tbl_ospf6_table_parser(char** array, char* key_value,tbl_ospf6_t *p_ospf6)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf6_key_str2val(key_value, p_ospf6));

    for(i=1; i<TBL_OSPF6_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf6_field_value_parser( array[j++], i, p_ospf6));
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_dump_one(tbl_ospf6_t *p_ospf6, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OSPF6_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF6_FLD_KEY].name,
            p_ospf6->key.id);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_ROUTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF6_FLD_ROUTER_ID].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_ospf6->router_id));
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_DEFAULT_ORIGINATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF6_FLD_DEFAULT_ORIGINATE].name,
            p_ospf6->default_originate);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF6_FLD_REDISTRIBUTE].name,
            p_ospf6->redistribute);
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_CONNECTED_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_CONNECTED_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ospf6->t_ospf6_redistribute_connected_timer));
    }
    if (FLD_MATCH(TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_STATIC_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF6_FLD_T_OSPF6_REDISTRIBUTE_STATIC_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ospf6->t_ospf6_redistribute_static_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf6_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf6_master_t *p_master = _g_p_tbl_ospf6_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf6_t *p_db_ospf6 = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf6_list, p_db_ospf6, listnode, next)
    {
        rc |= fn(p_db_ospf6, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf6_master_t*
tbl_ospf6_get_master()
{
    return _g_p_tbl_ospf6_master;
}

tbl_ospf6_master_t*
tbl_ospf6_init_ospf6()
{
    _g_p_tbl_ospf6_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf6_master_t));
    _g_p_tbl_ospf6_master->ospf6_hash = ctclib_hash_create(_tbl_ospf6_hash_make, _tbl_ospf6_hash_cmp);
    _g_p_tbl_ospf6_master->ospf6_list = ctclib_slist_create(tbl_ospf6_cmp, NULL);
    return _g_p_tbl_ospf6_master;
}


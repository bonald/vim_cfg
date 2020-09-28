
#include "proto.h"
#include "gen/tbl_ospf_define.h"
#include "gen/tbl_ospf.h"
#include "cdb_data_cmp.h"

tbl_ospf_master_t *_g_p_tbl_ospf_master = NULL;

static uint32
_tbl_ospf_hash_make(tbl_ospf_t *p_ospf)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf->key;
    for (index = 0; index < sizeof(p_ospf->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf_t *p_ospf1 = (tbl_ospf_t*)p_arg1;
    tbl_ospf_t *p_ospf2 = (tbl_ospf_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf1->key, &p_ospf2->key, sizeof(tbl_ospf_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf_add_ospf_sync(tbl_ospf_t *p_ospf, uint32 sync)
{
    tbl_ospf_master_t *p_master = _g_p_tbl_ospf_master;
    tbl_ospf_t *p_db_ospf = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf_get_ospf(&p_ospf->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf = XCALLOC(MEM_TBL_OSPF, sizeof(tbl_ospf_t));
    if (NULL == p_db_ospf)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf, p_ospf, sizeof(tbl_ospf_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf_hash, (void*)p_db_ospf, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf_list, p_db_ospf);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF, p_db_ospf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF, p_db_ospf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_del_ospf_sync(tbl_ospf_key_t *p_ospf_key, uint32 sync)
{
    tbl_ospf_master_t *p_master = _g_p_tbl_ospf_master;
    tbl_ospf_t *p_db_ospf = NULL;

    /* 1. lookup entry exist */
    p_db_ospf = tbl_ospf_get_ospf(p_ospf_key);
    if (NULL == p_db_ospf)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF, p_db_ospf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF, p_db_ospf);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf_hash, (void*)p_db_ospf);
    ctclib_slistnode_delete(p_master->ospf_list, p_db_ospf);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF, p_db_ospf);

    return PM_E_NONE;
}

int32
tbl_ospf_set_ospf_field_sync(tbl_ospf_t *p_ospf, tbl_ospf_field_id_t field_id, uint32 sync)
{
    tbl_ospf_t *p_db_ospf = NULL;

    /* 1. lookup entry exist */
    p_db_ospf = tbl_ospf_get_ospf(&p_ospf->key);
    if (NULL == p_db_ospf)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF_FLD_ROUTER_ID:
        sal_memcpy(&p_db_ospf->router_id, &p_ospf->router_id, sizeof(p_ospf->router_id));
        break;

    case TBL_OSPF_FLD_DEFAULT_ORIGINATE:
        p_db_ospf->default_originate = p_ospf->default_originate;
        break;

    case TBL_OSPF_FLD_REDISTRIBUTE:
        p_db_ospf->redistribute = p_ospf->redistribute;
        break;

    case TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_CONNECTED_TIMER:
        p_db_ospf->t_ospf_redistribute_connected_timer = p_ospf->t_ospf_redistribute_connected_timer;
        break;

    case TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_STATIC_TIMER:
        p_db_ospf->t_ospf_redistribute_static_timer = p_ospf->t_ospf_redistribute_static_timer;
        break;

    case TBL_OSPF_FLD_MAX:
        sal_memcpy(p_db_ospf, p_ospf, sizeof(tbl_ospf_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF, field_id, p_db_ospf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF, field_id, p_db_ospf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_add_ospf(tbl_ospf_t *p_ospf)
{
    return tbl_ospf_add_ospf_sync(p_ospf, TRUE);
}

int32
tbl_ospf_del_ospf(tbl_ospf_key_t *p_ospf_key)
{
    return tbl_ospf_del_ospf_sync(p_ospf_key, TRUE);
}

int32
tbl_ospf_set_ospf_field(tbl_ospf_t *p_ospf, tbl_ospf_field_id_t field_id)
{
    return tbl_ospf_set_ospf_field_sync(p_ospf, field_id, TRUE);
}

tbl_ospf_t*
tbl_ospf_get_ospf(tbl_ospf_key_t *p_ospf_key)
{
    tbl_ospf_master_t *p_master = _g_p_tbl_ospf_master;
    tbl_ospf_t lkp;

    sal_memcpy(&lkp.key, p_ospf_key, sizeof(tbl_ospf_key_t));
    return ctclib_hash_lookup(p_master->ospf_hash, &lkp);
}

char*
tbl_ospf_key_val2str(tbl_ospf_t *p_ospf, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ospf->key.id);
    return str;
}

int32
tbl_ospf_key_str2val(char *str, tbl_ospf_t *p_ospf)
{
    int32 ret = 0;

    p_ospf->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ospf_key_name_dump(tbl_ospf_t *p_ospf, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ospf->key.id);
    return str;
}

char*
tbl_ospf_key_value_dump(tbl_ospf_t *p_ospf, char *str)
{
    sal_sprintf(str, "%u", p_ospf->key.id);
    return str;
}

char*
tbl_ospf_field_name_dump(tbl_ospf_t *p_ospf, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf_field_value_dump(tbl_ospf_t *p_ospf, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ospf->key.id);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_ospf->router_id);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_DEFAULT_ORIGINATE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf->default_originate);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf->redistribute);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_CONNECTED_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ospf->t_ospf_redistribute_connected_timer);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_STATIC_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ospf->t_ospf_redistribute_static_timer);
    }
    return str;
}

char**
tbl_ospf_table_dump(tbl_ospf_t *p_ospf, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf_key_name_dump(p_ospf, buf));
    for(i=1; i<TBL_OSPF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf_field_name_dump(p_ospf, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf_field_value_dump(p_ospf, i, buf));
    }
    return str;
}

int32
tbl_ospf_field_value_parser(char *str, int32 field_id, tbl_ospf_t *p_ospf)
{
    if (FLD_MATCH(TBL_OSPF_FLD_KEY, field_id))
    {
        int32 ret;
        p_ospf->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_str2val(&p_ospf->router_id, str, 0);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_DEFAULT_ORIGINATE, field_id))
    {
        int32 ret;
        p_ospf->default_originate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_REDISTRIBUTE, field_id))
    {
        int32 ret;
        p_ospf->redistribute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_CONNECTED_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ospf->t_ospf_redistribute_connected_timer);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_STATIC_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ospf->t_ospf_redistribute_static_timer);
    }
    return PM_E_NONE;
}

int32
tbl_ospf_table_parser(char** array, char* key_value,tbl_ospf_t *p_ospf)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf_key_str2val(key_value, p_ospf));

    for(i=1; i<TBL_OSPF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf_field_value_parser( array[j++], i, p_ospf));
    }

    return PM_E_NONE;
}

int32
tbl_ospf_dump_one(tbl_ospf_t *p_ospf, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OSPF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_FLD_KEY].name,
            p_ospf->key.id);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_ROUTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_FLD_ROUTER_ID].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_ospf->router_id));
    }
    if (FLD_MATCH(TBL_OSPF_FLD_DEFAULT_ORIGINATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_FLD_DEFAULT_ORIGINATE].name,
            p_ospf->default_originate);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_FLD_REDISTRIBUTE].name,
            p_ospf->redistribute);
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_CONNECTED_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_CONNECTED_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ospf->t_ospf_redistribute_connected_timer));
    }
    if (FLD_MATCH(TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_STATIC_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_FLD_T_OSPF_REDISTRIBUTE_STATIC_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ospf->t_ospf_redistribute_static_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf_master_t *p_master = _g_p_tbl_ospf_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf_t *p_db_ospf = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf_list, p_db_ospf, listnode, next)
    {
        rc |= fn(p_db_ospf, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf_master_t*
tbl_ospf_get_master()
{
    return _g_p_tbl_ospf_master;
}

tbl_ospf_master_t*
tbl_ospf_init_ospf()
{
    _g_p_tbl_ospf_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf_master_t));
    _g_p_tbl_ospf_master->ospf_hash = ctclib_hash_create(_tbl_ospf_hash_make, _tbl_ospf_hash_cmp);
    _g_p_tbl_ospf_master->ospf_list = ctclib_slist_create(tbl_ospf_cmp, NULL);
    return _g_p_tbl_ospf_master;
}


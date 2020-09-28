
#include "proto.h"
#include "gen/tbl_rip_define.h"
#include "gen/tbl_rip.h"
#include "cdb_data_cmp.h"

tbl_rip_master_t *_g_p_tbl_rip_master = NULL;

static uint32
_tbl_rip_hash_make(tbl_rip_t *p_rip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rip->key;
    for (index = 0; index < sizeof(p_rip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rip_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rip_t *p_rip1 = (tbl_rip_t*)p_arg1;
    tbl_rip_t *p_rip2 = (tbl_rip_t*)p_arg2;

    if (0 == sal_memcmp(&p_rip1->key, &p_rip2->key, sizeof(tbl_rip_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rip_add_rip_sync(tbl_rip_t *p_rip, uint32 sync)
{
    tbl_rip_master_t *p_master = _g_p_tbl_rip_master;
    tbl_rip_t *p_db_rip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rip_get_rip(&p_rip->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rip = XCALLOC(MEM_TBL_RIP, sizeof(tbl_rip_t));
    if (NULL == p_db_rip)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rip, p_rip, sizeof(tbl_rip_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rip_hash, (void*)p_db_rip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rip_list, p_db_rip);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RIP, p_db_rip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RIP, p_db_rip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rip_del_rip_sync(tbl_rip_key_t *p_rip_key, uint32 sync)
{
    tbl_rip_master_t *p_master = _g_p_tbl_rip_master;
    tbl_rip_t *p_db_rip = NULL;

    /* 1. lookup entry exist */
    p_db_rip = tbl_rip_get_rip(p_rip_key);
    if (NULL == p_db_rip)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RIP, p_db_rip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RIP, p_db_rip);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rip_hash, (void*)p_db_rip);
    ctclib_slistnode_delete(p_master->rip_list, p_db_rip);

    /* 4. free db entry */
    XFREE(MEM_TBL_RIP, p_db_rip);

    return PM_E_NONE;
}

int32
tbl_rip_set_rip_field_sync(tbl_rip_t *p_rip, tbl_rip_field_id_t field_id, uint32 sync)
{
    tbl_rip_t *p_db_rip = NULL;

    /* 1. lookup entry exist */
    p_db_rip = tbl_rip_get_rip(&p_rip->key);
    if (NULL == p_db_rip)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RIP_FLD_VERSION:
        p_db_rip->version = p_rip->version;
        break;

    case TBL_RIP_FLD_ALLOW_ECMP:
        p_db_rip->allow_ecmp = p_rip->allow_ecmp;
        break;

    case TBL_RIP_FLD_TIMERS:
        sal_memcpy(&p_db_rip->timers, &p_rip->timers, sizeof(p_rip->timers));
        break;

    case TBL_RIP_FLD_DEFAULT_INFORMATION:
        p_db_rip->default_information = p_rip->default_information;
        break;

    case TBL_RIP_FLD_REDISTRIBUTE:
        p_db_rip->redistribute = p_rip->redistribute;
        break;

    case TBL_RIP_FLD_RED_METRIC:
        sal_memcpy(p_db_rip->red_metric, p_rip->red_metric, sizeof(p_rip->red_metric));
        break;

    case TBL_RIP_FLD_DEFAULT_METRIC:
        p_db_rip->default_metric = p_rip->default_metric;
        break;

    case TBL_RIP_FLD_PASSIVE_IF_DEFAULT:
        p_db_rip->passive_if_default = p_rip->passive_if_default;
        break;

    case TBL_RIP_FLD_MAX:
        sal_memcpy(p_db_rip, p_rip, sizeof(tbl_rip_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RIP, field_id, p_db_rip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RIP, field_id, p_db_rip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rip_add_rip(tbl_rip_t *p_rip)
{
    return tbl_rip_add_rip_sync(p_rip, TRUE);
}

int32
tbl_rip_del_rip(tbl_rip_key_t *p_rip_key)
{
    return tbl_rip_del_rip_sync(p_rip_key, TRUE);
}

int32
tbl_rip_set_rip_field(tbl_rip_t *p_rip, tbl_rip_field_id_t field_id)
{
    return tbl_rip_set_rip_field_sync(p_rip, field_id, TRUE);
}

tbl_rip_t*
tbl_rip_get_rip(tbl_rip_key_t *p_rip_key)
{
    tbl_rip_master_t *p_master = _g_p_tbl_rip_master;
    tbl_rip_t lkp;

    sal_memcpy(&lkp.key, p_rip_key, sizeof(tbl_rip_key_t));
    return ctclib_hash_lookup(p_master->rip_hash, &lkp);
}

char*
tbl_rip_key_val2str(tbl_rip_t *p_rip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rip->key.id);
    return str;
}

int32
tbl_rip_key_str2val(char *str, tbl_rip_t *p_rip)
{
    int32 ret = 0;

    p_rip->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_rip_key_name_dump(tbl_rip_t *p_rip, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_rip->key.id);
    return str;
}

char*
tbl_rip_key_value_dump(tbl_rip_t *p_rip, char *str)
{
    sal_sprintf(str, "%u", p_rip->key.id);
    return str;
}

char*
tbl_rip_field_name_dump(tbl_rip_t *p_rip, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RIP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rip_field_value_dump(tbl_rip_t *p_rip, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RIP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_rip->key.id);
    }
    if (FLD_MATCH(TBL_RIP_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_rip->version);
    }
    if (FLD_MATCH(TBL_RIP_FLD_ALLOW_ECMP, field_id))
    {
        sal_sprintf(str, "%u", p_rip->allow_ecmp);
    }
    if (FLD_MATCH(TBL_RIP_FLD_TIMERS, field_id))
    {
        cdb_rip_timers_val2str(str, MAX_CMD_STR_LEN, &p_rip->timers);
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_INFORMATION, field_id))
    {
        sal_sprintf(str, "%u", p_rip->default_information);
    }
    if (FLD_MATCH(TBL_RIP_FLD_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u", p_rip->redistribute);
    }
    if (FLD_MATCH(TBL_RIP_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_rip->red_metric, sizeof(p_rip->red_metric)/4);
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_METRIC, field_id))
    {
        sal_sprintf(str, "%u", p_rip->default_metric);
    }
    if (FLD_MATCH(TBL_RIP_FLD_PASSIVE_IF_DEFAULT, field_id))
    {
        sal_sprintf(str, "%u", p_rip->passive_if_default);
    }
    return str;
}

char**
tbl_rip_table_dump(tbl_rip_t *p_rip, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rip_key_name_dump(p_rip, buf));
    for(i=1; i<TBL_RIP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rip_field_name_dump(p_rip, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rip_field_value_dump(p_rip, i, buf));
    }
    return str;
}

int32
tbl_rip_field_value_parser(char *str, int32 field_id, tbl_rip_t *p_rip)
{
    if (FLD_MATCH(TBL_RIP_FLD_KEY, field_id))
    {
        int32 ret;
        p_rip->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_VERSION, field_id))
    {
        int32 ret;
        p_rip->version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_ALLOW_ECMP, field_id))
    {
        int32 ret;
        p_rip->allow_ecmp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_TIMERS, field_id))
    {
        cdb_rip_timers_str2val(str, &p_rip->timers);
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_INFORMATION, field_id))
    {
        int32 ret;
        p_rip->default_information = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_REDISTRIBUTE, field_id))
    {
        int32 ret;
        p_rip->redistribute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_str2val(str, p_rip->red_metric);
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_METRIC, field_id))
    {
        int32 ret;
        p_rip->default_metric = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIP_FLD_PASSIVE_IF_DEFAULT, field_id))
    {
        int32 ret;
        p_rip->passive_if_default = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_rip_table_parser(char** array, char* key_value,tbl_rip_t *p_rip)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rip_key_str2val(key_value, p_rip));

    for(i=1; i<TBL_RIP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rip_field_value_parser( array[j++], i, p_rip));
    }

    return PM_E_NONE;
}

int32
tbl_rip_dump_one(tbl_rip_t *p_rip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RIP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_KEY].name,
            p_rip->key.id);
    }
    if (FLD_MATCH(TBL_RIP_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_VERSION].name,
            p_rip->version);
    }
    if (FLD_MATCH(TBL_RIP_FLD_ALLOW_ECMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_ALLOW_ECMP].name,
            p_rip->allow_ecmp);
    }
    if (FLD_MATCH(TBL_RIP_FLD_TIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RIP_FLD_TIMERS].name, 
            cdb_rip_timers_val2str(buf, MAX_CMD_STR_LEN, &p_rip->timers));
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_INFORMATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_DEFAULT_INFORMATION].name,
            p_rip->default_information);
    }
    if (FLD_MATCH(TBL_RIP_FLD_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_REDISTRIBUTE].name,
            p_rip->redistribute);
    }
    if (FLD_MATCH(TBL_RIP_FLD_RED_METRIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RIP_FLD_RED_METRIC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_rip->red_metric, sizeof(p_rip->red_metric)/4));
    }
    if (FLD_MATCH(TBL_RIP_FLD_DEFAULT_METRIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_DEFAULT_METRIC].name,
            p_rip->default_metric);
    }
    if (FLD_MATCH(TBL_RIP_FLD_PASSIVE_IF_DEFAULT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIP_FLD_PASSIVE_IF_DEFAULT].name,
            p_rip->passive_if_default);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rip_master_t *p_master = _g_p_tbl_rip_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rip_t *p_db_rip = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rip_list, p_db_rip, listnode, next)
    {
        rc |= fn(p_db_rip, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rip_master_t*
tbl_rip_get_master()
{
    return _g_p_tbl_rip_master;
}

tbl_rip_master_t*
tbl_rip_init_rip()
{
    _g_p_tbl_rip_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rip_master_t));
    _g_p_tbl_rip_master->rip_hash = ctclib_hash_create(_tbl_rip_hash_make, _tbl_rip_hash_cmp);
    _g_p_tbl_rip_master->rip_list = ctclib_slist_create(tbl_rip_cmp, NULL);
    return _g_p_tbl_rip_master;
}


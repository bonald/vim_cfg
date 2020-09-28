
#include "proto.h"
#include "gen/tbl_controller_attr_define.h"
#include "gen/tbl_controller_attr.h"
#include "cdb_data_cmp.h"

tbl_controller_attr_master_t *_g_p_tbl_controller_attr_master = NULL;

static uint32
_tbl_controller_attr_hash_make(tbl_controller_attr_t *p_controller_attr)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_controller_attr->key;
    for (index = 0; index < sizeof(p_controller_attr->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_controller_attr_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_controller_attr_t *p_controller_attr1 = (tbl_controller_attr_t*)p_arg1;
    tbl_controller_attr_t *p_controller_attr2 = (tbl_controller_attr_t*)p_arg2;

    if (0 == sal_memcmp(&p_controller_attr1->key, &p_controller_attr2->key, sizeof(tbl_controller_attr_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_controller_attr_add_controller_attr_sync(tbl_controller_attr_t *p_controller_attr, uint32 sync)
{
    tbl_controller_attr_master_t *p_master = _g_p_tbl_controller_attr_master;
    tbl_controller_attr_t *p_db_controller_attr = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_controller_attr_get_controller_attr(&p_controller_attr->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_controller_attr = XCALLOC(MEM_TBL_CONTROLLER_ATTR, sizeof(tbl_controller_attr_t));
    if (NULL == p_db_controller_attr)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_controller_attr, p_controller_attr, sizeof(tbl_controller_attr_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->controller_attr_hash, (void*)p_db_controller_attr, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CONTROLLER_ATTR, p_db_controller_attr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CONTROLLER_ATTR, p_db_controller_attr);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_controller_attr_del_controller_attr_sync(tbl_controller_attr_key_t *p_controller_attr_key, uint32 sync)
{
    tbl_controller_attr_master_t *p_master = _g_p_tbl_controller_attr_master;
    tbl_controller_attr_t *p_db_controller_attr = NULL;

    /* 1. lookup entry exist */
    p_db_controller_attr = tbl_controller_attr_get_controller_attr(p_controller_attr_key);
    if (NULL == p_db_controller_attr)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CONTROLLER_ATTR, p_db_controller_attr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CONTROLLER_ATTR, p_db_controller_attr);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->controller_attr_hash, (void*)p_db_controller_attr);

    /* 4. free db entry */
    XFREE(MEM_TBL_CONTROLLER_ATTR, p_db_controller_attr);

    return PM_E_NONE;
}

int32
tbl_controller_attr_set_controller_attr_field_sync(tbl_controller_attr_t *p_controller_attr, tbl_controller_attr_field_id_t field_id, uint32 sync)
{
    tbl_controller_attr_t *p_db_controller_attr = NULL;

    /* 1. lookup entry exist */
    p_db_controller_attr = tbl_controller_attr_get_controller_attr(&p_controller_attr->key);
    if (NULL == p_db_controller_attr)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME:
        sal_memcpy(p_db_controller_attr->controller_name, p_controller_attr->controller_name, sizeof(p_controller_attr->controller_name));
        break;

    case TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF:
        p_db_controller_attr->max_backoff = p_controller_attr->max_backoff;
        break;

    case TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE:
        p_db_controller_attr->inactivity_probe = p_controller_attr->inactivity_probe;
        break;

    case TBL_CONTROLLER_ATTR_FLD_MAX:
        sal_memcpy(p_db_controller_attr, p_controller_attr, sizeof(tbl_controller_attr_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CONTROLLER_ATTR, field_id, p_db_controller_attr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CONTROLLER_ATTR, field_id, p_db_controller_attr);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_controller_attr_add_controller_attr(tbl_controller_attr_t *p_controller_attr)
{
    return tbl_controller_attr_add_controller_attr_sync(p_controller_attr, TRUE);
}

int32
tbl_controller_attr_del_controller_attr(tbl_controller_attr_key_t *p_controller_attr_key)
{
    return tbl_controller_attr_del_controller_attr_sync(p_controller_attr_key, TRUE);
}

int32
tbl_controller_attr_set_controller_attr_field(tbl_controller_attr_t *p_controller_attr, tbl_controller_attr_field_id_t field_id)
{
    return tbl_controller_attr_set_controller_attr_field_sync(p_controller_attr, field_id, TRUE);
}

tbl_controller_attr_t*
tbl_controller_attr_get_controller_attr(tbl_controller_attr_key_t *p_controller_attr_key)
{
    tbl_controller_attr_master_t *p_master = _g_p_tbl_controller_attr_master;
    tbl_controller_attr_t lkp;

    sal_memcpy(&lkp.key, p_controller_attr_key, sizeof(tbl_controller_attr_key_t));
    return ctclib_hash_lookup(p_master->controller_attr_hash, &lkp);
}

char*
tbl_controller_attr_key_val2str(tbl_controller_attr_t *p_controller_attr, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_controller_attr->key));
    return str;
}

int32
tbl_controller_attr_key_str2val(char *str, tbl_controller_attr_t *p_controller_attr)
{
    int32 ret = 0;

    ret = cdb_ns_route_key_str2val(str, &p_controller_attr->key);
    return ret;
}

char*
tbl_controller_attr_key_name_dump(tbl_controller_attr_t *p_controller_attr, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER_ATTR);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_controller_attr->key));
    return str;
}

char*
tbl_controller_attr_key_value_dump(tbl_controller_attr_t *p_controller_attr, char *str)
{

    cdb_ns_route_key_val2str(str, MAX_CMD_STR_LEN, &p_controller_attr->key);
    return str;
}

char*
tbl_controller_attr_field_name_dump(tbl_controller_attr_t *p_controller_attr, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER_ATTR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CONTROLLER_ATTR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_controller_attr_field_value_dump(tbl_controller_attr_t *p_controller_attr, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_KEY, field_id))
    {
        cdb_ns_route_key_val2str(str, MAX_CMD_STR_LEN, &p_controller_attr->key);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_controller_attr->controller_name);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF, field_id))
    {
        sal_sprintf(str, "%u", p_controller_attr->max_backoff);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE, field_id))
    {
        sal_sprintf(str, "%u", p_controller_attr->inactivity_probe);
    }
    return str;
}

char**
tbl_controller_attr_table_dump(tbl_controller_attr_t *p_controller_attr, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_controller_attr_key_name_dump(p_controller_attr, buf));
    for(i=1; i<TBL_CONTROLLER_ATTR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_controller_attr_field_name_dump(p_controller_attr, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_controller_attr_field_value_dump(p_controller_attr, i, buf));
    }
    return str;
}

int32
tbl_controller_attr_field_value_parser(char *str, int32 field_id, tbl_controller_attr_t *p_controller_attr)
{
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_KEY, field_id))
    {
        cdb_ns_route_key_str2val(str, &p_controller_attr->key);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME, field_id))
    {
        sal_strcpy(p_controller_attr->controller_name, str);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF, field_id))
    {
        int32 ret;
        p_controller_attr->max_backoff = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE, field_id))
    {
        int32 ret;
        p_controller_attr->inactivity_probe = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_controller_attr_table_parser(char** array, char* key_value,tbl_controller_attr_t *p_controller_attr)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER_ATTR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_controller_attr_key_str2val(key_value, p_controller_attr));

    for(i=1; i<TBL_CONTROLLER_ATTR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_controller_attr_field_value_parser( array[j++], i, p_controller_attr));
    }

    return PM_E_NONE;
}

int32
tbl_controller_attr_dump_one(tbl_controller_attr_t *p_controller_attr, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER_ATTR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONTROLLER_ATTR_FLD_KEY].name, 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_controller_attr->key));
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONTROLLER_ATTR_FLD_CONTROLLER_NAME].name,
            p_controller_attr->controller_name);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONTROLLER_ATTR_FLD_MAX_BACKOFF].name,
            p_controller_attr->max_backoff);
    }
    if (FLD_MATCH(TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONTROLLER_ATTR_FLD_INACTIVITY_PROBE].name,
            p_controller_attr->inactivity_probe);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_controller_attr_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_controller_attr_master_t *p_master = _g_p_tbl_controller_attr_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->controller_attr_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_controller_attr_master_t*
tbl_controller_attr_get_master()
{
    return _g_p_tbl_controller_attr_master;
}

tbl_controller_attr_master_t*
tbl_controller_attr_init_controller_attr()
{
    _g_p_tbl_controller_attr_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_controller_attr_master_t));
    _g_p_tbl_controller_attr_master->controller_attr_hash = ctclib_hash_create(_tbl_controller_attr_hash_make, _tbl_controller_attr_hash_cmp);
    return _g_p_tbl_controller_attr_master;
}


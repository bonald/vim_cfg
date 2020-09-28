
#include "proto.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "cdb_data_cmp.h"

tbl_snmp_notify_master_t *_g_p_tbl_snmp_notify_master = NULL;

static uint32
_tbl_snmp_notify_hash_make(tbl_snmp_notify_t *p_snmp_notify)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_snmp_notify->key;
    for (index = 0; index < sizeof(p_snmp_notify->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_snmp_notify_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_snmp_notify_t *p_snmp_notify1 = (tbl_snmp_notify_t*)p_arg1;
    tbl_snmp_notify_t *p_snmp_notify2 = (tbl_snmp_notify_t*)p_arg2;

    if (0 == sal_memcmp(&p_snmp_notify1->key, &p_snmp_notify2->key, sizeof(tbl_snmp_notify_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_snmp_notify_add_snmp_notify_sync(tbl_snmp_notify_t *p_snmp_notify, uint32 sync)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    tbl_snmp_notify_t *p_db_snmp_notify = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_notify_get_snmp_notify(&p_snmp_notify->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_notify = XCALLOC(MEM_TBL_SNMP_NOTIFY, sizeof(tbl_snmp_notify_t));
    if (NULL == p_db_snmp_notify)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_notify, p_snmp_notify, sizeof(tbl_snmp_notify_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->snmp_notify_hash, (void*)p_db_snmp_notify, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->snmp_notify_list, p_db_snmp_notify);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_NOTIFY, p_db_snmp_notify);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_NOTIFY, p_db_snmp_notify);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_notify_del_snmp_notify_sync(tbl_snmp_notify_key_t *p_snmp_notify_key, uint32 sync)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    tbl_snmp_notify_t *p_db_snmp_notify = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_notify = tbl_snmp_notify_get_snmp_notify(p_snmp_notify_key);
    if (NULL == p_db_snmp_notify)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_NOTIFY, p_db_snmp_notify);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_NOTIFY, p_db_snmp_notify);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->snmp_notify_hash, (void*)p_db_snmp_notify);
    ctclib_slistnode_delete(p_master->snmp_notify_list, p_db_snmp_notify);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_NOTIFY, p_db_snmp_notify);

    return PM_E_NONE;
}

int32
tbl_snmp_notify_set_snmp_notify_field_sync(tbl_snmp_notify_t *p_snmp_notify, tbl_snmp_notify_field_id_t field_id, uint32 sync)
{
    tbl_snmp_notify_t *p_db_snmp_notify = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_notify = tbl_snmp_notify_get_snmp_notify(&p_snmp_notify->key);
    if (NULL == p_db_snmp_notify)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_NOTIFY_FLD_FLAGS:
        p_db_snmp_notify->flags = p_snmp_notify->flags;
        break;

    case TBL_SNMP_NOTIFY_FLD_TAG_NAME:
        sal_memcpy(p_db_snmp_notify->tag_name, p_snmp_notify->tag_name, sizeof(p_snmp_notify->tag_name));
        break;

    case TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE:
        p_db_snmp_notify->notify_type = p_snmp_notify->notify_type;
        break;

    case TBL_SNMP_NOTIFY_FLD_MAX:
        sal_memcpy(p_db_snmp_notify, p_snmp_notify, sizeof(tbl_snmp_notify_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_NOTIFY, field_id, p_db_snmp_notify);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_NOTIFY, field_id, p_db_snmp_notify);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_notify_add_snmp_notify(tbl_snmp_notify_t *p_snmp_notify)
{
    return tbl_snmp_notify_add_snmp_notify_sync(p_snmp_notify, TRUE);
}

int32
tbl_snmp_notify_del_snmp_notify(tbl_snmp_notify_key_t *p_snmp_notify_key)
{
    return tbl_snmp_notify_del_snmp_notify_sync(p_snmp_notify_key, TRUE);
}

int32
tbl_snmp_notify_set_snmp_notify_field(tbl_snmp_notify_t *p_snmp_notify, tbl_snmp_notify_field_id_t field_id)
{
    return tbl_snmp_notify_set_snmp_notify_field_sync(p_snmp_notify, field_id, TRUE);
}

tbl_snmp_notify_t*
tbl_snmp_notify_get_snmp_notify(tbl_snmp_notify_key_t *p_snmp_notify_key)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    tbl_snmp_notify_t lkp;

    sal_memcpy(&lkp.key, p_snmp_notify_key, sizeof(tbl_snmp_notify_key_t));
    return ctclib_hash_lookup(p_master->snmp_notify_hash, &lkp);
}

char*
tbl_snmp_notify_key_val2str(tbl_snmp_notify_t *p_snmp_notify, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_snmp_notify->key.notify_name);
    return str;
}

int32
tbl_snmp_notify_key_str2val(char *str, tbl_snmp_notify_t *p_snmp_notify)
{
    int32 ret = 0;

    sal_strcpy(p_snmp_notify->key.notify_name, str);
    return ret;
}

char*
tbl_snmp_notify_key_name_dump(tbl_snmp_notify_t *p_snmp_notify, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_NOTIFY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_snmp_notify->key.notify_name);
    return str;
}

char*
tbl_snmp_notify_key_value_dump(tbl_snmp_notify_t *p_snmp_notify, char *str)
{
    sal_sprintf(str, "%s", p_snmp_notify->key.notify_name);
    return str;
}

char*
tbl_snmp_notify_field_name_dump(tbl_snmp_notify_t *p_snmp_notify, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_NOTIFY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_NOTIFY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_notify_field_value_dump(tbl_snmp_notify_t *p_snmp_notify, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_notify->key.notify_name);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_notify->flags);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_TAG_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_notify->tag_name);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_notify->notify_type);
    }
    return str;
}

char**
tbl_snmp_notify_table_dump(tbl_snmp_notify_t *p_snmp_notify, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_notify_key_name_dump(p_snmp_notify, buf));
    for(i=1; i<TBL_SNMP_NOTIFY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_notify_field_name_dump(p_snmp_notify, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_notify_field_value_dump(p_snmp_notify, i, buf));
    }
    return str;
}

int32
tbl_snmp_notify_field_value_parser(char *str, int32 field_id, tbl_snmp_notify_t *p_snmp_notify)
{
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_KEY, field_id))
    {
        sal_strcpy(p_snmp_notify->key.notify_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_snmp_notify->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_TAG_NAME, field_id))
    {
        sal_strcpy(p_snmp_notify->tag_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE, field_id))
    {
        int32 ret;
        p_snmp_notify->notify_type = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_notify_table_parser(char** array, char* key_value,tbl_snmp_notify_t *p_snmp_notify)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_NOTIFY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_notify_key_str2val(key_value, p_snmp_notify));

    for(i=1; i<TBL_SNMP_NOTIFY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_notify_field_value_parser( array[j++], i, p_snmp_notify));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_notify_dump_one(tbl_snmp_notify_t *p_snmp_notify, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_NOTIFY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_NOTIFY_FLD_KEY].name,
            p_snmp_notify->key.notify_name);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_NOTIFY_FLD_FLAGS].name,
            p_snmp_notify->flags);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_TAG_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_NOTIFY_FLD_TAG_NAME].name,
            p_snmp_notify->tag_name);
    }
    if (FLD_MATCH(TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_NOTIFY_FLD_NOTIFY_TYPE].name,
            p_snmp_notify->notify_type);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_notify_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_notify_t *p_db_snmp_notify = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_notify_list, p_db_snmp_notify, listnode, next)
    {
        rc |= fn(p_db_snmp_notify, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_notify_master_t*
tbl_snmp_notify_get_master()
{
    return _g_p_tbl_snmp_notify_master;
}

tbl_snmp_notify_master_t*
tbl_snmp_notify_init_snmp_notify()
{
    _g_p_tbl_snmp_notify_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_notify_master_t));
    _g_p_tbl_snmp_notify_master->snmp_notify_hash = ctclib_hash_create(_tbl_snmp_notify_hash_make, _tbl_snmp_notify_hash_cmp);
    _g_p_tbl_snmp_notify_master->snmp_notify_list = ctclib_slist_create(tbl_snmp_notify_cmp, NULL);
    return _g_p_tbl_snmp_notify_master;
}

int32
tbl_snmp_notify_num_get()
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_notify_list->count;
    return count;
}

tbl_snmp_notify_t *
tbl_snmp_notify_get_by_tagname(char *tag_name)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_notify_t *p_db_notify = NULL;

    if (NULL == p_master)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_notify_list, p_db_notify, listnode, next)
    {
        if (0 == sal_strcmp(p_db_notify->tag_name, tag_name))
        {
            return p_db_notify;
        }
    }
    return NULL;
}





#include "proto.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "cdb_data_cmp.h"

tbl_snmp_access_usm_master_t *_g_p_tbl_snmp_access_usm_master = NULL;

static uint32
_tbl_snmp_access_usm_hash_make(tbl_snmp_access_usm_t *p_snmp_access_usm)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_snmp_access_usm->key;
    for (index = 0; index < sizeof(p_snmp_access_usm->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_snmp_access_usm_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_snmp_access_usm_t *p_snmp_access_usm1 = (tbl_snmp_access_usm_t*)p_arg1;
    tbl_snmp_access_usm_t *p_snmp_access_usm2 = (tbl_snmp_access_usm_t*)p_arg2;

    if (0 == sal_memcmp(&p_snmp_access_usm1->key, &p_snmp_access_usm2->key, sizeof(tbl_snmp_access_usm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_snmp_access_usm_add_snmp_access_usm_sync(tbl_snmp_access_usm_t *p_snmp_access_usm, uint32 sync)
{
    tbl_snmp_access_usm_master_t *p_master = _g_p_tbl_snmp_access_usm_master;
    tbl_snmp_access_usm_t *p_db_snmp_access_usm = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_access_usm_get_snmp_access_usm(&p_snmp_access_usm->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_access_usm = XCALLOC(MEM_TBL_SNMP_ACCESS_USM, sizeof(tbl_snmp_access_usm_t));
    if (NULL == p_db_snmp_access_usm)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_access_usm, p_snmp_access_usm, sizeof(tbl_snmp_access_usm_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->snmp_access_usm_hash, (void*)p_db_snmp_access_usm, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->snmp_access_usm_list, p_db_snmp_access_usm);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_ACCESS_USM, p_db_snmp_access_usm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_ACCESS_USM, p_db_snmp_access_usm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_del_snmp_access_usm_sync(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key, uint32 sync)
{
    tbl_snmp_access_usm_master_t *p_master = _g_p_tbl_snmp_access_usm_master;
    tbl_snmp_access_usm_t *p_db_snmp_access_usm = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_access_usm = tbl_snmp_access_usm_get_snmp_access_usm(p_snmp_access_usm_key);
    if (NULL == p_db_snmp_access_usm)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_ACCESS_USM, p_db_snmp_access_usm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_ACCESS_USM, p_db_snmp_access_usm);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->snmp_access_usm_hash, (void*)p_db_snmp_access_usm);
    ctclib_slistnode_delete(p_master->snmp_access_usm_list, p_db_snmp_access_usm);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_ACCESS_USM, p_db_snmp_access_usm);

    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_set_snmp_access_usm_field_sync(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_snmp_access_usm_field_id_t field_id, uint32 sync)
{
    tbl_snmp_access_usm_t *p_db_snmp_access_usm = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_access_usm = tbl_snmp_access_usm_get_snmp_access_usm(&p_snmp_access_usm->key);
    if (NULL == p_db_snmp_access_usm)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_ACCESS_USM_FLD_FLAGS:
        p_db_snmp_access_usm->flags = p_snmp_access_usm->flags;
        break;

    case TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL:
        p_db_snmp_access_usm->usm_level = p_snmp_access_usm->usm_level;
        break;

    case TBL_SNMP_ACCESS_USM_FLD_READ_VIEW:
        sal_memcpy(p_db_snmp_access_usm->read_view, p_snmp_access_usm->read_view, sizeof(p_snmp_access_usm->read_view));
        break;

    case TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW:
        sal_memcpy(p_db_snmp_access_usm->write_view, p_snmp_access_usm->write_view, sizeof(p_snmp_access_usm->write_view));
        break;

    case TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW:
        sal_memcpy(p_db_snmp_access_usm->notify_view, p_snmp_access_usm->notify_view, sizeof(p_snmp_access_usm->notify_view));
        break;

    case TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME:
        sal_memcpy(p_db_snmp_access_usm->context_name, p_snmp_access_usm->context_name, sizeof(p_snmp_access_usm->context_name));
        break;

    case TBL_SNMP_ACCESS_USM_FLD_ISPREFIX:
        p_db_snmp_access_usm->isprefix = p_snmp_access_usm->isprefix;
        break;

    case TBL_SNMP_ACCESS_USM_FLD_MAX:
        sal_memcpy(p_db_snmp_access_usm, p_snmp_access_usm, sizeof(tbl_snmp_access_usm_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_ACCESS_USM, field_id, p_db_snmp_access_usm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_ACCESS_USM, field_id, p_db_snmp_access_usm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_add_snmp_access_usm(tbl_snmp_access_usm_t *p_snmp_access_usm)
{
    return tbl_snmp_access_usm_add_snmp_access_usm_sync(p_snmp_access_usm, TRUE);
}

int32
tbl_snmp_access_usm_del_snmp_access_usm(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key)
{
    return tbl_snmp_access_usm_del_snmp_access_usm_sync(p_snmp_access_usm_key, TRUE);
}

int32
tbl_snmp_access_usm_set_snmp_access_usm_field(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_snmp_access_usm_field_id_t field_id)
{
    return tbl_snmp_access_usm_set_snmp_access_usm_field_sync(p_snmp_access_usm, field_id, TRUE);
}

tbl_snmp_access_usm_t*
tbl_snmp_access_usm_get_snmp_access_usm(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key)
{
    tbl_snmp_access_usm_master_t *p_master = _g_p_tbl_snmp_access_usm_master;
    tbl_snmp_access_usm_t lkp;

    sal_memcpy(&lkp.key, p_snmp_access_usm_key, sizeof(tbl_snmp_access_usm_key_t));
    return ctclib_hash_lookup(p_master->snmp_access_usm_hash, &lkp);
}

char*
tbl_snmp_access_usm_key_val2str(tbl_snmp_access_usm_t *p_snmp_access_usm, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_snmp_access_usm->key.usm_access_name);
    return str;
}

int32
tbl_snmp_access_usm_key_str2val(char *str, tbl_snmp_access_usm_t *p_snmp_access_usm)
{
    int32 ret = 0;

    sal_strcpy(p_snmp_access_usm->key.usm_access_name, str);
    return ret;
}

char*
tbl_snmp_access_usm_key_name_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_ACCESS_USM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_snmp_access_usm->key.usm_access_name);
    return str;
}

char*
tbl_snmp_access_usm_key_value_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char *str)
{
    sal_sprintf(str, "%s", p_snmp_access_usm->key.usm_access_name);
    return str;
}

char*
tbl_snmp_access_usm_field_name_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_ACCESS_USM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_ACCESS_USM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_access_usm_field_value_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_access_usm->key.usm_access_name);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_access_usm->flags);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_access_usm->usm_level);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_READ_VIEW, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_access_usm->read_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_access_usm->write_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_access_usm->notify_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_access_usm->context_name);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_ISPREFIX, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_access_usm->isprefix);
    }
    return str;
}

char**
tbl_snmp_access_usm_table_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_access_usm_key_name_dump(p_snmp_access_usm, buf));
    for(i=1; i<TBL_SNMP_ACCESS_USM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_access_usm_field_name_dump(p_snmp_access_usm, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_access_usm_field_value_dump(p_snmp_access_usm, i, buf));
    }
    return str;
}

int32
tbl_snmp_access_usm_field_value_parser(char *str, int32 field_id, tbl_snmp_access_usm_t *p_snmp_access_usm)
{
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_KEY, field_id))
    {
        sal_strcpy(p_snmp_access_usm->key.usm_access_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_snmp_access_usm->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL, field_id))
    {
        int32 ret;
        p_snmp_access_usm->usm_level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_READ_VIEW, field_id))
    {
        sal_strcpy(p_snmp_access_usm->read_view, str);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW, field_id))
    {
        sal_strcpy(p_snmp_access_usm->write_view, str);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW, field_id))
    {
        sal_strcpy(p_snmp_access_usm->notify_view, str);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME, field_id))
    {
        sal_strcpy(p_snmp_access_usm->context_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_ISPREFIX, field_id))
    {
        int32 ret;
        p_snmp_access_usm->isprefix = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_table_parser(char** array, char* key_value,tbl_snmp_access_usm_t *p_snmp_access_usm)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_ACCESS_USM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_access_usm_key_str2val(key_value, p_snmp_access_usm));

    for(i=1; i<TBL_SNMP_ACCESS_USM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_access_usm_field_value_parser( array[j++], i, p_snmp_access_usm));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_dump_one(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_ACCESS_USM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_KEY].name,
            p_snmp_access_usm->key.usm_access_name);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_FLAGS].name,
            p_snmp_access_usm->flags);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_USM_LEVEL].name,
            p_snmp_access_usm->usm_level);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_READ_VIEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_READ_VIEW].name,
            p_snmp_access_usm->read_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_WRITE_VIEW].name,
            p_snmp_access_usm->write_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_NOTIFY_VIEW].name,
            p_snmp_access_usm->notify_view);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_CONTEXT_NAME].name,
            p_snmp_access_usm->context_name);
    }
    if (FLD_MATCH(TBL_SNMP_ACCESS_USM_FLD_ISPREFIX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_ACCESS_USM_FLD_ISPREFIX].name,
            p_snmp_access_usm->isprefix);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_access_usm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_access_usm_master_t *p_master = _g_p_tbl_snmp_access_usm_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_access_usm_t *p_db_snmp_access_usm = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_access_usm_list, p_db_snmp_access_usm, listnode, next)
    {
        rc |= fn(p_db_snmp_access_usm, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_access_usm_master_t*
tbl_snmp_access_usm_get_master()
{
    return _g_p_tbl_snmp_access_usm_master;
}

tbl_snmp_access_usm_master_t*
tbl_snmp_access_usm_init_snmp_access_usm()
{
    _g_p_tbl_snmp_access_usm_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_access_usm_master_t));
    _g_p_tbl_snmp_access_usm_master->snmp_access_usm_hash = ctclib_hash_create(_tbl_snmp_access_usm_hash_make, _tbl_snmp_access_usm_hash_cmp);
    _g_p_tbl_snmp_access_usm_master->snmp_access_usm_list = ctclib_slist_create(tbl_snmp_access_usm_cmp, NULL);
    return _g_p_tbl_snmp_access_usm_master;
}

int32
tbl_snmp_access_usm_num_get()
{
    tbl_snmp_access_usm_master_t *p_master = _g_p_tbl_snmp_access_usm_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_access_usm_list->count;
    return count;
}



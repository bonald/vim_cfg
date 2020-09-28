
#include "proto.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "cdb_data_cmp.h"

tbl_cem_master_t *_g_p_tbl_cem_master = NULL;

static uint32
_tbl_cem_hash_make(tbl_cem_t *p_cem)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_cem->key;
    for (index = 0; index < sizeof(p_cem->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cem_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cem_t *p_cem1 = (tbl_cem_t*)p_arg1;
    tbl_cem_t *p_cem2 = (tbl_cem_t*)p_arg2;

    if (0 == sal_memcmp(&p_cem1->key, &p_cem2->key, sizeof(tbl_cem_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cem_add_cem_sync(tbl_cem_t *p_cem, uint32 sync)
{
    tbl_cem_master_t *p_master = _g_p_tbl_cem_master;
    tbl_cem_t *p_db_cem = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cem_get_cem(&p_cem->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cem = XCALLOC(MEM_TBL_CEM, sizeof(tbl_cem_t));
    if (NULL == p_db_cem)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cem, p_cem, sizeof(tbl_cem_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cem_hash, (void*)p_db_cem, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cem_list, p_db_cem);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CEM, p_db_cem);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CEM, p_db_cem);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cem_del_cem_sync(tbl_cem_key_t *p_cem_key, uint32 sync)
{
    tbl_cem_master_t *p_master = _g_p_tbl_cem_master;
    tbl_cem_t *p_db_cem = NULL;

    /* 1. lookup entry exist */
    p_db_cem = tbl_cem_get_cem(p_cem_key);
    if (NULL == p_db_cem)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CEM, p_db_cem);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CEM, p_db_cem);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cem_hash, (void*)p_db_cem);
    ctclib_slistnode_delete(p_master->cem_list, p_db_cem);

    /* 4. free db entry */
    XFREE(MEM_TBL_CEM, p_db_cem);

    return PM_E_NONE;
}

int32
tbl_cem_set_cem_field_sync(tbl_cem_t *p_cem, tbl_cem_field_id_t field_id, uint32 sync)
{
    tbl_cem_t *p_db_cem = NULL;

    /* 1. lookup entry exist */
    p_db_cem = tbl_cem_get_cem(&p_cem->key);
    if (NULL == p_db_cem)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CEM_FLD_EVENT:
        sal_memcpy(p_db_cem->event, p_cem->event, sizeof(p_cem->event));
        break;

    case TBL_CEM_FLD_THRESHOLD:
        p_db_cem->threshold = p_cem->threshold;
        break;

    case TBL_CEM_FLD_SNMPTRAP:
        p_db_cem->snmptrap = p_cem->snmptrap;
        break;

    case TBL_CEM_FLD_LOADPATH:
        sal_memcpy(p_db_cem->loadpath, p_cem->loadpath, sizeof(p_cem->loadpath));
        break;

    case TBL_CEM_FLD_MAIL:
        sal_memcpy(&p_db_cem->mail, &p_cem->mail, sizeof(p_cem->mail));
        break;

    case TBL_CEM_FLD_MAX:
        sal_memcpy(p_db_cem, p_cem, sizeof(tbl_cem_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CEM, field_id, p_db_cem);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CEM, field_id, p_db_cem);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cem_add_cem(tbl_cem_t *p_cem)
{
    return tbl_cem_add_cem_sync(p_cem, TRUE);
}

int32
tbl_cem_del_cem(tbl_cem_key_t *p_cem_key)
{
    return tbl_cem_del_cem_sync(p_cem_key, TRUE);
}

int32
tbl_cem_set_cem_field(tbl_cem_t *p_cem, tbl_cem_field_id_t field_id)
{
    return tbl_cem_set_cem_field_sync(p_cem, field_id, TRUE);
}

tbl_cem_t*
tbl_cem_get_cem(tbl_cem_key_t *p_cem_key)
{
    tbl_cem_master_t *p_master = _g_p_tbl_cem_master;
    tbl_cem_t lkp;

    sal_memcpy(&lkp.key, p_cem_key, sizeof(tbl_cem_key_t));
    return ctclib_hash_lookup(p_master->cem_hash, &lkp);
}

char*
tbl_cem_key_val2str(tbl_cem_t *p_cem, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_cem->key.name);
    return str;
}

int32
tbl_cem_key_str2val(char *str, tbl_cem_t *p_cem)
{
    int32 ret = 0;

    sal_strcpy(p_cem->key.name, str);
    return ret;
}

char*
tbl_cem_key_name_dump(tbl_cem_t *p_cem, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CEM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_cem->key.name);
    return str;
}

char*
tbl_cem_key_value_dump(tbl_cem_t *p_cem, char *str)
{
    sal_sprintf(str, "%s", p_cem->key.name);
    return str;
}

char*
tbl_cem_field_name_dump(tbl_cem_t *p_cem, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CEM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CEM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cem_field_value_dump(tbl_cem_t *p_cem, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CEM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_cem->key.name);
    }
    if (FLD_MATCH(TBL_CEM_FLD_EVENT, field_id))
    {
        sal_sprintf(str, "%s", p_cem->event);
    }
    if (FLD_MATCH(TBL_CEM_FLD_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_cem->threshold);
    }
    if (FLD_MATCH(TBL_CEM_FLD_SNMPTRAP, field_id))
    {
        sal_sprintf(str, "%u", p_cem->snmptrap);
    }
    if (FLD_MATCH(TBL_CEM_FLD_LOADPATH, field_id))
    {
        sal_sprintf(str, "%s", p_cem->loadpath);
    }
    if (FLD_MATCH(TBL_CEM_FLD_MAIL, field_id))
    {
        cdb_cem_mail_val2str(str, MAX_CMD_STR_LEN, &p_cem->mail);
    }
    return str;
}

char**
tbl_cem_table_dump(tbl_cem_t *p_cem, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cem_key_name_dump(p_cem, buf));
    for(i=1; i<TBL_CEM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cem_field_name_dump(p_cem, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cem_field_value_dump(p_cem, i, buf));
    }
    return str;
}

int32
tbl_cem_field_value_parser(char *str, int32 field_id, tbl_cem_t *p_cem)
{
    if (FLD_MATCH(TBL_CEM_FLD_KEY, field_id))
    {
        sal_strcpy(p_cem->key.name, str);
    }
    if (FLD_MATCH(TBL_CEM_FLD_EVENT, field_id))
    {
        sal_strcpy(p_cem->event, str);
    }
    if (FLD_MATCH(TBL_CEM_FLD_THRESHOLD, field_id))
    {
        int32 ret;
        p_cem->threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CEM_FLD_SNMPTRAP, field_id))
    {
        int32 ret;
        p_cem->snmptrap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CEM_FLD_LOADPATH, field_id))
    {
        sal_strcpy(p_cem->loadpath, str);
    }
    if (FLD_MATCH(TBL_CEM_FLD_MAIL, field_id))
    {
        cdb_cem_mail_str2val(str, &p_cem->mail);
    }
    return PM_E_NONE;
}

int32
tbl_cem_table_parser(char** array, char* key_value,tbl_cem_t *p_cem)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CEM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cem_key_str2val(key_value, p_cem));

    for(i=1; i<TBL_CEM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cem_field_value_parser( array[j++], i, p_cem));
    }

    return PM_E_NONE;
}

int32
tbl_cem_dump_one(tbl_cem_t *p_cem, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CEM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CEM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CEM_FLD_KEY].name,
            p_cem->key.name);
    }
    if (FLD_MATCH(TBL_CEM_FLD_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CEM_FLD_EVENT].name,
            p_cem->event);
    }
    if (FLD_MATCH(TBL_CEM_FLD_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CEM_FLD_THRESHOLD].name,
            p_cem->threshold);
    }
    if (FLD_MATCH(TBL_CEM_FLD_SNMPTRAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CEM_FLD_SNMPTRAP].name,
            p_cem->snmptrap);
    }
    if (FLD_MATCH(TBL_CEM_FLD_LOADPATH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CEM_FLD_LOADPATH].name,
            p_cem->loadpath);
    }
    if (FLD_MATCH(TBL_CEM_FLD_MAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CEM_FLD_MAIL].name, 
            cdb_cem_mail_val2str(buf, MAX_CMD_STR_LEN, &p_cem->mail));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cem_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cem_master_t *p_master = _g_p_tbl_cem_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cem_t *p_db_cem = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cem_list, p_db_cem, listnode, next)
    {
        rc |= fn(p_db_cem, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cem_master_t*
tbl_cem_get_master()
{
    return _g_p_tbl_cem_master;
}

tbl_cem_master_t*
tbl_cem_init_cem()
{
    _g_p_tbl_cem_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cem_master_t));
    _g_p_tbl_cem_master->cem_hash = ctclib_hash_create(_tbl_cem_hash_make, _tbl_cem_hash_cmp);
    _g_p_tbl_cem_master->cem_list = ctclib_slist_create(tbl_cem_cmp, NULL);
    return _g_p_tbl_cem_master;
}


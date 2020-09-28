
#include "proto.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "cdb_data_cmp.h"

tbl_nat_session_master_t *_g_p_tbl_nat_session_master = NULL;

static uint32
_tbl_nat_session_hash_make(tbl_nat_session_t *p_nat_session)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nat_session->key;
    for (index = 0; index < sizeof(p_nat_session->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nat_session_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nat_session_t *p_nat_session1 = (tbl_nat_session_t*)p_arg1;
    tbl_nat_session_t *p_nat_session2 = (tbl_nat_session_t*)p_arg2;

    if (0 == sal_memcmp(&p_nat_session1->key, &p_nat_session2->key, sizeof(tbl_nat_session_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nat_session_add_nat_session_sync(tbl_nat_session_t *p_nat_session, uint32 sync)
{
    tbl_nat_session_master_t *p_master = _g_p_tbl_nat_session_master;
    tbl_nat_session_t *p_db_nat_session = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nat_session_get_nat_session(&p_nat_session->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nat_session = XCALLOC(MEM_TBL_NAT_SESSION, sizeof(tbl_nat_session_t));
    if (NULL == p_db_nat_session)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nat_session, p_nat_session, sizeof(tbl_nat_session_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nat_session_hash, (void*)p_db_nat_session, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->nat_session_list, p_db_nat_session);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NAT_SESSION, p_db_nat_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NAT_SESSION, p_db_nat_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_del_nat_session_sync(tbl_nat_session_key_t *p_nat_session_key, uint32 sync)
{
    tbl_nat_session_master_t *p_master = _g_p_tbl_nat_session_master;
    tbl_nat_session_t *p_db_nat_session = NULL;

    /* 1. lookup entry exist */
    p_db_nat_session = tbl_nat_session_get_nat_session(p_nat_session_key);
    if (NULL == p_db_nat_session)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NAT_SESSION, p_db_nat_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NAT_SESSION, p_db_nat_session);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nat_session_hash, (void*)p_db_nat_session);
    ctclib_slistnode_delete(p_master->nat_session_list, p_db_nat_session);

    /* 4. free db entry */
    XFREE(MEM_TBL_NAT_SESSION, p_db_nat_session);

    return PM_E_NONE;
}

int32
tbl_nat_session_set_nat_session_field_sync(tbl_nat_session_t *p_nat_session, tbl_nat_session_field_id_t field_id, uint32 sync)
{
    tbl_nat_session_t *p_db_nat_session = NULL;

    /* 1. lookup entry exist */
    p_db_nat_session = tbl_nat_session_get_nat_session(&p_nat_session->key);
    if (NULL == p_db_nat_session)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NAT_SESSION_FLD_ID:
        p_db_nat_session->id = p_nat_session->id;
        break;

    case TBL_NAT_SESSION_FLD_TIMESTAMP:
        sal_memcpy(&p_db_nat_session->timestamp, &p_nat_session->timestamp, sizeof(p_nat_session->timestamp));
        break;

    case TBL_NAT_SESSION_FLD_HIT:
        sal_memcpy(&p_db_nat_session->hit, &p_nat_session->hit, sizeof(p_nat_session->hit));
        break;

    case TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME:
        sal_memcpy(&p_db_nat_session->upd_kernel_ttl_time, &p_nat_session->upd_kernel_ttl_time, sizeof(p_nat_session->upd_kernel_ttl_time));
        break;

    case TBL_NAT_SESSION_FLD_TTL:
        p_db_nat_session->ttl = p_nat_session->ttl;
        break;

    case TBL_NAT_SESSION_FLD_IIF_INDEX:
        p_db_nat_session->iif_index = p_nat_session->iif_index;
        break;

    case TBL_NAT_SESSION_FLD_OIF_INDEX:
        p_db_nat_session->oif_index = p_nat_session->oif_index;
        break;

    case TBL_NAT_SESSION_FLD_HELPER:
        sal_memcpy(p_db_nat_session->helper, p_nat_session->helper, sizeof(p_nat_session->helper));
        break;

    case TBL_NAT_SESSION_FLD_SLOW_PATH_FWD:
        p_db_nat_session->slow_path_fwd = p_nat_session->slow_path_fwd;
        break;

    case TBL_NAT_SESSION_FLD_CREATOR:
        p_db_nat_session->creator = p_nat_session->creator;
        break;

    case TBL_NAT_SESSION_FLD_SET_SAI_STATUS:
        p_db_nat_session->set_sai_status = p_nat_session->set_sai_status;
        break;

    case TBL_NAT_SESSION_FLD_MAX:
        sal_memcpy(p_db_nat_session, p_nat_session, sizeof(tbl_nat_session_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NAT_SESSION, field_id, p_db_nat_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NAT_SESSION, field_id, p_db_nat_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_add_nat_session(tbl_nat_session_t *p_nat_session)
{
    return tbl_nat_session_add_nat_session_sync(p_nat_session, TRUE);
}

int32
tbl_nat_session_del_nat_session(tbl_nat_session_key_t *p_nat_session_key)
{
    return tbl_nat_session_del_nat_session_sync(p_nat_session_key, TRUE);
}

int32
tbl_nat_session_set_nat_session_field(tbl_nat_session_t *p_nat_session, tbl_nat_session_field_id_t field_id)
{
    return tbl_nat_session_set_nat_session_field_sync(p_nat_session, field_id, TRUE);
}

tbl_nat_session_t*
tbl_nat_session_get_nat_session(tbl_nat_session_key_t *p_nat_session_key)
{
    tbl_nat_session_master_t *p_master = _g_p_tbl_nat_session_master;
    tbl_nat_session_t lkp;

    sal_memcpy(&lkp.key, p_nat_session_key, sizeof(tbl_nat_session_key_t));
    return ctclib_hash_lookup(p_master->nat_session_hash, &lkp);
}

char*
tbl_nat_session_key_val2str(tbl_nat_session_t *p_nat_session, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nat_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->key));
    return str;
}

int32
tbl_nat_session_key_str2val(char *str, tbl_nat_session_t *p_nat_session)
{
    int32 ret = 0;

    ret = cdb_nat_session_key_str2val(str, &p_nat_session->key);
    return ret;
}

char*
tbl_nat_session_key_name_dump(tbl_nat_session_t *p_nat_session, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nat_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->key));
    return str;
}

char*
tbl_nat_session_key_value_dump(tbl_nat_session_t *p_nat_session, char *str)
{

    cdb_nat_session_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_session->key);
    return str;
}

char*
tbl_nat_session_field_name_dump(tbl_nat_session_t *p_nat_session, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NAT_SESSION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nat_session_field_value_dump(tbl_nat_session_t *p_nat_session, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_KEY, field_id))
    {
        cdb_nat_session_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_session->key);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_ID, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->id);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TIMESTAMP, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_nat_session->timestamp);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HIT, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_nat_session->hit);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_nat_session->upd_kernel_ttl_time);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->ttl);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_IIF_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->iif_index);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_OIF_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->oif_index);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HELPER, field_id))
    {
        sal_sprintf(str, "%s", p_nat_session->helper);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SLOW_PATH_FWD, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->slow_path_fwd);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_CREATOR, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->creator);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SET_SAI_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session->set_sai_status);
    }
    return str;
}

char**
tbl_nat_session_table_dump(tbl_nat_session_t *p_nat_session, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nat_session_key_name_dump(p_nat_session, buf));
    for(i=1; i<TBL_NAT_SESSION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nat_session_field_name_dump(p_nat_session, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nat_session_field_value_dump(p_nat_session, i, buf));
    }
    return str;
}

int32
tbl_nat_session_field_value_parser(char *str, int32 field_id, tbl_nat_session_t *p_nat_session)
{
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_KEY, field_id))
    {
        cdb_nat_session_key_str2val(str, &p_nat_session->key);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_ID, field_id))
    {
        int32 ret;
        p_nat_session->id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TIMESTAMP, field_id))
    {
        cdb_sal_time_str2val(str, &p_nat_session->timestamp);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HIT, field_id))
    {
        cdb_sal_time_str2val(str, &p_nat_session->hit);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_nat_session->upd_kernel_ttl_time);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TTL, field_id))
    {
        int32 ret;
        p_nat_session->ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_IIF_INDEX, field_id))
    {
        int32 ret;
        p_nat_session->iif_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_OIF_INDEX, field_id))
    {
        int32 ret;
        p_nat_session->oif_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HELPER, field_id))
    {
        sal_strcpy(p_nat_session->helper, str);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SLOW_PATH_FWD, field_id))
    {
        int32 ret;
        p_nat_session->slow_path_fwd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_CREATOR, field_id))
    {
        int32 ret;
        p_nat_session->creator = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SET_SAI_STATUS, field_id))
    {
        int32 ret;
        p_nat_session->set_sai_status = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nat_session_table_parser(char** array, char* key_value,tbl_nat_session_t *p_nat_session)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nat_session_key_str2val(key_value, p_nat_session));

    for(i=1; i<TBL_NAT_SESSION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nat_session_field_value_parser( array[j++], i, p_nat_session));
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_dump_one(tbl_nat_session_t *p_nat_session, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NAT_SESSION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_FLD_KEY].name, 
            cdb_nat_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->key));
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_ID].name,
            p_nat_session->id);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TIMESTAMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_FLD_TIMESTAMP].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->timestamp));
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_FLD_HIT].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->hit));
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_FLD_UPD_KERNEL_TTL_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session->upd_kernel_ttl_time));
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_TTL].name,
            p_nat_session->ttl);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_IIF_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_IIF_INDEX].name,
            p_nat_session->iif_index);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_OIF_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_OIF_INDEX].name,
            p_nat_session->oif_index);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_HELPER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_FLD_HELPER].name,
            p_nat_session->helper);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SLOW_PATH_FWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_SLOW_PATH_FWD].name,
            p_nat_session->slow_path_fwd);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_CREATOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_CREATOR].name,
            p_nat_session->creator);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_FLD_SET_SAI_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_FLD_SET_SAI_STATUS].name,
            p_nat_session->set_sai_status);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nat_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nat_session_master_t *p_master = _g_p_tbl_nat_session_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nat_session_t *p_db_nat_session = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_list, p_db_nat_session, listnode, next)
    {
        rc |= fn(p_db_nat_session, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nat_session_master_t*
tbl_nat_session_get_master()
{
    return _g_p_tbl_nat_session_master;
}

tbl_nat_session_master_t*
tbl_nat_session_init_nat_session()
{
    _g_p_tbl_nat_session_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nat_session_master_t));
    _g_p_tbl_nat_session_master->nat_session_hash = ctclib_hash_create(_tbl_nat_session_hash_make, _tbl_nat_session_hash_cmp);
    _g_p_tbl_nat_session_master->nat_session_list = ctclib_slist_create(tbl_nat_session_cmp, NULL);
    return _g_p_tbl_nat_session_master;
}


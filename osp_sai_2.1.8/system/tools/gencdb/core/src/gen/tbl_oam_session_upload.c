
#include "proto.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"
#include "cdb_data_cmp.h"

tbl_oam_session_upload_master_t *_g_p_tbl_oam_session_upload_master = NULL;

static uint32
_tbl_oam_session_upload_hash_make(tbl_oam_session_upload_t *p_oam_session_upload)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_oam_session_upload->key;
    for (index = 0; index < sizeof(p_oam_session_upload->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_oam_session_upload_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_oam_session_upload_t *p_oam_session_upload1 = (tbl_oam_session_upload_t*)p_arg1;
    tbl_oam_session_upload_t *p_oam_session_upload2 = (tbl_oam_session_upload_t*)p_arg2;

    if (0 == sal_memcmp(&p_oam_session_upload1->key, &p_oam_session_upload2->key, sizeof(tbl_oam_session_upload_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_oam_session_upload_add_oam_session_upload_sync(tbl_oam_session_upload_t *p_oam_session_upload, uint32 sync)
{
    tbl_oam_session_upload_master_t *p_master = _g_p_tbl_oam_session_upload_master;
    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_oam_session_upload_get_oam_session_upload(&p_oam_session_upload->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_oam_session_upload = XCALLOC(MEM_TBL_OAM_SESSION_UPLOAD, sizeof(tbl_oam_session_upload_t));
    if (NULL == p_db_oam_session_upload)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_oam_session_upload, p_oam_session_upload, sizeof(tbl_oam_session_upload_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->oam_session_hash, (void*)p_db_oam_session_upload, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->oam_session_list, p_db_oam_session_upload);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OAM_SESSION_UPLOAD, p_db_oam_session_upload);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OAM_SESSION_UPLOAD, p_db_oam_session_upload);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_upload_del_oam_session_upload_sync(tbl_oam_session_upload_key_t *p_oam_session_upload_key, uint32 sync)
{
    tbl_oam_session_upload_master_t *p_master = _g_p_tbl_oam_session_upload_master;
    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;

    /* 1. lookup entry exist */
    p_db_oam_session_upload = tbl_oam_session_upload_get_oam_session_upload(p_oam_session_upload_key);
    if (NULL == p_db_oam_session_upload)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OAM_SESSION_UPLOAD, p_db_oam_session_upload);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OAM_SESSION_UPLOAD, p_db_oam_session_upload);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->oam_session_hash, (void*)p_db_oam_session_upload);
    ctclib_slistnode_delete(p_master->oam_session_list, p_db_oam_session_upload);

    /* 4. free db entry */
    XFREE(MEM_TBL_OAM_SESSION_UPLOAD, p_db_oam_session_upload);

    return PM_E_NONE;
}

int32
tbl_oam_session_upload_set_oam_session_upload_field_sync(tbl_oam_session_upload_t *p_oam_session_upload, tbl_oam_session_upload_field_id_t field_id, uint32 sync)
{
    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;

    /* 1. lookup entry exist */
    p_db_oam_session_upload = tbl_oam_session_upload_get_oam_session_upload(&p_oam_session_upload->key);
    if (NULL == p_db_oam_session_upload)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID:
        p_db_oam_session_upload->bind_oam_flow_id = p_oam_session_upload->bind_oam_flow_id;
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM:
        sal_memcpy(&p_db_oam_session_upload->upload_dm, &p_oam_session_upload->upload_dm, sizeof(p_oam_session_upload->upload_dm));
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT:
        sal_memcpy(&p_db_oam_session_upload->upload_dm_event, &p_oam_session_upload->upload_dm_event, sizeof(p_oam_session_upload->upload_dm_event));
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM:
        sal_memcpy(&p_db_oam_session_upload->upload_lm, &p_oam_session_upload->upload_lm, sizeof(p_oam_session_upload->upload_lm));
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT:
        sal_memcpy(&p_db_oam_session_upload->upload_lm_event, &p_oam_session_upload->upload_lm_event, sizeof(p_oam_session_upload->upload_lm_event));
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM:
        sal_memcpy(&p_db_oam_session_upload->upload_ccm, &p_oam_session_upload->upload_ccm, sizeof(p_oam_session_upload->upload_ccm));
        break;

    case TBL_OAM_SESSION_UPLOAD_FLD_MAX:
        sal_memcpy(p_db_oam_session_upload, p_oam_session_upload, sizeof(tbl_oam_session_upload_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OAM_SESSION_UPLOAD, field_id, p_db_oam_session_upload);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OAM_SESSION_UPLOAD, field_id, p_db_oam_session_upload);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_upload_add_oam_session_upload(tbl_oam_session_upload_t *p_oam_session_upload)
{
    return tbl_oam_session_upload_add_oam_session_upload_sync(p_oam_session_upload, TRUE);
}

int32
tbl_oam_session_upload_del_oam_session_upload(tbl_oam_session_upload_key_t *p_oam_session_upload_key)
{
    return tbl_oam_session_upload_del_oam_session_upload_sync(p_oam_session_upload_key, TRUE);
}

int32
tbl_oam_session_upload_set_oam_session_upload_field(tbl_oam_session_upload_t *p_oam_session_upload, tbl_oam_session_upload_field_id_t field_id)
{
    return tbl_oam_session_upload_set_oam_session_upload_field_sync(p_oam_session_upload, field_id, TRUE);
}

tbl_oam_session_upload_t*
tbl_oam_session_upload_get_oam_session_upload(tbl_oam_session_upload_key_t *p_oam_session_upload_key)
{
    tbl_oam_session_upload_master_t *p_master = _g_p_tbl_oam_session_upload_master;
    tbl_oam_session_upload_t lkp;

    sal_memcpy(&lkp.key, p_oam_session_upload_key, sizeof(tbl_oam_session_upload_key_t));
    return ctclib_hash_lookup(p_master->oam_session_hash, &lkp);
}

char*
tbl_oam_session_upload_key_val2str(tbl_oam_session_upload_t *p_oam_session_upload, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_oam_session_upload->key.session_id);
    return str;
}

int32
tbl_oam_session_upload_key_str2val(char *str, tbl_oam_session_upload_t *p_oam_session_upload)
{
    int32 ret = 0;

    p_oam_session_upload->key.session_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_oam_session_upload_key_name_dump(tbl_oam_session_upload_t *p_oam_session_upload, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION_UPLOAD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_oam_session_upload->key.session_id);
    return str;
}

char*
tbl_oam_session_upload_key_value_dump(tbl_oam_session_upload_t *p_oam_session_upload, char *str)
{
    sal_sprintf(str, "%u", p_oam_session_upload->key.session_id);
    return str;
}

char*
tbl_oam_session_upload_field_name_dump(tbl_oam_session_upload_t *p_oam_session_upload, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION_UPLOAD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OAM_SESSION_UPLOAD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_oam_session_upload_field_value_dump(tbl_oam_session_upload_t *p_oam_session_upload, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session_upload->key.session_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID, field_id))
    {
        sal_sprintf(str, "%u", p_oam_session_upload->bind_oam_flow_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM, field_id))
    {
        cdb_upload_tpoam_dm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_dm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT, field_id))
    {
        cdb_upload_tpoam_dm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_dm_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM, field_id))
    {
        cdb_upload_tpoam_lm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_lm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT, field_id))
    {
        cdb_upload_tpoam_lm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_lm_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM, field_id))
    {
        cdb_upload_tpoam_ccm_val2str(str, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_ccm);
    }
    return str;
}

char**
tbl_oam_session_upload_table_dump(tbl_oam_session_upload_t *p_oam_session_upload, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_oam_session_upload_key_name_dump(p_oam_session_upload, buf));
    for(i=1; i<TBL_OAM_SESSION_UPLOAD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_oam_session_upload_field_name_dump(p_oam_session_upload, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_oam_session_upload_field_value_dump(p_oam_session_upload, i, buf));
    }
    return str;
}

int32
tbl_oam_session_upload_field_value_parser(char *str, int32 field_id, tbl_oam_session_upload_t *p_oam_session_upload)
{
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_KEY, field_id))
    {
        int32 ret;
        p_oam_session_upload->key.session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID, field_id))
    {
        int32 ret;
        p_oam_session_upload->bind_oam_flow_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM, field_id))
    {
        cdb_upload_tpoam_dm_str2val(str, &p_oam_session_upload->upload_dm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT, field_id))
    {
        cdb_upload_tpoam_dm_str2val(str, &p_oam_session_upload->upload_dm_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM, field_id))
    {
        cdb_upload_tpoam_lm_str2val(str, &p_oam_session_upload->upload_lm);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT, field_id))
    {
        cdb_upload_tpoam_lm_str2val(str, &p_oam_session_upload->upload_lm_event);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM, field_id))
    {
        cdb_upload_tpoam_ccm_str2val(str, &p_oam_session_upload->upload_ccm);
    }
    return PM_E_NONE;
}

int32
tbl_oam_session_upload_table_parser(char** array, char* key_value,tbl_oam_session_upload_t *p_oam_session_upload)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION_UPLOAD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_oam_session_upload_key_str2val(key_value, p_oam_session_upload));

    for(i=1; i<TBL_OAM_SESSION_UPLOAD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_oam_session_upload_field_value_parser( array[j++], i, p_oam_session_upload));
    }

    return PM_E_NONE;
}

int32
tbl_oam_session_upload_dump_one(tbl_oam_session_upload_t *p_oam_session_upload, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OAM_SESSION_UPLOAD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_KEY].name,
            p_oam_session_upload->key.session_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID].name,
            p_oam_session_upload->bind_oam_flow_id);
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM].name, 
            cdb_upload_tpoam_dm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_dm));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT].name, 
            cdb_upload_tpoam_dm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_dm_event));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM].name, 
            cdb_upload_tpoam_lm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_lm));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT].name, 
            cdb_upload_tpoam_lm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_lm_event));
    }
    if (FLD_MATCH(TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM].name, 
            cdb_upload_tpoam_ccm_val2str(buf, MAX_CMD_STR_LEN, &p_oam_session_upload->upload_ccm));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_oam_session_upload_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_oam_session_upload_master_t *p_master = _g_p_tbl_oam_session_upload_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_oam_session_upload_t *p_db_oam_session_upload = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->oam_session_list, p_db_oam_session_upload, listnode, next)
    {
        rc |= fn(p_db_oam_session_upload, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_oam_session_upload_master_t*
tbl_oam_session_upload_get_master()
{
    return _g_p_tbl_oam_session_upload_master;
}

tbl_oam_session_upload_master_t*
tbl_oam_session_upload_init_oam_session_upload()
{
    _g_p_tbl_oam_session_upload_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_oam_session_upload_master_t));
    _g_p_tbl_oam_session_upload_master->oam_session_hash = ctclib_hash_create(_tbl_oam_session_upload_hash_make, _tbl_oam_session_upload_hash_cmp);
    _g_p_tbl_oam_session_upload_master->oam_session_list = ctclib_slist_create(tbl_oam_session_upload_cmp, NULL);
    return _g_p_tbl_oam_session_upload_master;
}


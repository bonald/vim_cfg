
#include "proto.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/ds_cfm_error_define.h"
#include "gen/ds_cfm_error.h"
#include "cdb_data_cmp.h"

tbl_cfm_md_master_t *_g_p_tbl_cfm_md_master = NULL;

static uint32
_tbl_cfm_md_hash_make(tbl_cfm_md_t *p_md)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_md->key;
    for (index = 0; index < sizeof(p_md->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cfm_md_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cfm_md_t *p_md1 = (tbl_cfm_md_t*)p_arg1;
    tbl_cfm_md_t *p_md2 = (tbl_cfm_md_t*)p_arg2;

    if (0 == sal_memcmp(&p_md1->key, &p_md2->key, sizeof(tbl_cfm_md_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cfm_md_add_cfm_md_sync(tbl_cfm_md_t *p_md, uint32 sync)
{
    tbl_cfm_md_master_t *p_master = _g_p_tbl_cfm_md_master;
    tbl_cfm_md_t *p_db_md = NULL;
    void *p_ret = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_cfm_md_get_cfm_md(&p_md->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_md = XCALLOC(MEM_TBL_CFM_MD, sizeof(tbl_cfm_md_t));
    if (NULL == p_db_md)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_md, p_md, sizeof(tbl_cfm_md_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_md->cfm_error, CDB_NODE_DS, DS_CFM_ERROR, ds_cfm_error_cmp));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_md_hash, (void*)p_db_md, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_md_list, p_db_md);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CFM_MD, p_db_md);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CFM_MD, p_db_md);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_md_del_cfm_md_sync(tbl_cfm_md_key_t *p_md_key, uint32 sync)
{
    tbl_cfm_md_master_t *p_master = _g_p_tbl_cfm_md_master;
    tbl_cfm_md_t *p_db_md = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(p_md_key);
    if (NULL == p_db_md)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CFM_MD, p_db_md);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CFM_MD, p_db_md);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_md_hash, (void*)p_db_md);
    ctclib_slistnode_delete(p_master->cfm_md_list, p_db_md);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_md->cfm_error));

    /* 4. free db entry */
    XFREE(MEM_TBL_CFM_MD, p_db_md);

    return PM_E_NONE;
}

int32
tbl_cfm_md_set_cfm_md_field_sync(tbl_cfm_md_t *p_md, tbl_cfm_md_field_id_t field_id, uint32 sync)
{
    tbl_cfm_md_t *p_db_md = NULL;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(&p_md->key);
    if (NULL == p_db_md)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_MD_FLD_LEVEL:
        p_db_md->level = p_md->level;
        break;

    case TBL_CFM_MD_FLD_NAME_TYPE:
        p_db_md->name_type = p_md->name_type;
        break;

    case TBL_CFM_MD_FLD_PKT_NAME:
        sal_memcpy(p_db_md->pkt_name, p_md->pkt_name, sizeof(p_md->pkt_name));
        break;

    case TBL_CFM_MD_FLD_PKT_NAME_LENGTH:
        p_db_md->pkt_name_length = p_md->pkt_name_length;
        break;

    case TBL_CFM_MD_FLD_PKT_NAME_INPUT:
        sal_memcpy(p_db_md->pkt_name_input, p_md->pkt_name_input, sizeof(p_md->pkt_name_input));
        break;

    case TBL_CFM_MD_FLD_SERVICE_NUMBER:
        p_db_md->service_number = p_md->service_number;
        break;

    case TBL_CFM_MD_FLD_MAX:
        sal_memcpy(p_db_md, p_md, sizeof(tbl_cfm_md_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_MD, field_id, p_db_md);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_MD, field_id, p_db_md);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_md_add_cfm_md(tbl_cfm_md_t *p_md)
{
    return tbl_cfm_md_add_cfm_md_sync(p_md, TRUE);
}

int32
tbl_cfm_md_del_cfm_md(tbl_cfm_md_key_t *p_md_key)
{
    return tbl_cfm_md_del_cfm_md_sync(p_md_key, TRUE);
}

int32
tbl_cfm_md_set_cfm_md_field(tbl_cfm_md_t *p_md, tbl_cfm_md_field_id_t field_id)
{
    return tbl_cfm_md_set_cfm_md_field_sync(p_md, field_id, TRUE);
}

tbl_cfm_md_t*
tbl_cfm_md_get_cfm_md(tbl_cfm_md_key_t *p_md_key)
{
    tbl_cfm_md_master_t *p_master = _g_p_tbl_cfm_md_master;
    tbl_cfm_md_t lkp;

    sal_memcpy(&lkp.key, p_md_key, sizeof(tbl_cfm_md_key_t));
    return ctclib_hash_lookup(p_master->cfm_md_hash, &lkp);
}

char*
tbl_cfm_md_key_val2str(tbl_cfm_md_t *p_md, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_md->key.name);
    return str;
}

int32
tbl_cfm_md_key_str2val(char *str, tbl_cfm_md_t *p_md)
{
    int32 ret = 0;

    sal_strcpy(p_md->key.name, str);
    return ret;
}

char*
tbl_cfm_md_key_name_dump(tbl_cfm_md_t *p_md, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_md->key.name);
    return str;
}

char*
tbl_cfm_md_key_value_dump(tbl_cfm_md_t *p_md, char *str)
{
    sal_sprintf(str, "%s", p_md->key.name);
    return str;
}

char*
tbl_cfm_md_field_name_dump(tbl_cfm_md_t *p_md, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_MD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_md_field_value_dump(tbl_cfm_md_t *p_md, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_MD_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_md->key.name);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_md->level);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_NAME_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_md->name_type);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_md->pkt_name);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_LENGTH, field_id))
    {
        sal_sprintf(str, "%u", p_md->pkt_name_length);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_INPUT, field_id))
    {
        sal_sprintf(str, "%s", p_md->pkt_name_input);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_SERVICE_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_md->service_number);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_CFM_ERROR, field_id))
    {
    }
    return str;
}

char**
tbl_cfm_md_table_dump(tbl_cfm_md_t *p_md, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_md_key_name_dump(p_md, buf));
    for(i=1; i<TBL_CFM_MD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_md_field_name_dump(p_md, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_md_field_value_dump(p_md, i, buf));
    }
    return str;
}

int32
tbl_cfm_md_field_value_parser(char *str, int32 field_id, tbl_cfm_md_t *p_md)
{
    if (FLD_MATCH(TBL_CFM_MD_FLD_KEY, field_id))
    {
        sal_strcpy(p_md->key.name, str);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_LEVEL, field_id))
    {
        int32 ret;
        p_md->level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_NAME_TYPE, field_id))
    {
        int32 ret;
        p_md->name_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME, field_id))
    {
        sal_strcpy(p_md->pkt_name, str);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_LENGTH, field_id))
    {
        int32 ret;
        p_md->pkt_name_length = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_INPUT, field_id))
    {
        sal_strcpy(p_md->pkt_name_input, str);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_SERVICE_NUMBER, field_id))
    {
        int32 ret;
        p_md->service_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_CFM_ERROR, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_cfm_md_table_parser(char** array, char* key_value,tbl_cfm_md_t *p_md)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_md_key_str2val(key_value, p_md));

    for(i=1; i<TBL_CFM_MD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_md_field_value_parser( array[j++], i, p_md));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_md_dump_one(tbl_cfm_md_t *p_md, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_cfm_error_t *p_db_cfm_error = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if (FLD_MATCH(TBL_CFM_MD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MD_FLD_KEY].name,
            p_md->key.name);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MD_FLD_LEVEL].name,
            p_md->level);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_NAME_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MD_FLD_NAME_TYPE].name,
            p_md->name_type);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MD_FLD_PKT_NAME].name,
            p_md->pkt_name);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MD_FLD_PKT_NAME_LENGTH].name,
            p_md->pkt_name_length);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_PKT_NAME_INPUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MD_FLD_PKT_NAME_INPUT].name,
            p_md->pkt_name_input);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_SERVICE_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MD_FLD_SERVICE_NUMBER].name,
            p_md->service_number);
    }
    if (FLD_MATCH(TBL_CFM_MD_FLD_CFM_ERROR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_CFM_MD_FLD_CFM_ERROR].name);
        if (p_md->cfm_error.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_md->cfm_error.obj_list, p_db_cfm_error, listnode)
            {
                ds_cfm_error_dump_one(p_db_cfm_error, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_md_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_md_master_t *p_master = _g_p_tbl_cfm_md_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cfm_md_t *p_db_md = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_md_list, p_db_md, listnode, next)
    {
        rc |= fn(p_db_md, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_md_master_t*
tbl_cfm_md_get_master()
{
    return _g_p_tbl_cfm_md_master;
}

tbl_cfm_md_master_t*
tbl_cfm_md_init_cfm_md()
{
    _g_p_tbl_cfm_md_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cfm_md_master_t));
    _g_p_tbl_cfm_md_master->cfm_md_hash = ctclib_hash_create(_tbl_cfm_md_hash_make, _tbl_cfm_md_hash_cmp);
    _g_p_tbl_cfm_md_master->cfm_md_list = ctclib_slist_create(tbl_cfm_md_cmp, NULL);
    return _g_p_tbl_cfm_md_master;
}


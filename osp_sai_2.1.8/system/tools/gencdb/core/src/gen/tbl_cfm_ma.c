
#include "proto.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "cdb_data_cmp.h"

tbl_cfm_ma_master_t *_g_p_tbl_cfm_ma_master = NULL;

static uint32
_tbl_cfm_ma_hash_make(tbl_cfm_ma_t *p_ma)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ma->key;
    for (index = 0; index < sizeof(p_ma->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cfm_ma_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cfm_ma_t *p_ma1 = (tbl_cfm_ma_t*)p_arg1;
    tbl_cfm_ma_t *p_ma2 = (tbl_cfm_ma_t*)p_arg2;

    if (0 == sal_memcmp(&p_ma1->key, &p_ma2->key, sizeof(tbl_cfm_ma_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cfm_ma_add_cfm_ma_sync(tbl_cfm_ma_t *p_ma, uint32 sync)
{
    tbl_cfm_ma_master_t *p_master = _g_p_tbl_cfm_ma_master;
    tbl_cfm_ma_t *p_db_ma = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cfm_ma_get_cfm_ma(&p_ma->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ma = XCALLOC(MEM_TBL_CFM_MA, sizeof(tbl_cfm_ma_t));
    if (NULL == p_db_ma)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ma, p_ma, sizeof(tbl_cfm_ma_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_ma_hash, (void*)p_db_ma, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_ma_list, p_db_ma);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CFM_MA, p_db_ma);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CFM_MA, p_db_ma);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_ma_del_cfm_ma_sync(tbl_cfm_ma_key_t *p_ma_key, uint32 sync)
{
    tbl_cfm_ma_master_t *p_master = _g_p_tbl_cfm_ma_master;
    tbl_cfm_ma_t *p_db_ma = NULL;

    /* 1. lookup entry exist */
    p_db_ma = tbl_cfm_ma_get_cfm_ma(p_ma_key);
    if (NULL == p_db_ma)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CFM_MA, p_db_ma);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CFM_MA, p_db_ma);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_ma_hash, (void*)p_db_ma);
    ctclib_slistnode_delete(p_master->cfm_ma_list, p_db_ma);

    /* 4. free db entry */
    XFREE(MEM_TBL_CFM_MA, p_db_ma);

    return PM_E_NONE;
}

int32
tbl_cfm_ma_set_cfm_ma_field_sync(tbl_cfm_ma_t *p_ma, tbl_cfm_ma_field_id_t field_id, uint32 sync)
{
    tbl_cfm_ma_t *p_db_ma = NULL;

    /* 1. lookup entry exist */
    p_db_ma = tbl_cfm_ma_get_cfm_ma(&p_ma->key);
    if (NULL == p_db_ma)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_MA_FLD_NAME:
        sal_memcpy(p_db_ma->name, p_ma->name, sizeof(p_ma->name));
        break;

    case TBL_CFM_MA_FLD_NAME_TYPE:
        p_db_ma->name_type = p_ma->name_type;
        break;

    case TBL_CFM_MA_FLD_CCI_ENABLED:
        p_db_ma->cci_enabled = p_ma->cci_enabled;
        break;

    case TBL_CFM_MA_FLD_CCM_VLAN_COS:
        p_db_ma->ccm_vlan_cos = p_ma->ccm_vlan_cos;
        break;

    case TBL_CFM_MA_FLD_PKT_NAME:
        sal_memcpy(p_db_ma->pkt_name, p_ma->pkt_name, sizeof(p_ma->pkt_name));
        break;

    case TBL_CFM_MA_FLD_MAID:
        sal_memcpy(p_db_ma->maid, p_ma->maid, sizeof(p_ma->maid));
        break;

    case TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE:
        p_db_ma->error_write_disable = p_ma->error_write_disable;
        break;

    case TBL_CFM_MA_FLD_ERROR_NUMBER:
        p_db_ma->error_number = p_ma->error_number;
        break;

    case TBL_CFM_MA_FLD_MAX:
        sal_memcpy(p_db_ma, p_ma, sizeof(tbl_cfm_ma_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_MA, field_id, p_db_ma);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_MA, field_id, p_db_ma);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_ma_add_cfm_ma(tbl_cfm_ma_t *p_ma)
{
    return tbl_cfm_ma_add_cfm_ma_sync(p_ma, TRUE);
}

int32
tbl_cfm_ma_del_cfm_ma(tbl_cfm_ma_key_t *p_ma_key)
{
    return tbl_cfm_ma_del_cfm_ma_sync(p_ma_key, TRUE);
}

int32
tbl_cfm_ma_set_cfm_ma_field(tbl_cfm_ma_t *p_ma, tbl_cfm_ma_field_id_t field_id)
{
    return tbl_cfm_ma_set_cfm_ma_field_sync(p_ma, field_id, TRUE);
}

tbl_cfm_ma_t*
tbl_cfm_ma_get_cfm_ma(tbl_cfm_ma_key_t *p_ma_key)
{
    tbl_cfm_ma_master_t *p_master = _g_p_tbl_cfm_ma_master;
    tbl_cfm_ma_t lkp;

    sal_memcpy(&lkp.key, p_ma_key, sizeof(tbl_cfm_ma_key_t));
    return ctclib_hash_lookup(p_master->cfm_ma_hash, &lkp);
}

char*
tbl_cfm_ma_key_val2str(tbl_cfm_ma_t *p_ma, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_cfm_ma_key_val2str(buf, MAX_CMD_STR_LEN, &p_ma->key));
    return str;
}

int32
tbl_cfm_ma_key_str2val(char *str, tbl_cfm_ma_t *p_ma)
{
    int32 ret = 0;

    ret = cdb_cfm_ma_key_str2val(str, &p_ma->key);
    return ret;
}

char*
tbl_cfm_ma_key_name_dump(tbl_cfm_ma_t *p_ma, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MA);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_cfm_ma_key_val2str(buf, MAX_CMD_STR_LEN, &p_ma->key));
    return str;
}

char*
tbl_cfm_ma_key_value_dump(tbl_cfm_ma_t *p_ma, char *str)
{

    cdb_cfm_ma_key_val2str(str, MAX_CMD_STR_LEN, &p_ma->key);
    return str;
}

char*
tbl_cfm_ma_field_name_dump(tbl_cfm_ma_t *p_ma, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MA);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_MA_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_ma_field_value_dump(tbl_cfm_ma_t *p_ma, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_MA_FLD_KEY, field_id))
    {
        cdb_cfm_ma_key_val2str(str, MAX_CMD_STR_LEN, &p_ma->key);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_ma->name);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ma->name_type);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCI_ENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_ma->cci_enabled);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCM_VLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_ma->ccm_vlan_cos);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_PKT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_ma->pkt_name);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_MAID, field_id))
    {
        sal_sprintf(str, "%s", p_ma->maid);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ma->error_write_disable);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_ma->error_number);
    }
    return str;
}

char**
tbl_cfm_ma_table_dump(tbl_cfm_ma_t *p_ma, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_ma_key_name_dump(p_ma, buf));
    for(i=1; i<TBL_CFM_MA_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_ma_field_name_dump(p_ma, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_ma_field_value_dump(p_ma, i, buf));
    }
    return str;
}

int32
tbl_cfm_ma_field_value_parser(char *str, int32 field_id, tbl_cfm_ma_t *p_ma)
{
    if (FLD_MATCH(TBL_CFM_MA_FLD_KEY, field_id))
    {
        cdb_cfm_ma_key_str2val(str, &p_ma->key);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME, field_id))
    {
        sal_strcpy(p_ma->name, str);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME_TYPE, field_id))
    {
        int32 ret;
        p_ma->name_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCI_ENABLED, field_id))
    {
        int32 ret;
        p_ma->cci_enabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCM_VLAN_COS, field_id))
    {
        int32 ret;
        p_ma->ccm_vlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_PKT_NAME, field_id))
    {
        sal_strcpy(p_ma->pkt_name, str);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_MAID, field_id))
    {
        sal_strcpy(p_ma->maid, str);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE, field_id))
    {
        int32 ret;
        p_ma->error_write_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_NUMBER, field_id))
    {
        int32 ret;
        p_ma->error_number = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cfm_ma_table_parser(char** array, char* key_value,tbl_cfm_ma_t *p_ma)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MA);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_ma_key_str2val(key_value, p_ma));

    for(i=1; i<TBL_CFM_MA_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_ma_field_value_parser( array[j++], i, p_ma));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_ma_dump_one(tbl_cfm_ma_t *p_ma, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MA);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CFM_MA_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MA_FLD_KEY].name, 
            cdb_cfm_ma_key_val2str(buf, MAX_CMD_STR_LEN, &p_ma->key));
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MA_FLD_NAME].name,
            p_ma->name);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_NAME_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MA_FLD_NAME_TYPE].name,
            p_ma->name_type);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCI_ENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MA_FLD_CCI_ENABLED].name,
            p_ma->cci_enabled);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_CCM_VLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MA_FLD_CCM_VLAN_COS].name,
            p_ma->ccm_vlan_cos);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_PKT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MA_FLD_PKT_NAME].name,
            p_ma->pkt_name);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_MAID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MA_FLD_MAID].name,
            p_ma->maid);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE].name,
            p_ma->error_write_disable);
    }
    if (FLD_MATCH(TBL_CFM_MA_FLD_ERROR_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_MA_FLD_ERROR_NUMBER].name,
            p_ma->error_number);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_ma_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_ma_master_t *p_master = _g_p_tbl_cfm_ma_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cfm_ma_t *p_db_ma = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_ma_list, p_db_ma, listnode, next)
    {
        rc |= fn(p_db_ma, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_ma_master_t*
tbl_cfm_ma_get_master()
{
    return _g_p_tbl_cfm_ma_master;
}

tbl_cfm_ma_master_t*
tbl_cfm_ma_init_cfm_ma()
{
    _g_p_tbl_cfm_ma_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cfm_ma_master_t));
    _g_p_tbl_cfm_ma_master->cfm_ma_hash = ctclib_hash_create(_tbl_cfm_ma_hash_make, _tbl_cfm_ma_hash_cmp);
    _g_p_tbl_cfm_ma_master->cfm_ma_list = ctclib_slist_create(tbl_cfm_ma_cmp, NULL);
    return _g_p_tbl_cfm_ma_master;
}



#include "proto.h"
#include "gen/tbl_fea_ptp_ts_define.h"
#include "gen/tbl_fea_ptp_ts.h"
#include "cdb_data_cmp.h"

tbl_fea_ptp_ts_master_t *_g_p_tbl_fea_ptp_ts_master = NULL;

static uint32
_tbl_fea_ptp_ts_hash_make(tbl_fea_ptp_ts_t *p_fea_ptp_ts)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fea_ptp_ts->key;
    for (index = 0; index < sizeof(p_fea_ptp_ts->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_ptp_ts_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_ptp_ts_t *p_fea_ptp_ts1 = (tbl_fea_ptp_ts_t*)p_arg1;
    tbl_fea_ptp_ts_t *p_fea_ptp_ts2 = (tbl_fea_ptp_ts_t*)p_arg2;

    if (0 == sal_memcmp(&p_fea_ptp_ts1->key, &p_fea_ptp_ts2->key, sizeof(tbl_fea_ptp_ts_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_ptp_ts_add_fea_ptp_ts_sync(tbl_fea_ptp_ts_t *p_fea_ptp_ts, uint32 sync)
{
    tbl_fea_ptp_ts_master_t *p_master = _g_p_tbl_fea_ptp_ts_master;
    tbl_fea_ptp_ts_t *p_db_fea_ptp_ts = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_ptp_ts_get_fea_ptp_ts(&p_fea_ptp_ts->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_ptp_ts = XCALLOC(MEM_TBL_FEA_PTP_TS, sizeof(tbl_fea_ptp_ts_t));
    if (NULL == p_db_fea_ptp_ts)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_ptp_ts, p_fea_ptp_ts, sizeof(tbl_fea_ptp_ts_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fea_ptp_ts_hash, (void*)p_db_fea_ptp_ts, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->fea_ptp_ts_list, p_db_fea_ptp_ts);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_PTP_TS, p_db_fea_ptp_ts);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_PTP_TS, p_db_fea_ptp_ts);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_del_fea_ptp_ts_sync(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key, uint32 sync)
{
    tbl_fea_ptp_ts_master_t *p_master = _g_p_tbl_fea_ptp_ts_master;
    tbl_fea_ptp_ts_t *p_db_fea_ptp_ts = NULL;

    /* 1. lookup entry exist */
    p_db_fea_ptp_ts = tbl_fea_ptp_ts_get_fea_ptp_ts(p_fea_ptp_ts_key);
    if (NULL == p_db_fea_ptp_ts)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_PTP_TS, p_db_fea_ptp_ts);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_PTP_TS, p_db_fea_ptp_ts);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fea_ptp_ts_hash, (void*)p_db_fea_ptp_ts);
    ctclib_slistnode_delete(p_master->fea_ptp_ts_list, p_db_fea_ptp_ts);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_PTP_TS, p_db_fea_ptp_ts);

    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_set_fea_ptp_ts_field_sync(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_fea_ptp_ts_field_id_t field_id, uint32 sync)
{
    tbl_fea_ptp_ts_t *p_db_fea_ptp_ts = NULL;

    /* 1. lookup entry exist */
    p_db_fea_ptp_ts = tbl_fea_ptp_ts_get_fea_ptp_ts(&p_fea_ptp_ts->key);
    if (NULL == p_db_fea_ptp_ts)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_PTP_TS_FLD_INTR_IFINDEX:
        p_db_fea_ptp_ts->intr_ifindex = p_fea_ptp_ts->intr_ifindex;
        break;

    case TBL_FEA_PTP_TS_FLD_NS:
        p_db_fea_ptp_ts->ns = p_fea_ptp_ts->ns;
        break;

    case TBL_FEA_PTP_TS_FLD_S_LSB:
        p_db_fea_ptp_ts->s_lsb = p_fea_ptp_ts->s_lsb;
        break;

    case TBL_FEA_PTP_TS_FLD_S_MSB:
        p_db_fea_ptp_ts->s_msb = p_fea_ptp_ts->s_msb;
        break;

    case TBL_FEA_PTP_TS_FLD_MAX:
        sal_memcpy(p_db_fea_ptp_ts, p_fea_ptp_ts, sizeof(tbl_fea_ptp_ts_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_PTP_TS, field_id, p_db_fea_ptp_ts);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_PTP_TS, field_id, p_db_fea_ptp_ts);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_add_fea_ptp_ts(tbl_fea_ptp_ts_t *p_fea_ptp_ts)
{
    return tbl_fea_ptp_ts_add_fea_ptp_ts_sync(p_fea_ptp_ts, TRUE);
}

int32
tbl_fea_ptp_ts_del_fea_ptp_ts(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key)
{
    return tbl_fea_ptp_ts_del_fea_ptp_ts_sync(p_fea_ptp_ts_key, TRUE);
}

int32
tbl_fea_ptp_ts_set_fea_ptp_ts_field(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_fea_ptp_ts_field_id_t field_id)
{
    return tbl_fea_ptp_ts_set_fea_ptp_ts_field_sync(p_fea_ptp_ts, field_id, TRUE);
}

tbl_fea_ptp_ts_t*
tbl_fea_ptp_ts_get_fea_ptp_ts(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key)
{
    tbl_fea_ptp_ts_master_t *p_master = _g_p_tbl_fea_ptp_ts_master;
    tbl_fea_ptp_ts_t lkp;

    sal_memcpy(&lkp.key, p_fea_ptp_ts_key, sizeof(tbl_fea_ptp_ts_key_t));
    return ctclib_hash_lookup(p_master->fea_ptp_ts_hash, &lkp);
}

char*
tbl_fea_ptp_ts_key_val2str(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_fea_ptp_ts->key.intr_seq_id);
    return str;
}

int32
tbl_fea_ptp_ts_key_str2val(char *str, tbl_fea_ptp_ts_t *p_fea_ptp_ts)
{
    int32 ret = 0;

    p_fea_ptp_ts->key.intr_seq_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_ptp_ts_key_name_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_TS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_fea_ptp_ts->key.intr_seq_id);
    return str;
}

char*
tbl_fea_ptp_ts_key_value_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char *str)
{
    sal_sprintf(str, "%u", p_fea_ptp_ts->key.intr_seq_id);
    return str;
}

char*
tbl_fea_ptp_ts_field_name_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_TS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_PTP_TS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_ptp_ts_field_value_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_ts->key.intr_seq_id);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_INTR_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_ts->intr_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_NS, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_ts->ns);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_LSB, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_ts->s_lsb);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_MSB, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_ts->s_msb);
    }
    return str;
}

char**
tbl_fea_ptp_ts_table_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_ptp_ts_key_name_dump(p_fea_ptp_ts, buf));
    for(i=1; i<TBL_FEA_PTP_TS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_ptp_ts_field_name_dump(p_fea_ptp_ts, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_ptp_ts_field_value_dump(p_fea_ptp_ts, i, buf));
    }
    return str;
}

int32
tbl_fea_ptp_ts_field_value_parser(char *str, int32 field_id, tbl_fea_ptp_ts_t *p_fea_ptp_ts)
{
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_ptp_ts->key.intr_seq_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_INTR_IFINDEX, field_id))
    {
        int32 ret;
        p_fea_ptp_ts->intr_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_NS, field_id))
    {
        int32 ret;
        p_fea_ptp_ts->ns = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_LSB, field_id))
    {
        int32 ret;
        p_fea_ptp_ts->s_lsb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_MSB, field_id))
    {
        int32 ret;
        p_fea_ptp_ts->s_msb = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_table_parser(char** array, char* key_value,tbl_fea_ptp_ts_t *p_fea_ptp_ts)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_TS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_ptp_ts_key_str2val(key_value, p_fea_ptp_ts));

    for(i=1; i<TBL_FEA_PTP_TS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_ptp_ts_field_value_parser( array[j++], i, p_fea_ptp_ts));
    }

    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_dump_one(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_TS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_TS_FLD_KEY].name,
            p_fea_ptp_ts->key.intr_seq_id);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_INTR_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_TS_FLD_INTR_IFINDEX].name,
            p_fea_ptp_ts->intr_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_NS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_TS_FLD_NS].name,
            p_fea_ptp_ts->ns);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_LSB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_TS_FLD_S_LSB].name,
            p_fea_ptp_ts->s_lsb);
    }
    if (FLD_MATCH(TBL_FEA_PTP_TS_FLD_S_MSB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_TS_FLD_S_MSB].name,
            p_fea_ptp_ts->s_msb);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_ptp_ts_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_ptp_ts_master_t *p_master = _g_p_tbl_fea_ptp_ts_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fea_ptp_ts_t *p_db_fea_ptp_ts = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->fea_ptp_ts_list, p_db_fea_ptp_ts, listnode, next)
    {
        rc |= fn(p_db_fea_ptp_ts, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_ptp_ts_master_t*
tbl_fea_ptp_ts_get_master()
{
    return _g_p_tbl_fea_ptp_ts_master;
}

tbl_fea_ptp_ts_master_t*
tbl_fea_ptp_ts_init_fea_ptp_ts()
{
    _g_p_tbl_fea_ptp_ts_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_ptp_ts_master_t));
    _g_p_tbl_fea_ptp_ts_master->fea_ptp_ts_hash = ctclib_hash_create(_tbl_fea_ptp_ts_hash_make, _tbl_fea_ptp_ts_hash_cmp);
    _g_p_tbl_fea_ptp_ts_master->fea_ptp_ts_list = ctclib_slist_create(tbl_fea_ptp_ts_cmp, NULL);
    return _g_p_tbl_fea_ptp_ts_master;
}


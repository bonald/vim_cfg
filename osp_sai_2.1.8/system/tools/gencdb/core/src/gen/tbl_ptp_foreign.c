
#include "proto.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"
#include "cdb_data_cmp.h"

tbl_ptp_foreign_master_t *_g_p_tbl_ptp_foreign_master = NULL;

static uint32
_tbl_ptp_foreign_hash_make(tbl_ptp_foreign_t *p_foreign)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_foreign->key;
    for (index = 0; index < sizeof(p_foreign->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ptp_foreign_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ptp_foreign_t *p_foreign1 = (tbl_ptp_foreign_t*)p_arg1;
    tbl_ptp_foreign_t *p_foreign2 = (tbl_ptp_foreign_t*)p_arg2;

    if (0 == sal_memcmp(&p_foreign1->key, &p_foreign2->key, sizeof(tbl_ptp_foreign_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ptp_foreign_add_ptp_foreign_sync(tbl_ptp_foreign_t *p_foreign, uint32 sync)
{
    tbl_ptp_foreign_master_t *p_master = _g_p_tbl_ptp_foreign_master;
    tbl_ptp_foreign_t *p_db_foreign = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ptp_foreign_get_ptp_foreign(&p_foreign->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_foreign = XCALLOC(MEM_TBL_PTP_FOREIGN, sizeof(tbl_ptp_foreign_t));
    if (NULL == p_db_foreign)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_foreign, p_foreign, sizeof(tbl_ptp_foreign_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ptp_foreign_hash, (void*)p_db_foreign, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ptp_foreign_list, p_db_foreign);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PTP_FOREIGN, p_db_foreign);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PTP_FOREIGN, p_db_foreign);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_foreign_del_ptp_foreign_sync(tbl_ptp_foreign_key_t *p_foreign_key, uint32 sync)
{
    tbl_ptp_foreign_master_t *p_master = _g_p_tbl_ptp_foreign_master;
    tbl_ptp_foreign_t *p_db_foreign = NULL;

    /* 1. lookup entry exist */
    p_db_foreign = tbl_ptp_foreign_get_ptp_foreign(p_foreign_key);
    if (NULL == p_db_foreign)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PTP_FOREIGN, p_db_foreign);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PTP_FOREIGN, p_db_foreign);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ptp_foreign_hash, (void*)p_db_foreign);
    ctclib_slistnode_delete(p_master->ptp_foreign_list, p_db_foreign);

    /* 4. free db entry */
    XFREE(MEM_TBL_PTP_FOREIGN, p_db_foreign);

    return PM_E_NONE;
}

int32
tbl_ptp_foreign_set_ptp_foreign_field_sync(tbl_ptp_foreign_t *p_foreign, tbl_ptp_foreign_field_id_t field_id, uint32 sync)
{
    tbl_ptp_foreign_t *p_db_foreign = NULL;

    /* 1. lookup entry exist */
    p_db_foreign = tbl_ptp_foreign_get_ptp_foreign(&p_foreign->key);
    if (NULL == p_db_foreign)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES:
        p_db_foreign->foreign_master_announce_messages = p_foreign->foreign_master_announce_messages;
        break;

    case TBL_PTP_FOREIGN_FLD_HEADER:
        sal_memcpy(&p_db_foreign->header, &p_foreign->header, sizeof(p_foreign->header));
        break;

    case TBL_PTP_FOREIGN_FLD_ANNOUNCE:
        sal_memcpy(&p_db_foreign->announce, &p_foreign->announce, sizeof(p_foreign->announce));
        break;

    case TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER:
        p_db_foreign->ptm_fm_qualification_timer = p_foreign->ptm_fm_qualification_timer;
        break;

    case TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER:
        p_db_foreign->ptm_fm_announce_receipt_timer = p_foreign->ptm_fm_announce_receipt_timer;
        break;

    case TBL_PTP_FOREIGN_FLD_QUALIFIED:
        p_db_foreign->qualified = p_foreign->qualified;
        break;

    case TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM:
        p_db_foreign->port_foreign_num = p_foreign->port_foreign_num;
        break;

    case TBL_PTP_FOREIGN_FLD_MAX:
        sal_memcpy(p_db_foreign, p_foreign, sizeof(tbl_ptp_foreign_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PTP_FOREIGN, field_id, p_db_foreign);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PTP_FOREIGN, field_id, p_db_foreign);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_foreign_add_ptp_foreign(tbl_ptp_foreign_t *p_foreign)
{
    return tbl_ptp_foreign_add_ptp_foreign_sync(p_foreign, TRUE);
}

int32
tbl_ptp_foreign_del_ptp_foreign(tbl_ptp_foreign_key_t *p_foreign_key)
{
    return tbl_ptp_foreign_del_ptp_foreign_sync(p_foreign_key, TRUE);
}

int32
tbl_ptp_foreign_set_ptp_foreign_field(tbl_ptp_foreign_t *p_foreign, tbl_ptp_foreign_field_id_t field_id)
{
    return tbl_ptp_foreign_set_ptp_foreign_field_sync(p_foreign, field_id, TRUE);
}

tbl_ptp_foreign_t*
tbl_ptp_foreign_get_ptp_foreign(tbl_ptp_foreign_key_t *p_foreign_key)
{
    tbl_ptp_foreign_master_t *p_master = _g_p_tbl_ptp_foreign_master;
    tbl_ptp_foreign_t lkp;

    sal_memcpy(&lkp.key, p_foreign_key, sizeof(tbl_ptp_foreign_key_t));
    return ctclib_hash_lookup(p_master->ptp_foreign_hash, &lkp);
}

char*
tbl_ptp_foreign_key_val2str(tbl_ptp_foreign_t *p_foreign, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ptp_foreign_master_val2str(buf, MAX_CMD_STR_LEN, &p_foreign->key));
    return str;
}

int32
tbl_ptp_foreign_key_str2val(char *str, tbl_ptp_foreign_t *p_foreign)
{
    int32 ret = 0;

    ret = cdb_ptp_foreign_master_str2val(str, &p_foreign->key);
    return ret;
}

char*
tbl_ptp_foreign_key_name_dump(tbl_ptp_foreign_t *p_foreign, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_FOREIGN);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ptp_foreign_master_val2str(buf, MAX_CMD_STR_LEN, &p_foreign->key));
    return str;
}

char*
tbl_ptp_foreign_key_value_dump(tbl_ptp_foreign_t *p_foreign, char *str)
{

    cdb_ptp_foreign_master_val2str(str, MAX_CMD_STR_LEN, &p_foreign->key);
    return str;
}

char*
tbl_ptp_foreign_field_name_dump(tbl_ptp_foreign_t *p_foreign, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_FOREIGN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PTP_FOREIGN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ptp_foreign_field_value_dump(tbl_ptp_foreign_t *p_foreign, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_KEY, field_id))
    {
        cdb_ptp_foreign_master_val2str(str, MAX_CMD_STR_LEN, &p_foreign->key);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES, field_id))
    {
        sal_sprintf(str, "%u", p_foreign->foreign_master_announce_messages);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_HEADER, field_id))
    {
        cdb_ptp_common_message_header_val2str(str, MAX_CMD_STR_LEN, &p_foreign->header);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_ANNOUNCE, field_id))
    {
        cdb_ptp_msg_announce_val2str(str, MAX_CMD_STR_LEN, &p_foreign->announce);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_foreign->ptm_fm_qualification_timer);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_foreign->ptm_fm_announce_receipt_timer);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_QUALIFIED, field_id))
    {
        sal_sprintf(str, "%u", p_foreign->qualified);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_foreign->port_foreign_num);
    }
    return str;
}

char**
tbl_ptp_foreign_table_dump(tbl_ptp_foreign_t *p_foreign, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ptp_foreign_key_name_dump(p_foreign, buf));
    for(i=1; i<TBL_PTP_FOREIGN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ptp_foreign_field_name_dump(p_foreign, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ptp_foreign_field_value_dump(p_foreign, i, buf));
    }
    return str;
}

int32
tbl_ptp_foreign_field_value_parser(char *str, int32 field_id, tbl_ptp_foreign_t *p_foreign)
{
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_KEY, field_id))
    {
        cdb_ptp_foreign_master_str2val(str, &p_foreign->key);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES, field_id))
    {
        int32 ret;
        p_foreign->foreign_master_announce_messages = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_HEADER, field_id))
    {
        cdb_ptp_common_message_header_str2val(str, &p_foreign->header);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_ANNOUNCE, field_id))
    {
        cdb_ptp_msg_announce_str2val(str, &p_foreign->announce);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_foreign->ptm_fm_qualification_timer);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_foreign->ptm_fm_announce_receipt_timer);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_QUALIFIED, field_id))
    {
        int32 ret;
        p_foreign->qualified = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM, field_id))
    {
        int32 ret;
        p_foreign->port_foreign_num = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ptp_foreign_table_parser(char** array, char* key_value,tbl_ptp_foreign_t *p_foreign)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_FOREIGN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ptp_foreign_key_str2val(key_value, p_foreign));

    for(i=1; i<TBL_PTP_FOREIGN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ptp_foreign_field_value_parser( array[j++], i, p_foreign));
    }

    return PM_E_NONE;
}

int32
tbl_ptp_foreign_dump_one(tbl_ptp_foreign_t *p_foreign, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_FOREIGN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_KEY].name, 
            cdb_ptp_foreign_master_val2str(buf, MAX_CMD_STR_LEN, &p_foreign->key));
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES].name,
            p_foreign->foreign_master_announce_messages);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_HEADER].name, 
            cdb_ptp_common_message_header_val2str(buf, MAX_CMD_STR_LEN, &p_foreign->header));
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_ANNOUNCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_ANNOUNCE].name, 
            cdb_ptp_msg_announce_val2str(buf, MAX_CMD_STR_LEN, &p_foreign->announce));
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_foreign->ptm_fm_qualification_timer));
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_foreign->ptm_fm_announce_receipt_timer));
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_QUALIFIED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_QUALIFIED].name,
            p_foreign->qualified);
    }
    if (FLD_MATCH(TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM].name,
            p_foreign->port_foreign_num);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ptp_foreign_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ptp_foreign_master_t *p_master = _g_p_tbl_ptp_foreign_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ptp_foreign_t *p_db_foreign = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ptp_foreign_list, p_db_foreign, listnode, next)
    {
        rc |= fn(p_db_foreign, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ptp_foreign_master_t*
tbl_ptp_foreign_get_master()
{
    return _g_p_tbl_ptp_foreign_master;
}

tbl_ptp_foreign_master_t*
tbl_ptp_foreign_init_ptp_foreign()
{
    _g_p_tbl_ptp_foreign_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ptp_foreign_master_t));
    _g_p_tbl_ptp_foreign_master->ptp_foreign_hash = ctclib_hash_create(_tbl_ptp_foreign_hash_make, _tbl_ptp_foreign_hash_cmp);
    _g_p_tbl_ptp_foreign_master->ptp_foreign_list = ctclib_slist_create(tbl_ptp_foreign_cmp, NULL);
    return _g_p_tbl_ptp_foreign_master;
}


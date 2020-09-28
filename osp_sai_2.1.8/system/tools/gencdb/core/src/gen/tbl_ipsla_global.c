
#include "proto.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "cdb_data_cmp.h"

tbl_ipsla_global_t *_g_p_tbl_ipsla_global = NULL;

int32
tbl_ipsla_global_set_ipsla_global_field_sync(tbl_ipsla_global_t *p_ipsla_glb, tbl_ipsla_global_field_id_t field_id, uint32 sync)
{
    tbl_ipsla_global_t *p_db_ipsla_glb = _g_p_tbl_ipsla_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM:
        p_db_ipsla_glb->ipsla_entry_num = p_ipsla_glb->ipsla_entry_num;
        break;

    case TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER:
        p_db_ipsla_glb->t_ipsla_timer = p_ipsla_glb->t_ipsla_timer;
        break;

    case TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE:
        p_db_ipsla_glb->trap_test_failure = p_ipsla_glb->trap_test_failure;
        break;

    case TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS:
        p_db_ipsla_glb->trap_test_success = p_ipsla_glb->trap_test_success;
        break;

    case TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT:
        p_db_ipsla_glb->trap_pkt_timeout = p_ipsla_glb->trap_pkt_timeout;
        break;

    case TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD:
        p_db_ipsla_glb->trap_overthreshold = p_ipsla_glb->trap_overthreshold;
        break;

    case TBL_IPSLA_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_ipsla_glb, p_ipsla_glb, sizeof(tbl_ipsla_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSLA_GLOBAL, field_id, p_db_ipsla_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPSLA_GLOBAL, field_id, p_db_ipsla_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_global_set_ipsla_global_field(tbl_ipsla_global_t *p_ipsla_glb, tbl_ipsla_global_field_id_t field_id)
{
    return tbl_ipsla_global_set_ipsla_global_field_sync(p_ipsla_glb, field_id, TRUE);
}

tbl_ipsla_global_t*
tbl_ipsla_global_get_ipsla_global()
{
    return _g_p_tbl_ipsla_global;
}

int32
tbl_ipsla_global_key_str2val(char *str, tbl_ipsla_global_t *p_ipsla_glb)
{
    return PM_E_NONE;
}

char*
tbl_ipsla_global_key_name_dump(tbl_ipsla_global_t *p_ipsla_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ipsla_global_key_value_dump(tbl_ipsla_global_t *p_ipsla_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ipsla_global_field_name_dump(tbl_ipsla_global_t *p_ipsla_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPSLA_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipsla_global_field_value_dump(tbl_ipsla_global_t *p_ipsla_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_glb->ipsla_entry_num);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ipsla_glb->t_ipsla_timer);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_glb->trap_test_failure);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_glb->trap_test_success);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_glb->trap_pkt_timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_ipsla_glb->trap_overthreshold);
    }
    return str;
}

char**
tbl_ipsla_global_table_dump(tbl_ipsla_global_t *p_ipsla_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipsla_global_key_name_dump(p_ipsla_glb, buf));
    for(i=1; i<TBL_IPSLA_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_global_field_name_dump(p_ipsla_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipsla_global_field_value_dump(p_ipsla_glb, i, buf));
    }
    return str;
}

int32
tbl_ipsla_global_field_value_parser(char *str, int32 field_id, tbl_ipsla_global_t *p_ipsla_glb)
{
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_ipsla_glb->ipsla_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ipsla_glb->t_ipsla_timer);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE, field_id))
    {
        int32 ret;
        p_ipsla_glb->trap_test_failure = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS, field_id))
    {
        int32 ret;
        p_ipsla_glb->trap_test_success = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT, field_id))
    {
        int32 ret;
        p_ipsla_glb->trap_pkt_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD, field_id))
    {
        int32 ret;
        p_ipsla_glb->trap_overthreshold = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipsla_global_table_parser(char** array, char* key_value,tbl_ipsla_global_t *p_ipsla_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipsla_global_key_str2val(key_value, p_ipsla_glb));

    for(i=1; i<TBL_IPSLA_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipsla_global_field_value_parser( array[j++], i, p_ipsla_glb));
    }

    return PM_E_NONE;
}

int32
tbl_ipsla_global_dump_one(tbl_ipsla_global_t *p_ipsla_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSLA_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM].name,
            p_ipsla_glb->ipsla_entry_num);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ipsla_glb->t_ipsla_timer));
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE].name,
            p_ipsla_glb->trap_test_failure);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS].name,
            p_ipsla_glb->trap_test_success);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT].name,
            p_ipsla_glb->trap_pkt_timeout);
    }
    if (FLD_MATCH(TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD].name,
            p_ipsla_glb->trap_overthreshold);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsla_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsla_global_t *p_db_ipsla_glb = _g_p_tbl_ipsla_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ipsla_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ipsla_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipsla_global_t*
tbl_ipsla_global_init_ipsla_global()
{
    _g_p_tbl_ipsla_global = XCALLOC(MEM_TBL_IPSLA_GLOBAL, sizeof(tbl_ipsla_global_t));
    return _g_p_tbl_ipsla_global;
}


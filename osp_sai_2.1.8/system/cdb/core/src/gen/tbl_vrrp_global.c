
#include "proto.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "cdb_data_cmp.h"

tbl_vrrp_global_t *_g_p_tbl_vrrp_global = NULL;

int32
tbl_vrrp_global_set_vrrp_global_field_sync(tbl_vrrp_global_t *p_vrrp_glb, tbl_vrrp_global_field_id_t field_id, uint32 sync)
{
    tbl_vrrp_global_t *p_db_vrrp_glb = _g_p_tbl_vrrp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER:
        p_db_vrrp_glb->t_vrrp_timer = p_vrrp_glb->t_vrrp_timer;
        break;

    case TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER:
        p_db_vrrp_glb->t_vrrp_msec_timer = p_vrrp_glb->t_vrrp_msec_timer;
        break;

    case TBL_VRRP_GLOBAL_FLD_VRRP_VERSION:
        p_db_vrrp_glb->vrrp_version = p_vrrp_glb->vrrp_version;
        break;

    case TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT:
        p_db_vrrp_glb->vrrp_session_count = p_vrrp_glb->vrrp_session_count;
        break;

    case TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT:
        p_db_vrrp_glb->vrrp_msec_session_count = p_vrrp_glb->vrrp_msec_session_count;
        break;

    case TBL_VRRP_GLOBAL_FLD_BUF:
        sal_memcpy(p_db_vrrp_glb->buf, p_vrrp_glb->buf, sizeof(p_vrrp_glb->buf));
        break;

    case TBL_VRRP_GLOBAL_FLD_PKT_LEN:
        p_db_vrrp_glb->pkt_len = p_vrrp_glb->pkt_len;
        break;

    case TBL_VRRP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_vrrp_glb, p_vrrp_glb, sizeof(tbl_vrrp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VRRP_GLOBAL, field_id, p_db_vrrp_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VRRP_GLOBAL, field_id, p_db_vrrp_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_global_set_vrrp_global_field(tbl_vrrp_global_t *p_vrrp_glb, tbl_vrrp_global_field_id_t field_id)
{
    return tbl_vrrp_global_set_vrrp_global_field_sync(p_vrrp_glb, field_id, TRUE);
}

tbl_vrrp_global_t*
tbl_vrrp_global_get_vrrp_global()
{
    return _g_p_tbl_vrrp_global;
}

int32
tbl_vrrp_global_key_str2val(char *str, tbl_vrrp_global_t *p_vrrp_glb)
{
    return PM_E_NONE;
}

char*
tbl_vrrp_global_key_name_dump(tbl_vrrp_global_t *p_vrrp_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_vrrp_global_key_value_dump(tbl_vrrp_global_t *p_vrrp_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_vrrp_global_field_name_dump(tbl_vrrp_global_t *p_vrrp_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VRRP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vrrp_global_field_value_dump(tbl_vrrp_global_t *p_vrrp_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_vrrp_glb->t_vrrp_timer);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_vrrp_glb->t_vrrp_msec_timer);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_glb->vrrp_version);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_glb->vrrp_session_count);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_glb->vrrp_msec_session_count);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_vrrp_glb->buf, sizeof(p_vrrp_glb->buf));
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_PKT_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_vrrp_glb->pkt_len);
    }
    return str;
}

char**
tbl_vrrp_global_table_dump(tbl_vrrp_global_t *p_vrrp_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vrrp_global_key_name_dump(p_vrrp_glb, buf));
    for(i=1; i<TBL_VRRP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_global_field_name_dump(p_vrrp_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_global_field_value_dump(p_vrrp_glb, i, buf));
    }
    return str;
}

int32
tbl_vrrp_global_field_value_parser(char *str, int32 field_id, tbl_vrrp_global_t *p_vrrp_glb)
{
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER, field_id))
    {
        ctc_task_str2val(str, p_vrrp_glb->t_vrrp_timer);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER, field_id))
    {
        ctc_task_str2val(str, p_vrrp_glb->t_vrrp_msec_timer);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_VERSION, field_id))
    {
        int32 ret;
        p_vrrp_glb->vrrp_version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT, field_id))
    {
        int32 ret;
        p_vrrp_glb->vrrp_session_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT, field_id))
    {
        int32 ret;
        p_vrrp_glb->vrrp_msec_session_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_vrrp_glb->buf);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_PKT_LEN, field_id))
    {
        int32 ret;
        p_vrrp_glb->pkt_len = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vrrp_global_table_parser(char** array, char* key_value,tbl_vrrp_global_t *p_vrrp_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vrrp_global_key_str2val(key_value, p_vrrp_glb));

    for(i=1; i<TBL_VRRP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vrrp_global_field_value_parser( array[j++], i, p_vrrp_glb));
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_global_dump_one(tbl_vrrp_global_t *p_vrrp_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_glb->t_vrrp_timer));
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_glb->t_vrrp_msec_timer));
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_VRRP_VERSION].name,
            p_vrrp_glb->vrrp_version);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT].name,
            p_vrrp_glb->vrrp_session_count);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT].name,
            p_vrrp_glb->vrrp_msec_session_count);
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_glb->buf, sizeof(p_vrrp_glb->buf)));
    }
    if (FLD_MATCH(TBL_VRRP_GLOBAL_FLD_PKT_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VRRP_GLOBAL_FLD_PKT_LEN].name,
            p_vrrp_glb->pkt_len);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vrrp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vrrp_global_t *p_db_vrrp_glb = _g_p_tbl_vrrp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_vrrp_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_vrrp_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vrrp_global_t*
tbl_vrrp_global_init_vrrp_global()
{
    _g_p_tbl_vrrp_global = XCALLOC(MEM_TBL_VRRP_GLOBAL, sizeof(tbl_vrrp_global_t));
    return _g_p_tbl_vrrp_global;
}


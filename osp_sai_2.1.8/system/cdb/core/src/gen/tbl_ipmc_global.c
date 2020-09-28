
#include "proto.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "cdb_data_cmp.h"

tbl_ipmc_global_t *_g_p_tbl_ipmc_global = NULL;

int32
tbl_ipmc_global_set_ipmc_global_field_sync(tbl_ipmc_global_t *p_pim_glb, tbl_ipmc_global_field_id_t field_id, uint32 sync)
{
    tbl_ipmc_global_t *p_db_pim_glb = _g_p_tbl_ipmc_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN:
        p_db_pim_glb->mcast_route_en = p_pim_glb->mcast_route_en;
        break;

    case TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME:
        p_db_pim_glb->register_suppress_time = p_pim_glb->register_suppress_time;
        break;

    case TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS:
        p_db_pim_glb->pim_packet_process = p_pim_glb->pim_packet_process;
        break;

    case TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME:
        p_db_pim_glb->keep_alive_time = p_pim_glb->keep_alive_time;
        break;

    case TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME:
        p_db_pim_glb->rp_keep_alive_time = p_pim_glb->rp_keep_alive_time;
        break;

    case TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL:
        p_db_pim_glb->join_prune_interval = p_pim_glb->join_prune_interval;
        break;

    case TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE:
        p_db_pim_glb->switchover_mode = p_pim_glb->switchover_mode;
        break;

    case TBL_IPMC_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_pim_glb, p_pim_glb, sizeof(tbl_ipmc_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPMC_GLOBAL, field_id, p_db_pim_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPMC_GLOBAL, field_id, p_db_pim_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipmc_global_set_ipmc_global_field(tbl_ipmc_global_t *p_pim_glb, tbl_ipmc_global_field_id_t field_id)
{
    return tbl_ipmc_global_set_ipmc_global_field_sync(p_pim_glb, field_id, TRUE);
}

tbl_ipmc_global_t*
tbl_ipmc_global_get_ipmc_global()
{
    return _g_p_tbl_ipmc_global;
}

int32
tbl_ipmc_global_key_str2val(char *str, tbl_ipmc_global_t *p_pim_glb)
{
    return PM_E_NONE;
}

char*
tbl_ipmc_global_key_name_dump(tbl_ipmc_global_t *p_pim_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ipmc_global_key_value_dump(tbl_ipmc_global_t *p_pim_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ipmc_global_field_name_dump(tbl_ipmc_global_t *p_pim_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPMC_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipmc_global_field_value_dump(tbl_ipmc_global_t *p_pim_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->mcast_route_en);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->register_suppress_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->pim_packet_process);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->keep_alive_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->rp_keep_alive_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->join_prune_interval);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_pim_glb->switchover_mode);
    }
    return str;
}

char**
tbl_ipmc_global_table_dump(tbl_ipmc_global_t *p_pim_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipmc_global_key_name_dump(p_pim_glb, buf));
    for(i=1; i<TBL_IPMC_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipmc_global_field_name_dump(p_pim_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipmc_global_field_value_dump(p_pim_glb, i, buf));
    }
    return str;
}

int32
tbl_ipmc_global_field_value_parser(char *str, int32 field_id, tbl_ipmc_global_t *p_pim_glb)
{
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN, field_id))
    {
        int32 ret;
        p_pim_glb->mcast_route_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME, field_id))
    {
        int32 ret;
        p_pim_glb->register_suppress_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS, field_id))
    {
        int32 ret;
        p_pim_glb->pim_packet_process = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME, field_id))
    {
        int32 ret;
        p_pim_glb->keep_alive_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME, field_id))
    {
        int32 ret;
        p_pim_glb->rp_keep_alive_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL, field_id))
    {
        int32 ret;
        p_pim_glb->join_prune_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE, field_id))
    {
        int32 ret;
        p_pim_glb->switchover_mode = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipmc_global_table_parser(char** array, char* key_value,tbl_ipmc_global_t *p_pim_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipmc_global_key_str2val(key_value, p_pim_glb));

    for(i=1; i<TBL_IPMC_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipmc_global_field_value_parser( array[j++], i, p_pim_glb));
    }

    return PM_E_NONE;
}

int32
tbl_ipmc_global_dump_one(tbl_ipmc_global_t *p_pim_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN].name,
            p_pim_glb->mcast_route_en);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME].name,
            p_pim_glb->register_suppress_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS].name,
            p_pim_glb->pim_packet_process);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME].name,
            p_pim_glb->keep_alive_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME].name,
            p_pim_glb->rp_keep_alive_time);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL].name,
            p_pim_glb->join_prune_interval);
    }
    if (FLD_MATCH(TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE].name,
            p_pim_glb->switchover_mode);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipmc_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipmc_global_t *p_db_pim_glb = _g_p_tbl_ipmc_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_pim_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_pim_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipmc_global_t*
tbl_ipmc_global_init_ipmc_global()
{
    _g_p_tbl_ipmc_global = XCALLOC(MEM_TBL_IPMC_GLOBAL, sizeof(tbl_ipmc_global_t));
    return _g_p_tbl_ipmc_global;
}


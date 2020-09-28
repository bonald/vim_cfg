
#include "proto.h"
#include "gen/tbl_iptables_prevent_define.h"
#include "gen/tbl_iptables_prevent.h"
#include "cdb_data_cmp.h"

tbl_iptables_prevent_t *_g_p_tbl_iptables_prevent = NULL;

int32
tbl_iptables_prevent_set_iptables_prevent_field_sync(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iptables_prevent_field_id_t field_id, uint32 sync)
{
    tbl_iptables_prevent_t *p_db_iptables_prevent = _g_p_tbl_iptables_prevent;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_ICMP)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_SMURF)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_UDP)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_TCP)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_MACEQ)
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ:
        CDB_FLAG_CHECK_SET(p_db_iptables_prevent, p_iptables_prevent, flags, GLB_IPTABLES_CFG_FLAG_IPEQ)
        break;

    case TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT:
        p_db_iptables_prevent->icmp_rate_limit = p_iptables_prevent->icmp_rate_limit;
        break;

    case TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT:
        p_db_iptables_prevent->tcp_rate_limit = p_iptables_prevent->tcp_rate_limit;
        break;

    case TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT:
        p_db_iptables_prevent->udp_rate_limit = p_iptables_prevent->udp_rate_limit;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH:
        p_db_iptables_prevent->small_pkt_length = p_iptables_prevent->small_pkt_length;
        break;

    case TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE:
        p_db_iptables_prevent->pkt_statistic_update = p_iptables_prevent->pkt_statistic_update;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE:
        p_db_iptables_prevent->small_pkt_static_base = p_iptables_prevent->small_pkt_static_base;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE:
        p_db_iptables_prevent->small_pkt_mgmt_static_base = p_iptables_prevent->small_pkt_mgmt_static_base;
        break;

    case TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT:
        p_db_iptables_prevent->icmp_count = p_iptables_prevent->icmp_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT:
        p_db_iptables_prevent->smurf_count = p_iptables_prevent->smurf_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT:
        p_db_iptables_prevent->fraggle_count = p_iptables_prevent->fraggle_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_UDP_COUNT:
        p_db_iptables_prevent->udp_count = p_iptables_prevent->udp_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_TCP_COUNT:
        p_db_iptables_prevent->tcp_count = p_iptables_prevent->tcp_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT:
        p_db_iptables_prevent->small_pkt_count = p_iptables_prevent->small_pkt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT:
        p_db_iptables_prevent->icmp_mgmt_count = p_iptables_prevent->icmp_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT:
        p_db_iptables_prevent->smurf_mgmt_count = p_iptables_prevent->smurf_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT:
        p_db_iptables_prevent->fraggle_mgmt_count = p_iptables_prevent->fraggle_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT:
        p_db_iptables_prevent->udp_mgmt_count = p_iptables_prevent->udp_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT:
        p_db_iptables_prevent->tcp_mgmt_count = p_iptables_prevent->tcp_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT:
        p_db_iptables_prevent->small_pkt_mgmt_count = p_iptables_prevent->small_pkt_mgmt_count;
        break;

    case TBL_IPTABLES_PREVENT_FLD_MAX:
        sal_memcpy(p_db_iptables_prevent, p_iptables_prevent, sizeof(tbl_iptables_prevent_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPTABLES_PREVENT, field_id, p_db_iptables_prevent);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPTABLES_PREVENT, field_id, p_db_iptables_prevent);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_iptables_prevent_set_iptables_prevent_field(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iptables_prevent_field_id_t field_id)
{
    return tbl_iptables_prevent_set_iptables_prevent_field_sync(p_iptables_prevent, field_id, TRUE);
}

tbl_iptables_prevent_t*
tbl_iptables_prevent_get_iptables_prevent()
{
    return _g_p_tbl_iptables_prevent;
}

int32
tbl_iptables_prevent_key_str2val(char *str, tbl_iptables_prevent_t *p_iptables_prevent)
{
    return PM_E_NONE;
}

char*
tbl_iptables_prevent_key_name_dump(tbl_iptables_prevent_t *p_iptables_prevent, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPTABLES_PREVENT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_iptables_prevent_key_value_dump(tbl_iptables_prevent_t *p_iptables_prevent, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_iptables_prevent_field_name_dump(tbl_iptables_prevent_t *p_iptables_prevent, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPTABLES_PREVENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPTABLES_PREVENT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_iptables_prevent_field_value_dump(tbl_iptables_prevent_t *p_iptables_prevent, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMURF));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->icmp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->tcp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->udp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->small_pkt_length);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->pkt_statistic_update);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->small_pkt_static_base);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->small_pkt_mgmt_static_base);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->icmp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->smurf_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->fraggle_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->udp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->tcp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->small_pkt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->icmp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->smurf_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->fraggle_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->udp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->tcp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_iptables_prevent->small_pkt_mgmt_count);
    }
    return str;
}

char**
tbl_iptables_prevent_table_dump(tbl_iptables_prevent_t *p_iptables_prevent, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_iptables_prevent_key_name_dump(p_iptables_prevent, buf));
    for(i=1; i<TBL_IPTABLES_PREVENT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_iptables_prevent_field_name_dump(p_iptables_prevent, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_iptables_prevent_field_value_dump(p_iptables_prevent, i, buf));
    }
    return str;
}

int32
tbl_iptables_prevent_field_value_parser(char *str, int32 field_id, tbl_iptables_prevent_t *p_iptables_prevent)
{
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMURF);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ, field_id))
    {
        GLB_SET_FLAG(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT, field_id))
    {
        int32 ret;
        p_iptables_prevent->icmp_rate_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT, field_id))
    {
        int32 ret;
        p_iptables_prevent->tcp_rate_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT, field_id))
    {
        int32 ret;
        p_iptables_prevent->udp_rate_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH, field_id))
    {
        int32 ret;
        p_iptables_prevent->small_pkt_length = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE, field_id))
    {
        int32 ret;
        p_iptables_prevent->pkt_statistic_update = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE, field_id))
    {
        int32 ret;
        p_iptables_prevent->small_pkt_static_base = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE, field_id))
    {
        int32 ret;
        p_iptables_prevent->small_pkt_mgmt_static_base = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->icmp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->smurf_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->fraggle_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->udp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->tcp_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->small_pkt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->icmp_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->smurf_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->fraggle_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->udp_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->tcp_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT, field_id))
    {
        int32 ret;
        p_iptables_prevent->small_pkt_mgmt_count = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_iptables_prevent_table_parser(char** array, char* key_value,tbl_iptables_prevent_t *p_iptables_prevent)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPTABLES_PREVENT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_iptables_prevent_key_str2val(key_value, p_iptables_prevent));

    for(i=1; i<TBL_IPTABLES_PREVENT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_iptables_prevent_field_value_parser( array[j++], i, p_iptables_prevent));
    }

    return PM_E_NONE;
}

int32
tbl_iptables_prevent_dump_one(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPTABLES_PREVENT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMURF));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ].name,
            GLB_FLAG_ISSET(p_iptables_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ));
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT].name,
            p_iptables_prevent->icmp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT].name,
            p_iptables_prevent->tcp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT].name,
            p_iptables_prevent->udp_rate_limit);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH].name,
            p_iptables_prevent->small_pkt_length);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE].name,
            p_iptables_prevent->pkt_statistic_update);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE].name,
            p_iptables_prevent->small_pkt_static_base);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE].name,
            p_iptables_prevent->small_pkt_mgmt_static_base);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT].name,
            p_iptables_prevent->icmp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT].name,
            p_iptables_prevent->smurf_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT].name,
            p_iptables_prevent->fraggle_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_UDP_COUNT].name,
            p_iptables_prevent->udp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_TCP_COUNT].name,
            p_iptables_prevent->tcp_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT].name,
            p_iptables_prevent->small_pkt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT].name,
            p_iptables_prevent->icmp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT].name,
            p_iptables_prevent->smurf_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT].name,
            p_iptables_prevent->fraggle_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT].name,
            p_iptables_prevent->udp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT].name,
            p_iptables_prevent->tcp_mgmt_count);
    }
    if (FLD_MATCH(TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT].name,
            p_iptables_prevent->small_pkt_mgmt_count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_iptables_prevent_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_iptables_prevent_t *p_db_iptables_prevent = _g_p_tbl_iptables_prevent;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_iptables_prevent)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_iptables_prevent, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_iptables_prevent_t*
tbl_iptables_prevent_init_iptables_prevent()
{
    _g_p_tbl_iptables_prevent = XCALLOC(MEM_TBL_IPTABLES_PREVENT, sizeof(tbl_iptables_prevent_t));
    return _g_p_tbl_iptables_prevent;
}


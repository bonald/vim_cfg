
#include "proto.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "cdb_data_cmp.h"

tbl_nat_global_t *_g_p_tbl_nat_global = NULL;

int32
tbl_nat_global_set_nat_global_field_sync(tbl_nat_global_t *p_nat_glb, tbl_nat_global_field_id_t field_id, uint32 sync)
{
    tbl_nat_global_t *p_db_nat_glb = _g_p_tbl_nat_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NAT_GLOBAL_FLD_TIMEOUT:
        p_db_nat_glb->timeout = p_nat_glb->timeout;
        break;

    case TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP:
        p_db_nat_glb->timeout_icmp = p_nat_glb->timeout_icmp;
        break;

    case TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP:
        p_db_nat_glb->timeout_tcp = p_nat_glb->timeout_tcp;
        break;

    case TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP:
        p_db_nat_glb->timeout_udp = p_nat_glb->timeout_udp;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_LIMIT:
        p_db_nat_glb->session_limit = p_nat_glb->session_limit;
        break;

    case TBL_NAT_GLOBAL_FLD_NFT_HDL:
        p_db_nat_glb->nft_hdl = p_nat_glb->nft_hdl;
        break;

    case TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF:
        p_db_nat_glb->nft_hdl_upstream_if = p_nat_glb->nft_hdl_upstream_if;
        break;

    case TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST:
        p_db_nat_glb->nft_hdl_connlimit_post = p_nat_glb->nft_hdl_connlimit_post;
        break;

    case TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE:
        p_db_nat_glb->nft_hdl_connlimit_pre = p_nat_glb->nft_hdl_connlimit_pre;
        break;

    case TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE:
        p_db_nat_glb->nft_hdl_upstream_filter_nat_rule = p_nat_glb->nft_hdl_upstream_filter_nat_rule;
        break;

    case TBL_NAT_GLOBAL_FLD_RULE_ID_OPB:
        sal_memcpy(&p_db_nat_glb->rule_id_opb, &p_nat_glb->rule_id_opb, sizeof(p_nat_glb->rule_id_opb));
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB:
        sal_memcpy(&p_db_nat_glb->session_id_opb, &p_nat_glb->session_id_opb, sizeof(p_nat_glb->session_id_opb));
        break;

    case TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB:
        sal_memcpy(&p_db_nat_glb->category_id_opb, &p_nat_glb->category_id_opb, sizeof(p_nat_glb->category_id_opb));
        break;

    case TBL_NAT_GLOBAL_FLD_AGING_TIMER:
        p_db_nat_glb->aging_timer = p_nat_glb->aging_timer;
        break;

    case TBL_NAT_GLOBAL_FLD_TIMER_MARK:
        p_db_nat_glb->timer_mark = p_nat_glb->timer_mark;
        break;

    case TBL_NAT_GLOBAL_FLD_NFCT_HANDLE:
        p_db_nat_glb->nfct_handle = p_nat_glb->nfct_handle;
        break;

    case TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER:
        p_db_nat_glb->rule_cfg_order = p_nat_glb->rule_cfg_order;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM:
        p_db_nat_glb->session_cnt_upstream = p_nat_glb->session_cnt_upstream;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM:
        p_db_nat_glb->session_cnt_dnstream = p_nat_glb->session_cnt_dnstream;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM:
        p_db_nat_glb->session_fast_fwd_cnt_upstream = p_nat_glb->session_fast_fwd_cnt_upstream;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM:
        p_db_nat_glb->session_fast_fwd_cnt_dnstream = p_nat_glb->session_fast_fwd_cnt_dnstream;
        break;

    case TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT:
        p_db_nat_glb->nfct_kernel_notify_add_cnt = p_nat_glb->nfct_kernel_notify_add_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT:
        p_db_nat_glb->nfct_kernel_notify_del_cnt = p_nat_glb->nfct_kernel_notify_del_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT:
        p_db_nat_glb->nfct_notify_kernel_add_cnt = p_nat_glb->nfct_notify_kernel_add_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT:
        p_db_nat_glb->nfct_notify_kernel_del_cnt = p_nat_glb->nfct_notify_kernel_del_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT:
        p_db_nat_glb->session_aging_del_cnt = p_nat_glb->session_aging_del_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT:
        p_db_nat_glb->session_aging_upd_ttl_cnt = p_nat_glb->session_aging_upd_ttl_cnt;
        break;

    case TBL_NAT_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_nat_glb, p_nat_glb, sizeof(tbl_nat_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NAT_GLOBAL, field_id, p_db_nat_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NAT_GLOBAL, field_id, p_db_nat_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_global_set_nat_global_field(tbl_nat_global_t *p_nat_glb, tbl_nat_global_field_id_t field_id)
{
    return tbl_nat_global_set_nat_global_field_sync(p_nat_glb, field_id, TRUE);
}

tbl_nat_global_t*
tbl_nat_global_get_nat_global()
{
    return _g_p_tbl_nat_global;
}

int32
tbl_nat_global_key_str2val(char *str, tbl_nat_global_t *p_nat_glb)
{
    return PM_E_NONE;
}

char*
tbl_nat_global_key_name_dump(tbl_nat_global_t *p_nat_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_nat_global_key_value_dump(tbl_nat_global_t *p_nat_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_nat_global_field_name_dump(tbl_nat_global_t *p_nat_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NAT_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nat_global_field_value_dump(tbl_nat_global_t *p_nat_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->timeout);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->timeout_icmp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->timeout_tcp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->timeout_udp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_limit);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nft_hdl);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nft_hdl_upstream_if);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nft_hdl_connlimit_post);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nft_hdl_connlimit_pre);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nft_hdl_upstream_filter_nat_rule);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_ID_OPB, field_id))
    {
        cdb_opb_val2str(str, MAX_CMD_STR_LEN, &p_nat_glb->rule_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB, field_id))
    {
        cdb_opb_val2str(str, MAX_CMD_STR_LEN, &p_nat_glb->session_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB, field_id))
    {
        cdb_opb_val2str(str, MAX_CMD_STR_LEN, &p_nat_glb->category_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_AGING_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_nat_glb->aging_timer);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMER_MARK, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->timer_mark);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_HANDLE, field_id))
    {
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nat_glb->rule_cfg_order);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_cnt_upstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_cnt_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_fast_fwd_cnt_upstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_fast_fwd_cnt_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nfct_kernel_notify_add_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nfct_kernel_notify_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nfct_notify_kernel_add_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->nfct_notify_kernel_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_aging_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_glb->session_aging_upd_ttl_cnt);
    }
    return str;
}

char**
tbl_nat_global_table_dump(tbl_nat_global_t *p_nat_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nat_global_key_name_dump(p_nat_glb, buf));
    for(i=1; i<TBL_NAT_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nat_global_field_name_dump(p_nat_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nat_global_field_value_dump(p_nat_glb, i, buf));
    }
    return str;
}

int32
tbl_nat_global_field_value_parser(char *str, int32 field_id, tbl_nat_global_t *p_nat_glb)
{
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_nat_glb->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP, field_id))
    {
        int32 ret;
        p_nat_glb->timeout_icmp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP, field_id))
    {
        int32 ret;
        p_nat_glb->timeout_tcp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP, field_id))
    {
        int32 ret;
        p_nat_glb->timeout_udp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_LIMIT, field_id))
    {
        int32 ret;
        p_nat_glb->session_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL, field_id))
    {
        int32 ret;
        p_nat_glb->nft_hdl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF, field_id))
    {
        int32 ret;
        p_nat_glb->nft_hdl_upstream_if = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST, field_id))
    {
        int32 ret;
        p_nat_glb->nft_hdl_connlimit_post = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE, field_id))
    {
        int32 ret;
        p_nat_glb->nft_hdl_connlimit_pre = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE, field_id))
    {
        int32 ret;
        p_nat_glb->nft_hdl_upstream_filter_nat_rule = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_ID_OPB, field_id))
    {
        cdb_opb_str2val(str, &p_nat_glb->rule_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB, field_id))
    {
        cdb_opb_str2val(str, &p_nat_glb->session_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB, field_id))
    {
        cdb_opb_str2val(str, &p_nat_glb->category_id_opb);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_AGING_TIMER, field_id))
    {
        ctc_task_str2val(str, p_nat_glb->aging_timer);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMER_MARK, field_id))
    {
        int32 ret;
        p_nat_glb->timer_mark = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_HANDLE, field_id))
    {
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER, field_id))
    {
        int32 ret;
        p_nat_glb->rule_cfg_order = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM, field_id))
    {
        int32 ret;
        p_nat_glb->session_cnt_upstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM, field_id))
    {
        int32 ret;
        p_nat_glb->session_cnt_dnstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM, field_id))
    {
        int32 ret;
        p_nat_glb->session_fast_fwd_cnt_upstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM, field_id))
    {
        int32 ret;
        p_nat_glb->session_fast_fwd_cnt_dnstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->nfct_kernel_notify_add_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->nfct_kernel_notify_del_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->nfct_notify_kernel_add_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->nfct_notify_kernel_del_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->session_aging_del_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT, field_id))
    {
        int32 ret;
        p_nat_glb->session_aging_upd_ttl_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nat_global_table_parser(char** array, char* key_value,tbl_nat_global_t *p_nat_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nat_global_key_str2val(key_value, p_nat_glb));

    for(i=1; i<TBL_NAT_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nat_global_field_value_parser( array[j++], i, p_nat_glb));
    }

    return PM_E_NONE;
}

int32
tbl_nat_global_dump_one(tbl_nat_global_t *p_nat_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_TIMEOUT].name,
            p_nat_glb->timeout);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP].name,
            p_nat_glb->timeout_icmp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP].name,
            p_nat_glb->timeout_tcp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP].name,
            p_nat_glb->timeout_udp);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_LIMIT].name,
            p_nat_glb->session_limit);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFT_HDL].name,
            p_nat_glb->nft_hdl);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF].name,
            p_nat_glb->nft_hdl_upstream_if);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST].name,
            p_nat_glb->nft_hdl_connlimit_post);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE].name,
            p_nat_glb->nft_hdl_connlimit_pre);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE].name,
            p_nat_glb->nft_hdl_upstream_filter_nat_rule);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_RULE_ID_OPB].name, 
            cdb_opb_val2str(buf, MAX_CMD_STR_LEN, &p_nat_glb->rule_id_opb));
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB].name, 
            cdb_opb_val2str(buf, MAX_CMD_STR_LEN, &p_nat_glb->session_id_opb));
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB].name, 
            cdb_opb_val2str(buf, MAX_CMD_STR_LEN, &p_nat_glb->category_id_opb));
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_AGING_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_AGING_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_nat_glb->aging_timer));
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_TIMER_MARK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_TIMER_MARK].name,
            p_nat_glb->timer_mark);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_HANDLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFCT_HANDLE].name);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER].name,
            p_nat_glb->rule_cfg_order);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM].name,
            p_nat_glb->session_cnt_upstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM].name,
            p_nat_glb->session_cnt_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM].name,
            p_nat_glb->session_fast_fwd_cnt_upstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM].name,
            p_nat_glb->session_fast_fwd_cnt_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT].name,
            p_nat_glb->nfct_kernel_notify_add_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT].name,
            p_nat_glb->nfct_kernel_notify_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT].name,
            p_nat_glb->nfct_notify_kernel_add_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT].name,
            p_nat_glb->nfct_notify_kernel_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT].name,
            p_nat_glb->session_aging_del_cnt);
    }
    if (FLD_MATCH(TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT].name,
            p_nat_glb->session_aging_upd_ttl_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nat_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nat_global_t *p_db_nat_glb = _g_p_tbl_nat_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_nat_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_nat_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nat_global_t*
tbl_nat_global_init_nat_global()
{
    _g_p_tbl_nat_global = XCALLOC(MEM_TBL_NAT_GLOBAL, sizeof(tbl_nat_global_t));
    return _g_p_tbl_nat_global;
}


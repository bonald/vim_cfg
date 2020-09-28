
#include "snmpagt_define.h"
#include "snmpagt.h"


snmpagt_row_t IFX_TBL_ROW[] = 
{
    {IFX_TBL_MEM_NAME,                  ASN_OCTET_STR,  0},
    {IFX_TBL_MEM_IN_MULTICAST_PKTS,     ASN_COUNTER,    0},
    {IFX_TBL_MEM_IN_BROADCAST_PKTS,     ASN_COUNTER,    0},
    {IFX_TBL_MEM_OUT_MULTICAST_PKTS,    ASN_COUNTER,    0},
    {IFX_TBL_MEM_OUT_BROADCAST_PKTS,    ASN_COUNTER,    0},
    {IFX_TBL_MEM_HC_IN_OCTETS,          ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_IN_UCAST_PKTS,      ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_IN_MULTICAST_PKTS,  ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_IN_BROADCAST_PKTS,  ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_OUT_OCTETS,         ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_OUT_UCAST_PKTS,     ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS, ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS, ASN_COUNTER64,  0},
    {IFX_TBL_MEM_HIGH_SPEED,            ASN_UNSIGNED,   0},
    {IFX_TBL_MEM_ALIAS,                 ASN_OCTET_STR,  1}
};

static snmpagt_table_t *g_p_ifx_tbl = NULL;
static oid IFXTABLE_OID[] = { PUBLICMIB_OID, 31, 1, 1 };

int
ifx_table_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    netsnmp_table_request_info *table_info = NULL;

    //snmp_log(LOG_WARNING, "ifx_table_handler mode (%d)", reqinfo->mode);

    switch (reqinfo->mode)
    {
    case MODE_SET_RESERVE1: /* check */
        if (requests->requestvb->type != ASN_OCTET_STR)
        {
            return SNMP_ERR_BADVALUE;
        }
        table_info = netsnmp_extract_table_info(requests);
        switch (table_info->colnum)
        {
        case IFX_TBL_MEM_ALIAS:
            return SNMP_ERR_NOERROR;
        default:
            return SNMP_ERR_BADVALUE;
        }
        break;

    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_GENERR;
}

int32
ifx_table_add_row(tbl_interface_t *p_if)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    char ifname_ext[IFNAME_SIZE];
    
    if (NULL == g_p_ifx_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &ifindex, sizeof(ifindex));
    
    /* columns */
    IFNAME_ETH2SHORT(p_if->key.name, ifname_ext);
    netsnmp_set_row_column(row, IFX_TBL_MEM_NAME,       ASN_OCTET_STR, (char*)ifname_ext, sal_strlen(ifname_ext));
    u32 = (p_if->oper_bandwidth/1000);
    netsnmp_set_row_column(row, IFX_TBL_MEM_HIGH_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
    netsnmp_set_row_column(row, IFX_TBL_MEM_ALIAS,      ASN_OCTET_STR, (char*)p_if->desc, sal_strlen(p_if->desc));
    
    netsnmp_table_data_add_row(g_p_ifx_tbl->p_dataset->table, row);
    
    return 0;
}

netsnmp_table_row*
ifx_table_get_row(int32 ifindex)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 ifindex_i32 = ifindex;

    if (NULL == g_p_ifx_tbl)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &ifindex_i32, sizeof(ifindex_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    
    row = netsnmp_table_data_get(g_p_ifx_tbl->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}

int32
ifx_table_update_row(tbl_interface_t *p_if, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    
    if (NULL == g_p_ifx_tbl)
    {
        return -1;
    }
    
    row = ifx_table_get_row(ifindex);
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_BANDWIDTH:
    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
        u32 = (p_if->oper_bandwidth/1000);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HIGH_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
        break;
    case TBL_INTERFACE_FLD_DESC:
        netsnmp_set_row_column(row, IFX_TBL_MEM_ALIAS, ASN_OCTET_STR, (char*)p_if->desc, sal_strlen(p_if->desc));
        break;
    default:
        break;
    }
    
    return 0;
}


int32
ifx_table_update_row_stats(tbl_port_stats_t *p_if_stats, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if_stats->key.ifindex;
    uint32 u32 = 0;
    uint64 u64 = 0;
    struct counter64 c64;
    
    if (NULL == g_p_ifx_tbl)
    {
        return -1;
    }
    
    row = ifx_table_get_row(ifindex);

    if (NULL == row)
    {
        return 0;
    }
    switch (field_id)
    {      
    case TBL_PORT_STATS_FLD_MC_PKTS_RCV :
        u32 = p_if_stats->mc_pkts_rcv;
        netsnmp_set_row_column(row, IFX_TBL_MEM_IN_MULTICAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->mc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_MULTICAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_BRDC_PKTS_RCV :
        u32 = p_if_stats->brdc_pkts_rcv;
        netsnmp_set_row_column(row, IFX_TBL_MEM_IN_BROADCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->brdc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_BROADCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_MC_PKTS_SEND :
        u32 = p_if_stats->mc_pkts_send;
        netsnmp_set_row_column(row, IFX_TBL_MEM_OUT_MULTICAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->mc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_BRDC_PKTS_SEND :
        u32 = p_if_stats->brdc_pkts_send;
        netsnmp_set_row_column(row, IFX_TBL_MEM_OUT_BROADCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->brdc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_OCTETS_RCV :
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            u64 = p_if_stats->octets_rcv + p_if_stats->pkts_rcv*GLB_DEF_IPG_LEN;
        }
        else
        {
            u64 = p_if_stats->octets_rcv;
        }
        c64 = snmpagt_u64_to_c64(u64);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_OCTETS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_RCV :
        c64 = snmpagt_u64_to_c64(p_if_stats->uc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_UCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_OCTETS_SEND :
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            u64 = p_if_stats->octets_send + p_if_stats->pkts_send*GLB_DEF_IPG_LEN;
        }
        else
        {
            u64 = p_if_stats->octets_send;
        }
        c64 = snmpagt_u64_to_c64(u64);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_OCTETS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_SEND :
        c64 = snmpagt_u64_to_c64(p_if_stats->uc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_UCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;
        
    case TBL_PORT_STATS_FLD_MAX:
        u32 = p_if_stats->mc_pkts_rcv;
        netsnmp_set_row_column(row, IFX_TBL_MEM_IN_MULTICAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->mc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_MULTICAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        u32 = p_if_stats->brdc_pkts_rcv;
        netsnmp_set_row_column(row, IFX_TBL_MEM_IN_BROADCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->brdc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_BROADCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        u32 = p_if_stats->mc_pkts_send;
        netsnmp_set_row_column(row, IFX_TBL_MEM_OUT_MULTICAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->mc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        u32 = p_if_stats->brdc_pkts_send;
        netsnmp_set_row_column(row, IFX_TBL_MEM_OUT_BROADCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        c64 = snmpagt_u64_to_c64(p_if_stats->brdc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            u64 = p_if_stats->octets_rcv + p_if_stats->pkts_rcv*GLB_DEF_IPG_LEN;
        }
        else
        {
            u64 = p_if_stats->octets_rcv;
        }
        c64 = snmpagt_u64_to_c64(u64);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_OCTETS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        c64 = snmpagt_u64_to_c64(p_if_stats->uc_pkts_rcv);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_IN_UCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            u64 = p_if_stats->octets_send + p_if_stats->pkts_send*GLB_DEF_IPG_LEN;
        }
        else
        {
            u64 = p_if_stats->octets_send;
        }
        c64 = snmpagt_u64_to_c64(u64);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_OCTETS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        c64 = snmpagt_u64_to_c64(p_if_stats->uc_pkts_send);
        netsnmp_set_row_column(row, IFX_TBL_MEM_HC_OUT_UCAST_PKTS, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
        break;

    default:
        break;
    }
    
    return 0;
}

int32
ifx_table_del_row(int32 ifindex)
{
    netsnmp_table_row *row = NULL;
    row = ifx_table_get_row(ifindex);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_ifx_tbl->p_dataset, row);
    return 0;
}

int32
ifx_table_dump_row()
{
    netsnmp_table_row *row = NULL;
    uint32 i = 0;
    uint32 count = 0;
    char tmp[CMD_BUF_16];
    char buf[CMD_BUF_1024];

    sal_printf("IFX table %s\n", g_p_ifx_tbl->p_dataset->table->name);
    
    for (row = netsnmp_table_data_set_get_first_row(g_p_ifx_tbl->p_dataset); row; row = netsnmp_table_data_set_get_next_row(g_p_ifx_tbl->p_dataset, row))
    {
        buf[0] = CMD_ZERO_CHAR;
        for (i = 0; i < row->index_oid_len; i++)
        {
            sal_sprintf(tmp, "%d.", (int32)row->index_oid[i]);
            sal_strcat(buf, tmp);
        }
        sal_printf("oids[%d] = %s \n", count, buf);
        count++;
    }

    return 0;
}

int32
ifx_table_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];
    
    snmpagt_oid2str(IFXTABLE_OID, sizeof(IFXTABLE_OID)/sizeof(oid), oid_str, CMD_BUF_256);

    sal_memset(&param, 0, sizeof(param));
    param.oid_str = oid_str;
    param.no_idx_col = TRUE;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = IFX_TBL_ROW;
    param.row_count = sizeof(IFX_TBL_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = ifx_table_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }

    g_p_ifx_tbl = table;

    return 0;
}



#include "snmpagt_define.h"
#include "snmpagt.h"

snmpagt_row_t IF_TBL_ROW[] = 
{
    {IF_TBL_MEM_INDEX             ,    ASN_INTEGER,    0},
    {IF_TBL_MEM_DESCR             ,    ASN_OCTET_STR,  0},
    {IF_TBL_MEM_TYPE              ,    ASN_INTEGER,    0},
    {IF_TBL_MEM_MTU               ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_SPEED             ,    ASN_UNSIGNED,   0},
    {IF_TBL_MEM_PHYS_ADDRESS      ,    ASN_OCTET_STR,  0},
    {IF_TBL_MEM_ADMIN_STATUS      ,    ASN_INTEGER,    1},
    {IF_TBL_MEM_OPER_STATUS       ,    ASN_INTEGER,    0},
    {IF_TBL_MEM_IN_OCTECTS        ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_IN_UCASTPKTS      ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_IN_NUCASTPKTS     ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_IN_DISCARDS       ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_IN_ERRORS         ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_OUT_OCTECTS       ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_OUT_UCAST_PKTS    ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_OUT_NUCAST_PKTS   ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_OUT_DISCARDS      ,    ASN_COUNTER,    0},
    {IF_TBL_MEM_OUT_ERRORS        ,    ASN_COUNTER,    0}

  
};

static snmpagt_table_t *g_p_if_tbl = NULL;
static oid IFTABLE_OID[] = { PUBLICMIB_OID, 2, 2};

int
if_table_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
    netsnmp_table_request_info *table_info = NULL;

    //snmp_log(LOG_WARNING, "ifx_table_handler mode (%d)", reqinfo->mode);

    switch (reqinfo->mode)
    {
    case MODE_SET_RESERVE1: /* check */
        if (requests->requestvb->type != ASN_INTEGER)
        {
            return SNMP_ERR_BADVALUE;
        }
        table_info = netsnmp_extract_table_info(requests);
        switch (table_info->colnum)
        {
        case IF_TBL_MEM_OPER_STATUS:
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

uint32
if_table_get_speed(uint32 bandwidth)
{
    uint64 u64 = 0;
    uint32 u32 = 0;

    u64 = bandwidth;
    u64 *= 1000;
    u32 = (u64 > GLB_COUNTER32_MAX) ? GLB_COUNTER32_MAX : u64;
    return u32;
}

int32
if_table_add_row(tbl_interface_t *p_if)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    char ifname_ext[IFNAME_SIZE];
    char str_buf[CMD_BUF_256];
    
    if (NULL == g_p_if_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &ifindex, sizeof(ifindex));
    
    /* columns */
    u32 = p_if->ifindex;
    netsnmp_set_row_column(row, IF_TBL_MEM_INDEX,       ASN_INTEGER, (char*)(&u32), sizeof(u32));
    
    IFNAME_ETH2SHORT(p_if->key.name, ifname_ext);
    netsnmp_set_row_column(row, IF_TBL_MEM_DESCR,       ASN_OCTET_STR, (char*)ifname_ext, sal_strlen(ifname_ext));
    u32 = p_if->mtu;
    netsnmp_set_row_column(row, IF_TBL_MEM_MTU, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    switch (p_if->hw_type)
    {
    case GLB_IF_TYPE_LINKAGG_IF:
        u32 = GLB_IF_TYPE_AGG;
        break;
    case GLB_IF_TYPE_VLAN_IF:
        u32 = GLB_IF_TYPE_VLANIF;
        break;
    case GLB_IF_TYPE_LOOPBACK_IF:
        u32 = GLB_IF_TYPE_SOFTWARELOOPBACK;
        break;
    case GLB_IF_TYPE_TUNNEL_IF:
        u32 = GLB_IF_TYPE_TUNNEL;
        break;
    default:
        u32 = GLB_IF_TYPE_ETHERNETCSMACD;
        break;
    }
    netsnmp_set_row_column(row, IF_TBL_MEM_TYPE, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    u32 = if_table_get_speed(p_if->oper_bandwidth);
    netsnmp_set_row_column(row, IF_TBL_MEM_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
    
    if (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF || p_if->hw_type == GLB_IF_TYPE_VLAN_IF)
    {
        sal_memset(str_buf, 0 , sizeof(str_buf));
        netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS,      ASN_OCTET_STR, (char*)p_if->mac_addr, sizeof(p_if->mac_addr));
    }
    else if (p_if->hw_type == GLB_IF_TYPE_LOOPBACK_IF)
    {
        sal_memset(str_buf, 0 , sizeof(str_buf));
        str_buf[0]= '\0';
        netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS,      ASN_OCTET_STR, str_buf, sal_strlen(str_buf));
    }
    else if (NULL == p_if->brgif)
    {
        sal_memset(str_buf, 0 , sizeof(str_buf));
        netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS,      ASN_OCTET_STR, (char*)p_if->mac_addr, sizeof(p_if->mac_addr));
    }
    else
    {
        //sal_memset(str_buf, 0 , sizeof(str_buf));
        netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS,      ASN_OCTET_STR, (char*)p_if->hw_mac_addr, sizeof(p_if->hw_mac_addr));
    }
    u32 = (p_if->admin_up) ? GLB_STATE_ENABLE : GLB_STATE_DISABLE;
    netsnmp_set_row_column(row, IF_TBL_MEM_ADMIN_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = (tbl_interface_is_running(p_if)) ? GLB_STATE_ENABLE : GLB_STATE_DISABLE;
    netsnmp_set_row_column(row, IF_TBL_MEM_OPER_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    u32 = 0;
    netsnmp_set_row_column(row, IF_TBL_MEM_OUT_DISCARDS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
    netsnmp_table_data_add_row(g_p_if_tbl->p_dataset->table, row);
    
    return 0;
}

netsnmp_table_row*
if_table_get_row(int32 ifindex)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 ifindex_i32 = ifindex;

    if (NULL == g_p_if_tbl)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &ifindex_i32, sizeof(ifindex_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    
    row = netsnmp_table_data_get(g_p_if_tbl->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}

int32
if_table_update_row(tbl_interface_t *p_if, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    
    if (NULL == g_p_if_tbl)
    {
        return -1;
    }
    
    row = if_table_get_row(ifindex);
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_MTU:
        u32 = p_if->mtu;
        netsnmp_set_row_column(row, IF_TBL_MEM_MTU, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;
    case TBL_INTERFACE_FLD_BANDWIDTH:
    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
        u32 = if_table_get_speed(p_if->oper_bandwidth);
        netsnmp_set_row_column(row, IF_TBL_MEM_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
        break;
    case TBL_INTERFACE_FLD_ADMIN_UP:
        if (p_if->admin_up)
        {
            u32 = GLB_STATE_ENABLE;
            netsnmp_set_row_column(row, IF_TBL_MEM_ADMIN_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
            u32 = if_table_get_speed(p_if->oper_bandwidth);
            netsnmp_set_row_column(row, IF_TBL_MEM_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
        }
        else
        {
            u32 = GLB_STATE_DISABLE;
            netsnmp_set_row_column(row, IF_TBL_MEM_ADMIN_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        }
        
        break;

    case TBL_INTERFACE_FLD_RUNNING:
        if ((GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING) && GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP))
            || (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP) && GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type))
        {
            u32 = GLB_STATE_ENABLE;
            netsnmp_set_row_column(row, IF_TBL_MEM_OPER_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
            u32 = if_table_get_speed(p_if->oper_bandwidth);
            netsnmp_set_row_column(row, IF_TBL_MEM_SPEED, ASN_UNSIGNED, (char*)(&u32), sizeof(u32));
        }
        else
        {
            u32 = GLB_STATE_DISABLE;
            netsnmp_set_row_column(row, IF_TBL_MEM_OPER_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        }
        break;

    case TBL_INTERFACE_FLD_UP:
        if (GLB_IF_TYPE_LOOPBACK_IF != p_if->hw_type)
        {
            break;
        }
        if (GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP) && GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type)
        {
            u32 = GLB_STATE_ENABLE;
        }
        else
        {
            u32 = GLB_STATE_DISABLE;
        }
        netsnmp_set_row_column(row, IF_TBL_MEM_OPER_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_INTERFACE_FLD_MAC_ADDR:
        if (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF || p_if->hw_type == GLB_IF_TYPE_VLAN_IF || NULL == p_if->brgif)
        {
            netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS, ASN_OCTET_STR, (char*)p_if->mac_addr, sizeof(p_if->mac_addr));
        }
        break;

    case TBL_INTERFACE_FLD_HW_MAC_ADDR:
        if (p_if->hw_type == GLB_IF_TYPE_PORT_IF)
        {
            netsnmp_set_row_column(row, IF_TBL_MEM_PHYS_ADDRESS, ASN_OCTET_STR, (char*)p_if->hw_mac_addr, sizeof(p_if->hw_mac_addr));
        }
        break;
    default:
        break;
    }
    
    return 0;
}


int32
if_table_update_row_stats(tbl_port_stats_t *p_if_stats, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if_stats->key.ifindex;
    uint32 u32 = 0;
    
    if (NULL == g_p_if_tbl)
    {
        return -1;
    }
    
    row = if_table_get_row(ifindex);

    if (NULL == row)
    {
        return 0;
    }
    switch (field_id)
    {      
    case TBL_PORT_STATS_FLD_OCTETS_RCV :
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            p_if_stats->octets_rcv += p_if_stats->pkts_rcv*GLB_DEF_IPG_LEN;
        }
        u32 = p_if_stats->octets_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_OCTECTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_RCV :
        u32 = p_if_stats->uc_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_UCASTPKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_MC_PKTS_RCV :
    case TBL_PORT_STATS_FLD_BRDC_PKTS_RCV :
        u32 = p_if_stats->brdc_pkts_rcv + p_if_stats->mc_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_NUCASTPKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

#if 0
    // delete for bug 48989
    case TBL_PORT_STATS_FLD_BAD_CRC :
        u32 = (p_if_stats->bad_crc > GLB_COUNTER32_MAX) ? GLB_COUNTER32_MAX : p_if_stats->bad_crc;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_DISCARDS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;
#endif
    case TBL_PORT_STATS_FLD_BAD_PKTS_RCV :
        u32 = p_if_stats->bad_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_ERRORS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_OCTETS_SEND :
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            p_if_stats->octets_send += p_if_stats->pkts_send*GLB_DEF_IPG_LEN;
        }
        u32 = p_if_stats->octets_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_OCTECTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_UC_PKTS_SEND :
        u32 = p_if_stats->uc_pkts_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_UCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_MC_PKTS_SEND :
    case TBL_PORT_STATS_FLD_BRDC_PKTS_SEND :
        u32 = p_if_stats->brdc_pkts_send + p_if_stats->mc_pkts_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_NUCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_MAC_TRANSMIT_ERR :
        u32 = p_if_stats->mac_transmit_err;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_ERRORS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_PORT_STATS_FLD_MAX:
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            p_if_stats->octets_rcv += p_if_stats->pkts_rcv*GLB_DEF_IPG_LEN;
        }
        u32 = p_if_stats->octets_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_OCTECTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->uc_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_UCASTPKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->brdc_pkts_rcv + p_if_stats->mc_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_NUCASTPKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        /* in_discard dont get crc error stats for bug 48989 */
        u32 = 0;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_DISCARDS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->bad_pkts_rcv;
        netsnmp_set_row_column(row, IF_TBL_MEM_IN_ERRORS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        if (snmpagt_get_ifg_stats_en(p_if_stats->key.ifindex))
        {
            p_if_stats->octets_send += p_if_stats->pkts_send*GLB_DEF_IPG_LEN;
        }
        u32 = p_if_stats->octets_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_OCTECTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->uc_pkts_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_UCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->brdc_pkts_send + p_if_stats->mc_pkts_send;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_NUCAST_PKTS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        u32 = p_if_stats->mac_transmit_err;
        netsnmp_set_row_column(row, IF_TBL_MEM_OUT_ERRORS, ASN_COUNTER, (char*)(&u32), sizeof(u32));
        break;

    default:
        break;
    }
    
    return 0;
}

int32
if_table_del_row(int32 ifindex)
{
    netsnmp_table_row *row = NULL;
    row = if_table_get_row(ifindex);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_if_tbl->p_dataset, row);
    return 0;
}

int32
if_table_dump_row()
{
    netsnmp_table_row *row = NULL;
    uint32 i = 0;
    uint32 count = 0;
    char tmp[CMD_BUF_16];
    char buf[CMD_BUF_1024];

    sal_printf("IF table %s\n", g_p_if_tbl->p_dataset->table->name);
    
    for (row = netsnmp_table_data_set_get_first_row(g_p_if_tbl->p_dataset); row; row = netsnmp_table_data_set_get_next_row(g_p_if_tbl->p_dataset, row))
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
if_table_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];
    
    snmpagt_oid2str(IFTABLE_OID, sizeof(IFTABLE_OID)/sizeof(oid), oid_str, CMD_BUF_256);

    sal_memset(&param, 0, sizeof(param));
    param.oid_str = oid_str;
    param.no_idx_col = TRUE;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = IF_TBL_ROW;
    param.row_count = sizeof(IF_TBL_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = if_table_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }

    g_p_if_tbl = table;

    return 0;
}



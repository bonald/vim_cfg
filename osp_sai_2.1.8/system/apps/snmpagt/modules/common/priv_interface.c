
#include "snmpagt_define.h"
#include "snmpagt.h"

extern uint32
cdb_uint_str2val(const char *str, int32 *ret);

extern uint64
cdb_uint64_str2val(const char *str, int32 *ret);

snmpagt_row_t INTERFACE_TBL_ROW[] = 
{
    {INTERFACE_TBL_NAME               ,     ASN_OCTET_STR,  0},
    {INTERFACE_TBL_SPEED              ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_DUPLEX             ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_RESET_CNT          ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_BANDWIDTH          ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_UNIDIRECTIONAL     ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_LINK_STATUS        ,     ASN_INTEGER,    1},
    {INTERFACE_TBL_5MIN_IN_BITS_RATE  ,     ASN_COUNTER64,  0},
    {INTERFACE_TBL_5MIN_IN_PKTS_RATE  ,     ASN_COUNTER64,  0},
    {INTERFACE_TBL_5MIN_OUT_BITS_RATE ,     ASN_COUNTER64,  0},
    {INTERFACE_TBL_5MIN_OUT_PKTS_RATE ,     ASN_COUNTER64,  0}
};

static snmpagt_table_t *g_p_interface_tbl = NULL;
static oid INTERFACE_TABLE_OID[] = { CTCENTERPRISE_OID, 42, 1 };

              
int
interface_table_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{
//    netsnmp_table_request_info *table_info = NULL;

    switch (reqinfo->mode)
    {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
        return SNMP_ERR_NOERROR;

    default:
        break;
    }

    return SNMP_ERR_NOERROR;
}

netsnmp_table_row*
interface_table_get_row(int32 ifindex)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 ifindex_i32 = ifindex;

    if (NULL == g_p_interface_tbl)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &ifindex_i32, sizeof(ifindex_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    
    row = netsnmp_table_data_get(g_p_interface_tbl->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}

int32
interface_table_update_row(tbl_interface_t *p_if, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    
    if (NULL == g_p_interface_tbl)
    {
        return -1;
    }
    
    row = interface_table_get_row(ifindex);
    if (NULL == row)
    {
        return 0;
    }
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_SPEED:
    case TBL_INTERFACE_FLD_OPER_SPEED:
        u32 = (0 == p_if->speed)? p_if->oper_speed : p_if->speed;
        netsnmp_set_row_column(row, INTERFACE_TBL_SPEED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_INTERFACE_FLD_DUPLEX:
    case TBL_INTERFACE_FLD_OPER_DUPLEX:
        u32 = (0 == p_if->duplex)? p_if->oper_duplex : p_if->duplex;
        netsnmp_set_row_column(row, INTERFACE_TBL_DUPLEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;
        
    //case TBL_INTERFACE_FLD_BANDWIDTH:
    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
        u32 = (p_if->oper_bandwidth);
        netsnmp_set_row_column(row, INTERFACE_TBL_BANDWIDTH, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;
        
    case TBL_INTERFACE_FLD_UNIDIR:
        u32 = (0 == p_if->unidir)? GLB_STATE_DISABLE : GLB_STATE_ENABLE;
        netsnmp_set_row_column(row, INTERFACE_TBL_UNIDIRECTIONAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;

    case TBL_INTERFACE_FLD_RUNNING:
    case TBL_INTERFACE_FLD_ADMIN_UP:
        u32 = (tbl_interface_is_running(p_if)) ? GLB_STATE_ENABLE : GLB_STATE_DISABLE;
        netsnmp_set_row_column(row, INTERFACE_TBL_LINK_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));
        break;
        
    default:
        break;
    }
    
    return 0;
}

int32
interface_table_add_row(tbl_interface_t *p_if)
{
    netsnmp_table_row *row = NULL;
    int32 ifindex = p_if->ifindex;
    uint32 u32 = 0;
    uint64 u64 = 0;
    struct counter64 c64;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    if (NULL == g_p_interface_tbl)
    {
        return -1;
    }
    
    row = netsnmp_create_table_data_row();

    /* indexes */
    netsnmp_table_row_add_index(row, ASN_INTEGER, &ifindex, sizeof(ifindex));
    
    /* columns */
    IFNAME_ETH2SHORT(p_if->key.name, ifname_ext);
    netsnmp_set_row_column(row, INTERFACE_TBL_NAME,       ASN_OCTET_STR, (char*)ifname_ext, sal_strlen(ifname_ext));
    u32 = (0 != p_if->speed) ? p_if->speed: p_if->oper_speed;
    netsnmp_set_row_column(row, INTERFACE_TBL_SPEED, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = (0 != p_if->duplex) ? p_if->duplex: p_if->oper_duplex;
    netsnmp_set_row_column(row, INTERFACE_TBL_DUPLEX, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = 0;
    netsnmp_set_row_column(row, INTERFACE_TBL_RESET_CNT, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = p_if->oper_bandwidth;
    netsnmp_set_row_column(row, INTERFACE_TBL_BANDWIDTH, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    if (0 == p_if->unidir)
    {
        u32 = GLB_STATE_DISABLE;
    }
    else if (1 == p_if->unidir)
    {
        u32 = GLB_STATE_ENABLE;
    }
    else if (2 == p_if->unidir)
    {
        u32 = GLB_UNIDIR_RX_ONLY;
    }
    else if (3 == p_if->unidir)
    {
        u32 = GLB_UNIDIR_TX_ONLY;
    }    
    netsnmp_set_row_column(row, INTERFACE_TBL_UNIDIRECTIONAL, ASN_INTEGER, (char*)(&u32), sizeof(u32));
    u32 = (tbl_interface_is_running(p_if)) ? GLB_STATE_ENABLE : GLB_STATE_DISABLE;
    netsnmp_set_row_column(row, INTERFACE_TBL_LINK_STATUS, ASN_INTEGER, (char*)(&u32), sizeof(u32));

    u64 = 0;
    c64 = snmpagt_u64_to_c64(u64);
    netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_IN_BITS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
    netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_IN_PKTS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
    netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_OUT_BITS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
    netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_OUT_PKTS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
    
    netsnmp_table_data_add_row(g_p_interface_tbl->p_dataset->table, row);
    
    return 0;
}

int32
interface_table_del_row(int32 ifindex)
{
    netsnmp_table_row *row = NULL;
    row = interface_table_get_row(ifindex);
    if (NULL == row)
    {
        return -1;
    }
    
    netsnmp_table_dataset_remove_and_delete_row(g_p_interface_tbl->p_dataset, row);
    return 0;
}

int32
interface_table_dump_row()
{
    netsnmp_table_row *row = NULL;
    uint32 i = 0;
    uint32 count = 0;
    char tmp[CMD_BUF_16];
    char buf[CMD_BUF_1024];

    sal_printf("INTERFACE table %s\n", g_p_interface_tbl->p_dataset->table->name);
    
    for (row = netsnmp_table_data_set_get_first_row(g_p_interface_tbl->p_dataset); row; row = netsnmp_table_data_set_get_next_row(g_p_interface_tbl->p_dataset, row))
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

#define MIB_RATE_STATS_FILE "/tmp/mib_rate_stats"

int32
interface_update_rate()
{
    netsnmp_table_row *row = NULL;
    cfg_parse_get_info_t info;
    FILE *fp = NULL;
    char line[CMD_BUF_1024];
    uint32 ifindex = 0;
    uint64 u64_inpkts = 0;
    uint64 u64_outpkts = 0;
    uint64 u64_inbits = 0;
    uint64 u64_outbits = 0;
    struct counter64 c64;
    int32 ret = 0;

    if (NULL == g_p_interface_tbl)
    {
        return 0;
    }
    
    sal_system("cdbctl read/cdb/sys/show_mib_stats/rate > "MIB_RATE_STATS_FILE);
    
    fp = sal_fopen(MIB_RATE_STATS_FILE, "r");
    if (NULL == fp)
    {
        return 0;
    }

    while(sal_fgets(line, CMD_BUF_1024, fp) != NULL)
    {
        sal_memset(&info, 0, sizeof(info));
        cfg_get_parse(line, &info, 0);
        if (info.argc >= 5)
        {
            ifindex = cdb_uint_str2val(info.value[0], &ret);
            u64_inpkts = cdb_uint64_str2val(info.value[1], &ret);
            u64_outpkts = cdb_uint64_str2val(info.value[2], &ret);
            u64_inbits = cdb_uint64_str2val(info.value[3], &ret);
            u64_outbits = cdb_uint64_str2val(info.value[4], &ret);
            row = interface_table_get_row(ifindex);
            if (row)
            {
                c64 = snmpagt_u64_to_c64(u64_inpkts);
                netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_IN_PKTS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
                c64 = snmpagt_u64_to_c64(u64_outpkts);
                netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_OUT_PKTS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
                c64 = snmpagt_u64_to_c64(u64_inbits);
                netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_IN_BITS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
                c64 = snmpagt_u64_to_c64(u64_outbits);
                netsnmp_set_row_column(row, INTERFACE_TBL_5MIN_OUT_BITS_RATE, ASN_COUNTER64, (char*)(&c64), sizeof(c64));
            }
        }
    }

    sal_fclose(fp);
    sal_unlink(MIB_RATE_STATS_FILE);
    
    return 0;
}

int32
interface_table_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];
    
    snmpagt_oid2str(INTERFACE_TABLE_OID, sizeof(INTERFACE_TABLE_OID)/sizeof(oid), oid_str, CMD_BUF_256);

    sal_memset(&param, 0, sizeof(param));
    param.oid_str = oid_str;
    param.no_idx_col = TRUE;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = INTERFACE_TBL_ROW;
    param.row_count = sizeof(INTERFACE_TBL_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = interface_table_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }

    g_p_interface_tbl = table;

    return 0;
}


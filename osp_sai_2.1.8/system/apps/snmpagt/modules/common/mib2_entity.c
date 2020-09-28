
#include "snmpagt_define.h"
#include "snmpagt.h"

static oid ENTPHY_TABLE_OID[] = { PUBLICMIB_OID, 47, 1, 1, 1};

#define ENT_INFO_LEN 256

static snmpagt_table_t *g_p_entPhy_tbl = NULL;

snmpagt_row_t ENTPHY_TBL_ROW[] =
{
    {ENTPHY_TBL_SW_REV  ,     ASN_OCTET_STR,  0},
    {ENTPHY_TBL_SERIAL_NO,   ASN_OCTET_STR,   0},
    {ENTPHY_TBL_HW_TYPE,     ASN_OCTET_STR,  0}
};

int
entPhy_table_handler(netsnmp_mib_handler *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info *reqinfo,
                netsnmp_request_info *requests)
{

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
entPhy_table_get_row(int32 ifindex)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 ifindex_i32 = ifindex;

    if (NULL == g_p_entPhy_tbl)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &ifindex_i32, sizeof(ifindex_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    row = netsnmp_table_data_get(g_p_entPhy_tbl->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}


int32
version_add_row(tbl_version_t *p_version)
{
    netsnmp_table_row *row = NULL;
    uint32 index = 1;
    char hw_info[256] = {0};

    if (NULL == g_p_entPhy_tbl)
    {
        return -1;
    }

    row = entPhy_table_get_row(index);
    if (NULL == row)
    {
        row = netsnmp_create_table_data_row();
    }
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));

    if (p_version->is_cst)
    {
        netsnmp_set_row_column(row, ENTPHY_TBL_SW_REV, ASN_OCTET_STR, p_version->cst_sw_ver, sal_strlen(p_version->cst_sw_ver));
        /*modify by zhaoqd for bug 53084 2019-07-29*/
        sal_snprintf(hw_info, 256, "%s.%s", p_version->cst_hw_type, p_version->cst_hw_info);
        netsnmp_set_row_column(row, ENTPHY_TBL_HW_TYPE, ASN_OCTET_STR, hw_info, sal_strlen(hw_info));
    }
    else
    {
        netsnmp_set_row_column(row, ENTPHY_TBL_SW_REV, ASN_OCTET_STR, p_version->version, sal_strlen(p_version->version));
        netsnmp_set_row_column(row, ENTPHY_TBL_HW_TYPE, ASN_OCTET_STR, p_version->hw_type, sal_strlen(p_version->hw_type));
    }
    netsnmp_table_data_add_row(g_p_entPhy_tbl->p_dataset->table, row);

    return 0;

}

int32
version_set_row(tbl_version_t *p_version, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    uint32 index = 1;
    char hw_info[256] = {0};

    if (NULL == g_p_entPhy_tbl)
    {
        return -1;
    }

    row = entPhy_table_get_row(index);
    switch (field_id)
    {
        case TBL_VERSION_FLD_CST_SW_VER:
            netsnmp_set_row_column(row, ENTPHY_TBL_SW_REV, ASN_OCTET_STR, p_version->cst_sw_ver, sizeof(p_version->cst_sw_ver));
        case TBL_VERSION_FLD_VERSION:
            netsnmp_set_row_column(row, ENTPHY_TBL_SW_REV, ASN_OCTET_STR, p_version->version, sizeof(p_version->version));
        case TBL_VERSION_FLD_HW_TYPE:
            netsnmp_set_row_column(row, ENTPHY_TBL_HW_TYPE, ASN_OCTET_STR, p_version->hw_type, sal_strlen(p_version->hw_type));
        case TBL_VERSION_FLD_CST_HW_TYPE:
            /*modify by zhaoqd for bug 53084 2019-07-29*/
            sal_snprintf(hw_info, 256, "%s.%s", p_version->cst_hw_type, p_version->cst_hw_info);
            netsnmp_set_row_column(row, ENTPHY_TBL_HW_TYPE, ASN_OCTET_STR, hw_info, sal_strlen(hw_info));
        default:
            break;
    }
    return 0;
}

int32
card_add_row(tbl_card_t *p_card)
{
    netsnmp_table_row *row = NULL;
    uint32 index = 1;

    if (NULL == g_p_entPhy_tbl)
    {
        return -1;
    }

    row = entPhy_table_get_row(index);
    if (NULL == row)
    {
        row = netsnmp_create_table_data_row();
    }
    netsnmp_table_row_add_index(row, ASN_INTEGER, &index, sizeof(index));

    netsnmp_set_row_column(row, ENTPHY_TBL_SERIAL_NO, ASN_OCTET_STR, p_card->serial_no, sal_strlen(p_card->serial_no));

    netsnmp_table_data_add_row(g_p_entPhy_tbl->p_dataset->table, row);

    return 0;

}

int32
card_set_row(tbl_card_t *p_card, uint32 field_id)
{
    netsnmp_table_row *row = NULL;
    uint32 index = 1;

    if (NULL == g_p_entPhy_tbl)
    {
        return -1;
    }

    row = entPhy_table_get_row(index);
    if (NULL == row)
    {
        return -1;
    }
    switch (field_id)
    {
        case TBL_CARD_FLD_SERIAL_NO:
            netsnmp_set_row_column(row, ENTPHY_TBL_SERIAL_NO, ASN_OCTET_STR, p_card->serial_no, sal_strlen(p_card->serial_no));
        default:
            break;
    }
    return 0;
}

int32
mib2_entPhysicalTable_init()
{
    snmpagt_table_param_t param;
    snmpagt_table_t *table = NULL;
    char oid_str[CMD_BUF_256];

    snmpagt_oid2str(ENTPHY_TABLE_OID, sizeof(ENTPHY_TABLE_OID)/sizeof(oid), oid_str, CMD_BUF_256);

    sal_memset(&param, 0, sizeof(param));
    param.oid_str = oid_str;
    param.no_idx_col = TRUE;
    param.index_type[0] = ASN_INTEGER;
    param.index_count = 1;
    param.row = ENTPHY_TBL_ROW;
    param.row_count = sizeof(ENTPHY_TBL_ROW)/sizeof(snmpagt_row_t);
    param.nodehandler = entPhy_table_handler;
    table = snmpagt_table_create(&param);
    if (NULL == table)
    {
        return -1;
    }

    g_p_entPhy_tbl = table;

    return 0;
}



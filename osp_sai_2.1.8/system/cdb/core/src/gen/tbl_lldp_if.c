
#include "proto.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/ds_lldp_neighbour_define.h"
#include "gen/ds_lldp_neighbour.h"
#include "cdb_data_cmp.h"

tbl_lldp_if_master_t *_g_p_tbl_lldp_if_master = NULL;

int32
tbl_lldp_if_add_lldp_if_sync(tbl_lldp_if_t *p_lldp_if, uint32 sync)
{
    tbl_lldp_if_master_t *p_master = _g_p_tbl_lldp_if_master;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_lldp_if_get_lldp_if(&p_lldp_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lldp_if = XCALLOC(MEM_TBL_LLDP_IF, sizeof(tbl_lldp_if_t));
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lldp_if, p_lldp_if, sizeof(tbl_lldp_if_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_lldp_if->neigh_list, CDB_NODE_DS, DS_LLDP_NEIGHBOUR, ds_lldp_neighbour_cmp));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_lldp_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LLDP_IF, p_db_lldp_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LLDP_IF, p_db_lldp_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lldp_if_del_lldp_if_sync(tbl_lldp_if_key_t *p_lldp_if_key, uint32 sync)
{
    tbl_lldp_if_master_t *p_master = _g_p_tbl_lldp_if_master;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(p_lldp_if_key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LLDP_IF, p_db_lldp_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LLDP_IF, p_db_lldp_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->if_list, p_db_lldp_if);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_lldp_if->neigh_list));

    /* 4. free db entry */
    XFREE(MEM_TBL_LLDP_IF, p_db_lldp_if);

    return PM_E_NONE;
}

int32
tbl_lldp_if_set_lldp_if_field_sync(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_field_id_t field_id, uint32 sync)
{
    tbl_lldp_if_t *p_db_lldp_if = NULL;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LLDP_IF_FLD_MODE:
        p_db_lldp_if->mode = p_lldp_if->mode;
        break;

    case TBL_LLDP_IF_FLD_TX_TTL:
        p_db_lldp_if->tx_ttl = p_lldp_if->tx_ttl;
        break;

    case TBL_LLDP_IF_FLD_TX_COUNT:
        p_db_lldp_if->tx_count = p_lldp_if->tx_count;
        break;

    case TBL_LLDP_IF_FLD_RX_COUNT:
        p_db_lldp_if->rx_count = p_lldp_if->rx_count;
        break;

    case TBL_LLDP_IF_FLD_RX_ERROR_COUNT:
        p_db_lldp_if->rx_error_count = p_lldp_if->rx_error_count;
        break;

    case TBL_LLDP_IF_FLD_RX_DISCARD_COUNT:
        p_db_lldp_if->rx_discard_count = p_lldp_if->rx_discard_count;
        break;

    case TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT:
        p_db_lldp_if->tlv_discard_count = p_lldp_if->tlv_discard_count;
        break;

    case TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT:
        p_db_lldp_if->tlv_unrecognized_count = p_lldp_if->tlv_unrecognized_count;
        break;

    case TBL_LLDP_IF_FLD_AGED_COUNT:
        p_db_lldp_if->aged_count = p_lldp_if->aged_count;
        break;

    case TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC:
        p_db_lldp_if->tlv_enable_basic = p_lldp_if->tlv_enable_basic;
        break;

    case TBL_LLDP_IF_FLD_TLV_ENABLE_8021:
        p_db_lldp_if->tlv_enable_8021 = p_lldp_if->tlv_enable_8021;
        break;

    case TBL_LLDP_IF_FLD_TLV_ENABLE_8023:
        p_db_lldp_if->tlv_enable_8023 = p_lldp_if->tlv_enable_8023;
        break;

    case TBL_LLDP_IF_FLD_TLV_ENABLE_MED:
        p_db_lldp_if->tlv_enable_med = p_lldp_if->tlv_enable_med;
        break;

    case TBL_LLDP_IF_FLD_PPVID_FLAG:
        p_db_lldp_if->ppvid_flag = p_lldp_if->ppvid_flag;
        break;

    case TBL_LLDP_IF_FLD_PPVID:
        p_db_lldp_if->ppvid = p_lldp_if->ppvid;
        break;

    case TBL_LLDP_IF_FLD_PROTOCOL_ID:
        p_db_lldp_if->protocol_id = p_lldp_if->protocol_id;
        break;

    case TBL_LLDP_IF_FLD_VLAN_NAME:
        sal_memcpy(p_db_lldp_if->vlan_name, p_lldp_if->vlan_name, sizeof(p_lldp_if->vlan_name));
        break;

    case TBL_LLDP_IF_FLD_VLAN_NAME_ID:
        p_db_lldp_if->vlan_name_id = p_lldp_if->vlan_name_id;
        break;

    case TBL_LLDP_IF_FLD_MDI_SUPPORT:
        p_db_lldp_if->mdi_support = p_lldp_if->mdi_support;
        break;

    case TBL_LLDP_IF_FLD_PSE_POWER_PAIR:
        p_db_lldp_if->pse_power_pair = p_lldp_if->pse_power_pair;
        break;

    case TBL_LLDP_IF_FLD_POWER_CLASS:
        p_db_lldp_if->power_class = p_lldp_if->power_class;
        break;

    case TBL_LLDP_IF_FLD_MED_CAPBILITY:
        p_db_lldp_if->med_capbility = p_lldp_if->med_capbility;
        break;

    case TBL_LLDP_IF_FLD_MED_DEV_TYPE:
        p_db_lldp_if->med_dev_type = p_lldp_if->med_dev_type;
        break;

    case TBL_LLDP_IF_FLD_MED_POWER_FLAG:
        p_db_lldp_if->med_power_flag = p_lldp_if->med_power_flag;
        break;

    case TBL_LLDP_IF_FLD_MED_POWER_VALUE:
        p_db_lldp_if->med_power_value = p_lldp_if->med_power_value;
        break;

    case TBL_LLDP_IF_FLD_MED_POLICY_FLAG:
        p_db_lldp_if->med_policy_flag = p_lldp_if->med_policy_flag;
        break;

    case TBL_LLDP_IF_FLD_MED_LCI_FORMAT:
        p_db_lldp_if->med_lci_format = p_lldp_if->med_lci_format;
        break;

    case TBL_LLDP_IF_FLD_MED_LCI_COORDINATE:
        sal_memcpy(p_db_lldp_if->med_lci_coordinate, p_lldp_if->med_lci_coordinate, sizeof(p_lldp_if->med_lci_coordinate));
        break;

    case TBL_LLDP_IF_FLD_MED_LCI_ELIN:
        sal_memcpy(p_db_lldp_if->med_lci_elin, p_lldp_if->med_lci_elin, sizeof(p_lldp_if->med_lci_elin));
        break;

    case TBL_LLDP_IF_FLD_MED_LCI_CIVIC:
        sal_memcpy(&p_db_lldp_if->med_lci_civic, &p_lldp_if->med_lci_civic, sizeof(p_lldp_if->med_lci_civic));
        break;

    case TBL_LLDP_IF_FLD_TX_TIMER:
        p_db_lldp_if->tx_timer = p_lldp_if->tx_timer;
        break;

    case TBL_LLDP_IF_FLD_MAX:
        sal_memcpy(p_db_lldp_if, p_lldp_if, sizeof(tbl_lldp_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LLDP_IF, field_id, p_db_lldp_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LLDP_IF, field_id, p_db_lldp_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lldp_if_add_lldp_if(tbl_lldp_if_t *p_lldp_if)
{
    return tbl_lldp_if_add_lldp_if_sync(p_lldp_if, TRUE);
}

int32
tbl_lldp_if_del_lldp_if(tbl_lldp_if_key_t *p_lldp_if_key)
{
    return tbl_lldp_if_del_lldp_if_sync(p_lldp_if_key, TRUE);
}

int32
tbl_lldp_if_set_lldp_if_field(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_field_id_t field_id)
{
    return tbl_lldp_if_set_lldp_if_field_sync(p_lldp_if, field_id, TRUE);
}

tbl_lldp_if_t*
tbl_lldp_if_get_lldp_if(tbl_lldp_if_key_t *p_lldp_if_key)
{
    tbl_lldp_if_master_t *p_master = _g_p_tbl_lldp_if_master;
    tbl_lldp_if_t lkp;

    sal_memcpy(&lkp.key, p_lldp_if_key, sizeof(tbl_lldp_if_key_t));
    return ctclib_slistdata_lookup(p_master->if_list, &lkp);
}

char*
tbl_lldp_if_key_val2str(tbl_lldp_if_t *p_lldp_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_lldp_if->key.ifindex);
    return str;
}

int32
tbl_lldp_if_key_str2val(char *str, tbl_lldp_if_t *p_lldp_if)
{
    int32 ret = 0;

    p_lldp_if->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_lldp_if_key_name_dump(tbl_lldp_if_t *p_lldp_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_lldp_if->key.ifindex);
    return str;
}

char*
tbl_lldp_if_key_value_dump(tbl_lldp_if_t *p_lldp_if, char *str)
{
    sal_sprintf(str, "%u", p_lldp_if->key.ifindex);
    return str;
}

char*
tbl_lldp_if_field_name_dump(tbl_lldp_if_t *p_lldp_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LLDP_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lldp_if_field_value_dump(tbl_lldp_if_t *p_lldp_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LLDP_IF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->key.ifindex);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(lldp_if_mode_strs, LLDP_IF_MODE_MAX, p_lldp_if->mode));
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tx_ttl);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tx_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->rx_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_ERROR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->rx_error_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_DISCARD_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->rx_discard_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_discard_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_unrecognized_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_AGED_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->aged_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_enable_basic);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8021, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_enable_8021);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8023, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_enable_8023);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_MED, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->tlv_enable_med);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->ppvid_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->ppvid);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PROTOCOL_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->protocol_id);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_if->vlan_name);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->vlan_name_id);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MDI_SUPPORT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->mdi_support);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PSE_POWER_PAIR, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->pse_power_pair);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_POWER_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->power_class);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_CAPBILITY, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_capbility);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_DEV_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_dev_type);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_power_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_VALUE, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_power_value);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POLICY_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_policy_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_FORMAT, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_if->med_lci_format);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_COORDINATE, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_if->med_lci_coordinate);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_ELIN, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_if->med_lci_elin);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_CIVIC, field_id))
    {
        cdb_lldp_civic_address_t_val2str(str, MAX_CMD_STR_LEN, &p_lldp_if->med_lci_civic);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_lldp_if->tx_timer);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_NEIGH_LIST, field_id))
    {
    }
    return str;
}

char**
tbl_lldp_if_table_dump(tbl_lldp_if_t *p_lldp_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lldp_if_key_name_dump(p_lldp_if, buf));
    for(i=1; i<TBL_LLDP_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lldp_if_field_name_dump(p_lldp_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lldp_if_field_value_dump(p_lldp_if, i, buf));
    }
    return str;
}

int32
tbl_lldp_if_field_value_parser(char *str, int32 field_id, tbl_lldp_if_t *p_lldp_if)
{
    if (FLD_MATCH(TBL_LLDP_IF_FLD_KEY, field_id))
    {
        int32 ret;
        p_lldp_if->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MODE, field_id))
    {
        cdb_enum_str2val(lldp_if_mode_strs, LLDP_IF_MODE_MAX, str);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TTL, field_id))
    {
        int32 ret;
        p_lldp_if->tx_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->tx_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->rx_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_ERROR_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->rx_error_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_DISCARD_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->rx_discard_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_discard_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_unrecognized_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_AGED_COUNT, field_id))
    {
        int32 ret;
        p_lldp_if->aged_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_enable_basic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8021, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_enable_8021 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8023, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_enable_8023 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_MED, field_id))
    {
        int32 ret;
        p_lldp_if->tlv_enable_med = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID_FLAG, field_id))
    {
        int32 ret;
        p_lldp_if->ppvid_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID, field_id))
    {
        int32 ret;
        p_lldp_if->ppvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PROTOCOL_ID, field_id))
    {
        int32 ret;
        p_lldp_if->protocol_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME, field_id))
    {
        sal_strcpy(p_lldp_if->vlan_name, str);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME_ID, field_id))
    {
        int32 ret;
        p_lldp_if->vlan_name_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MDI_SUPPORT, field_id))
    {
        int32 ret;
        p_lldp_if->mdi_support = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PSE_POWER_PAIR, field_id))
    {
        int32 ret;
        p_lldp_if->pse_power_pair = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_POWER_CLASS, field_id))
    {
        int32 ret;
        p_lldp_if->power_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_CAPBILITY, field_id))
    {
        int32 ret;
        p_lldp_if->med_capbility = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_DEV_TYPE, field_id))
    {
        int32 ret;
        p_lldp_if->med_dev_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_FLAG, field_id))
    {
        int32 ret;
        p_lldp_if->med_power_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_VALUE, field_id))
    {
        int32 ret;
        p_lldp_if->med_power_value = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POLICY_FLAG, field_id))
    {
        int32 ret;
        p_lldp_if->med_policy_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_FORMAT, field_id))
    {
        int32 ret;
        p_lldp_if->med_lci_format = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_COORDINATE, field_id))
    {
        sal_strcpy(p_lldp_if->med_lci_coordinate, str);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_ELIN, field_id))
    {
        sal_strcpy(p_lldp_if->med_lci_elin, str);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_CIVIC, field_id))
    {
        cdb_lldp_civic_address_t_str2val(str, &p_lldp_if->med_lci_civic);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TIMER, field_id))
    {
        ctc_task_str2val(str, p_lldp_if->tx_timer);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_NEIGH_LIST, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_lldp_if_table_parser(char** array, char* key_value,tbl_lldp_if_t *p_lldp_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lldp_if_key_str2val(key_value, p_lldp_if));

    for(i=1; i<TBL_LLDP_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lldp_if_field_value_parser( array[j++], i, p_lldp_if));
    }

    return PM_E_NONE;
}

int32
tbl_lldp_if_dump_one(tbl_lldp_if_t *p_lldp_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_lldp_neighbour_t *p_db_lldp_neighbour = NULL;
    ctclib_slistnode_t *listnode = NULL;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LLDP_IF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_KEY].name,
            p_lldp_if->key.ifindex);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_MODE].name, 
            cdb_enum_val2str(lldp_if_mode_strs, LLDP_IF_MODE_MAX, p_lldp_if->mode));
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TTL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TX_TTL].name,
            p_lldp_if->tx_ttl);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TX_COUNT].name,
            p_lldp_if->tx_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_RX_COUNT].name,
            p_lldp_if->rx_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_ERROR_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_RX_ERROR_COUNT].name,
            p_lldp_if->rx_error_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_RX_DISCARD_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_RX_DISCARD_COUNT].name,
            p_lldp_if->rx_discard_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT].name,
            p_lldp_if->tlv_discard_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT].name,
            p_lldp_if->tlv_unrecognized_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_AGED_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_AGED_COUNT].name,
            p_lldp_if->aged_count);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC].name,
            p_lldp_if->tlv_enable_basic);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8021, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_ENABLE_8021].name,
            p_lldp_if->tlv_enable_8021);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_8023, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_ENABLE_8023].name,
            p_lldp_if->tlv_enable_8023);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TLV_ENABLE_MED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_TLV_ENABLE_MED].name,
            p_lldp_if->tlv_enable_med);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_PPVID_FLAG].name,
            p_lldp_if->ppvid_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PPVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_PPVID].name,
            p_lldp_if->ppvid);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PROTOCOL_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_PROTOCOL_ID].name,
            p_lldp_if->protocol_id);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_VLAN_NAME].name,
            p_lldp_if->vlan_name);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_VLAN_NAME_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_VLAN_NAME_ID].name,
            p_lldp_if->vlan_name_id);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MDI_SUPPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MDI_SUPPORT].name,
            p_lldp_if->mdi_support);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_PSE_POWER_PAIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_PSE_POWER_PAIR].name,
            p_lldp_if->pse_power_pair);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_POWER_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_POWER_CLASS].name,
            p_lldp_if->power_class);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_CAPBILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_CAPBILITY].name,
            p_lldp_if->med_capbility);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_DEV_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_DEV_TYPE].name,
            p_lldp_if->med_dev_type);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_POWER_FLAG].name,
            p_lldp_if->med_power_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POWER_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_POWER_VALUE].name,
            p_lldp_if->med_power_value);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_POLICY_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_POLICY_FLAG].name,
            p_lldp_if->med_policy_flag);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_FORMAT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_LCI_FORMAT].name,
            p_lldp_if->med_lci_format);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_COORDINATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_LCI_COORDINATE].name,
            p_lldp_if->med_lci_coordinate);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_ELIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_LCI_ELIN].name,
            p_lldp_if->med_lci_elin);
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_MED_LCI_CIVIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_MED_LCI_CIVIC].name, 
            cdb_lldp_civic_address_t_val2str(buf, MAX_CMD_STR_LEN, &p_lldp_if->med_lci_civic));
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_TX_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_IF_FLD_TX_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_lldp_if->tx_timer));
    }
    if (FLD_MATCH(TBL_LLDP_IF_FLD_NEIGH_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_LLDP_IF_FLD_NEIGH_LIST].name);
        if (p_lldp_if->neigh_list.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_lldp_if->neigh_list.obj_list, p_db_lldp_neighbour, listnode)
            {
                ds_lldp_neighbour_dump_one(p_db_lldp_neighbour, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lldp_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lldp_if_master_t *p_master = _g_p_tbl_lldp_if_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_lldp_if, listnode, next)
    {
        rc |= fn(p_db_lldp_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lldp_if_master_t*
tbl_lldp_if_get_master()
{
    return _g_p_tbl_lldp_if_master;
}

tbl_lldp_if_master_t*
tbl_lldp_if_init_lldp_if()
{
    _g_p_tbl_lldp_if_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_lldp_if_master_t));
    _g_p_tbl_lldp_if_master->if_list = ctclib_slist_create(tbl_lldp_if_cmp, NULL);
    return _g_p_tbl_lldp_if_master;
}



#include "proto.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "cdb_data_cmp.h"

int32
ds_lldp_neighbour_add_lldp_neighbour_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, uint32 sync)
{
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ds_lldp_neighbour_t *p_db_neigh = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_neigh = ds_lldp_neighbour_get_lldp_neighbour(p_lldp_if, p_neigh);
    if (p_db_neigh)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_neigh = XCALLOC(MEM_DS_LLDP_NEIGHBOUR, sizeof(ds_lldp_neighbour_t));
    if (NULL == p_db_neigh)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_neigh, p_neigh, sizeof(ds_lldp_neighbour_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_lldp_if->neigh_list, p_db_neigh));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, p_db_lldp_if, p_db_neigh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, p_db_lldp_if, p_db_neigh);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lldp_neighbour_del_lldp_neighbour_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, uint32 sync)
{
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ds_lldp_neighbour_t *p_db_neigh = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_neigh = ds_lldp_neighbour_get_lldp_neighbour(p_lldp_if, p_neigh);
    if (NULL == p_db_neigh)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, p_db_lldp_if, p_db_neigh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, p_db_lldp_if, p_db_neigh);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_lldp_if->neigh_list, p_db_neigh));

    /* 4. free db entry */
    XFREE(MEM_DS_LLDP_NEIGHBOUR, p_db_neigh);

    return PM_E_NONE;
}

int32
ds_lldp_neighbour_set_lldp_neighbour_field_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, ds_lldp_neighbour_field_id_t field_id, uint32 sync)
{
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ds_lldp_neighbour_t *p_db_neigh = NULL;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_neigh = ds_lldp_neighbour_get_lldp_neighbour(p_lldp_if, p_neigh);
    if (NULL == p_db_neigh)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX:
        p_db_neigh->rx_ifindex = p_neigh->rx_ifindex;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR:
        sal_memcpy(p_db_neigh->mac_addr, p_neigh->mac_addr, sizeof(p_neigh->mac_addr));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_RX_TTL:
        p_db_neigh->rx_ttl = p_neigh->rx_ttl;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE:
        p_db_neigh->chassis_id_sub_type = p_neigh->chassis_id_sub_type;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE:
        p_db_neigh->port_id_sub_type = p_neigh->port_id_sub_type;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME:
        sal_memcpy(p_db_neigh->system_name, p_neigh->system_name, sizeof(p_neigh->system_name));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC:
        sal_memcpy(p_db_neigh->system_desc, p_neigh->system_desc, sizeof(p_neigh->system_desc));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PORT_DESC:
        sal_memcpy(p_db_neigh->port_desc, p_neigh->port_desc, sizeof(p_neigh->port_desc));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_IFNAME:
        sal_memcpy(p_db_neigh->ifname, p_neigh->ifname, sizeof(p_neigh->ifname));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME:
        sal_memcpy(p_db_neigh->vlan_name, p_neigh->vlan_name, sizeof(p_neigh->vlan_name));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_VLAN_ID:
        p_db_neigh->vlan_id = p_neigh->vlan_id;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_SYS_CAP:
        p_db_neigh->sys_cap = p_neigh->sys_cap;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN:
        p_db_neigh->sys_cap_en = p_neigh->sys_cap_en;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR:
        sal_memcpy(p_db_neigh->mant_addr, p_neigh->mant_addr, sizeof(p_neigh->mant_addr));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_OID:
        sal_memcpy(p_db_neigh->oid, p_neigh->oid, sizeof(p_neigh->oid));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_OID_LEN:
        p_db_neigh->oid_len = p_neigh->oid_len;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE:
        p_db_neigh->mgmt_addr_sub_type = p_neigh->mgmt_addr_sub_type;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING:
        p_db_neigh->if_numbering = p_neigh->if_numbering;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER:
        p_db_neigh->if_number = p_neigh->if_number;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PVID:
        p_db_neigh->pvid = p_neigh->pvid;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG:
        p_db_neigh->ppvid_flag = p_neigh->ppvid_flag;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PPVID:
        p_db_neigh->ppvid = p_neigh->ppvid;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID:
        p_db_neigh->protocol_id = p_neigh->protocol_id;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT:
        p_db_neigh->autonego_support = p_neigh->autonego_support;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP:
        p_db_neigh->autonego_cap = p_neigh->autonego_cap;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE:
        p_db_neigh->oper_mau_type = p_neigh->oper_mau_type;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS:
        p_db_neigh->link_aggr_status = p_neigh->link_aggr_status;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID:
        p_db_neigh->link_aggr_id = p_neigh->link_aggr_id;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE:
        p_db_neigh->max_frame_size = p_neigh->max_frame_size;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG:
        p_db_neigh->power_flag = p_neigh->power_flag;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR:
        p_db_neigh->power_pair = p_neigh->power_pair;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS:
        p_db_neigh->power_class = p_neigh->power_class;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY:
        p_db_neigh->med_capbility = p_neigh->med_capbility;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE:
        p_db_neigh->med_dev_type = p_neigh->med_dev_type;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG:
        p_db_neigh->med_policy_flag = p_neigh->med_policy_flag;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG:
        p_db_neigh->med_power_flag = p_neigh->med_power_flag;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE:
        p_db_neigh->med_power_value = p_neigh->med_power_value;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT:
        p_db_neigh->med_lci_format = p_neigh->med_lci_format;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER:
        sal_memcpy(p_db_neigh->med_hard_ver, p_neigh->med_hard_ver, sizeof(p_neigh->med_hard_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER:
        sal_memcpy(p_db_neigh->med_firm_ver, p_neigh->med_firm_ver, sizeof(p_neigh->med_firm_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER:
        sal_memcpy(p_db_neigh->med_soft_ver, p_neigh->med_soft_ver, sizeof(p_neigh->med_soft_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER:
        sal_memcpy(p_db_neigh->med_serial_ver, p_neigh->med_serial_ver, sizeof(p_neigh->med_serial_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER:
        sal_memcpy(p_db_neigh->med_manufac_ver, p_neigh->med_manufac_ver, sizeof(p_neigh->med_manufac_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER:
        sal_memcpy(p_db_neigh->med_moname_ver, p_neigh->med_moname_ver, sizeof(p_neigh->med_moname_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER:
        sal_memcpy(p_db_neigh->med_asset_ver, p_neigh->med_asset_ver, sizeof(p_neigh->med_asset_ver));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE:
        sal_memcpy(p_db_neigh->med_lci_coordinate, p_neigh->med_lci_coordinate, sizeof(p_neigh->med_lci_coordinate));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN:
        sal_memcpy(p_db_neigh->med_lci_elin, p_neigh->med_lci_elin, sizeof(p_neigh->med_lci_elin));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC:
        sal_memcpy(&p_db_neigh->med_lci_civic, &p_neigh->med_lci_civic, sizeof(p_neigh->med_lci_civic));
        break;

    case DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER:
        p_db_neigh->rx_ttl_timer = p_neigh->rx_ttl_timer;
        break;

    case DS_LLDP_NEIGHBOUR_FLD_MAX:
        sal_memcpy(p_db_neigh, p_neigh, sizeof(ds_lldp_neighbour_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, field_id, p_db_lldp_if, p_db_neigh);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_LLDP_IF, DS_LLDP_NEIGHBOUR, field_id, p_db_lldp_if, p_db_neigh);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lldp_neighbour_add_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    return ds_lldp_neighbour_add_lldp_neighbour_sync(p_lldp_if, p_neigh, TRUE);
}

int32
ds_lldp_neighbour_del_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    return ds_lldp_neighbour_del_lldp_neighbour_sync(p_lldp_if, p_neigh, TRUE);
}

int32
ds_lldp_neighbour_set_lldp_neighbour_field(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, ds_lldp_neighbour_field_id_t field_id)
{
    return ds_lldp_neighbour_set_lldp_neighbour_field_sync(p_lldp_if, p_neigh, field_id, TRUE);
}

ds_lldp_neighbour_t*
ds_lldp_neighbour_get_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    tbl_lldp_if_t *p_db_lldp_if = NULL;

    /* 1. lookup entry exist */
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_lldp_if->neigh_list, p_neigh);
}

int32
ds_lldp_neighbour_key_str2val(char* tbl_key_value, char* str, tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{

    int32 ret = 0;

    ret = tbl_lldp_if_key_str2val(tbl_key_value, p_lldp_if);

    ret = cdb_lldp_msap_id_str2val(str, &p_neigh->key);

    return ret;
}

char*
ds_lldp_neighbour_key_value_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char* str)
{


    cdb_lldp_msap_id_val2str(str, MAX_CMD_STR_LEN, &p_neigh->key);
    return str;
}

char*
ds_lldp_neighbour_key_name_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_LLDP_IF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_LLDP_NEIGHBOUR);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_lldp_if_key_value_dump(p_lldp_if, tbl_str), p_node_ds->name, ds_lldp_neighbour_key_value_dump(p_lldp_if, p_neigh, ds_str));
    return str;
}

char*
ds_lldp_neighbour_field_name_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_LLDP_NEIGHBOUR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_LLDP_NEIGHBOUR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_lldp_neighbour_field_value_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_KEY, field_id))
    {
        cdb_lldp_msap_id_val2str(str, MAX_CMD_STR_LEN, &p_neigh->key);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->rx_ifindex);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_neigh->mac_addr);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->rx_ttl);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->chassis_id_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->port_id_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->system_name);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->system_desc);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->port_desc);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->ifname);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->vlan_name);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->vlan_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->sys_cap);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->sys_cap_en);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->mant_addr);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->oid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->oid_len);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->mgmt_addr_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->if_numbering);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->if_number);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PVID, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->pvid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->ppvid_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->ppvid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->protocol_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->autonego_support);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->autonego_cap);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->oper_mau_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->link_aggr_status);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->link_aggr_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->max_frame_size);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->power_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->power_pair);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->power_class);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_capbility);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_dev_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_policy_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_power_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_power_value);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT, field_id))
    {
        sal_sprintf(str, "%u", p_neigh->med_lci_format);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_hard_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_firm_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_soft_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_serial_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_manufac_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_moname_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_asset_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_lci_coordinate);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN, field_id))
    {
        sal_sprintf(str, "%s", p_neigh->med_lci_elin);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC, field_id))
    {
        cdb_lldp_civic_address_t_val2str(str, MAX_CMD_STR_LEN, &p_neigh->med_lci_civic);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_neigh->rx_ttl_timer);
    }
    return str;
}

char**
ds_lldp_neighbour_table_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_lldp_neighbour_key_name_dump(p_lldp_if, p_neigh, buf));
    for(i=1; i<DS_LLDP_NEIGHBOUR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_lldp_neighbour_field_name_dump(p_lldp_if, p_neigh, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_lldp_neighbour_field_value_dump(p_lldp_if, p_neigh, i, buf));
    }
    return str;
}

int32
ds_lldp_neighbour_field_value_parser(char *str, int32 field_id, tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    int32 ret=0;
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_KEY, field_id))
    {
        cdb_lldp_msap_id_str2val(str, &p_neigh->key);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX, field_id))
    {
        p_neigh->rx_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_neigh->mac_addr, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL, field_id))
    {
        p_neigh->rx_ttl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE, field_id))
    {
        p_neigh->chassis_id_sub_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE, field_id))
    {
        p_neigh->port_id_sub_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME, field_id))
    {
        sal_strcpy(p_neigh->system_name, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC, field_id))
    {
        sal_strcpy(p_neigh->system_desc, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_DESC, field_id))
    {
        sal_strcpy(p_neigh->port_desc, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_neigh->ifname, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME, field_id))
    {
        sal_strcpy(p_neigh->vlan_name, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_ID, field_id))
    {
        p_neigh->vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP, field_id))
    {
        p_neigh->sys_cap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN, field_id))
    {
        p_neigh->sys_cap_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR, field_id))
    {
        sal_strcpy(p_neigh->mant_addr, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID, field_id))
    {
        sal_strcpy(p_neigh->oid, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID_LEN, field_id))
    {
        p_neigh->oid_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE, field_id))
    {
        p_neigh->mgmt_addr_sub_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING, field_id))
    {
        p_neigh->if_numbering = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER, field_id))
    {
        p_neigh->if_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PVID, field_id))
    {
        p_neigh->pvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG, field_id))
    {
        p_neigh->ppvid_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID, field_id))
    {
        p_neigh->ppvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID, field_id))
    {
        p_neigh->protocol_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT, field_id))
    {
        p_neigh->autonego_support = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP, field_id))
    {
        p_neigh->autonego_cap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE, field_id))
    {
        p_neigh->oper_mau_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS, field_id))
    {
        p_neigh->link_aggr_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID, field_id))
    {
        p_neigh->link_aggr_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE, field_id))
    {
        p_neigh->max_frame_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG, field_id))
    {
        p_neigh->power_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR, field_id))
    {
        p_neigh->power_pair = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS, field_id))
    {
        p_neigh->power_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY, field_id))
    {
        p_neigh->med_capbility = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE, field_id))
    {
        p_neigh->med_dev_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG, field_id))
    {
        p_neigh->med_policy_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG, field_id))
    {
        p_neigh->med_power_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE, field_id))
    {
        p_neigh->med_power_value = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT, field_id))
    {
        p_neigh->med_lci_format = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER, field_id))
    {
        sal_strcpy(p_neigh->med_hard_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER, field_id))
    {
        sal_strcpy(p_neigh->med_firm_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER, field_id))
    {
        sal_strcpy(p_neigh->med_soft_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER, field_id))
    {
        sal_strcpy(p_neigh->med_serial_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER, field_id))
    {
        sal_strcpy(p_neigh->med_manufac_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER, field_id))
    {
        sal_strcpy(p_neigh->med_moname_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER, field_id))
    {
        sal_strcpy(p_neigh->med_asset_ver, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE, field_id))
    {
        sal_strcpy(p_neigh->med_lci_coordinate, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN, field_id))
    {
        sal_strcpy(p_neigh->med_lci_elin, str);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC, field_id))
    {
        cdb_lldp_civic_address_t_str2val(str, &p_neigh->med_lci_civic);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER, field_id))
    {
        ctc_task_str2val(str, p_neigh->rx_ttl_timer);
    }
    return ret;
}

int32
ds_lldp_neighbour_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_LLDP_NEIGHBOUR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_lldp_neighbour_key_str2val(tbl_key_value, ds_key_value, p_lldp_if, p_neigh));

    for(i=1; i<DS_LLDP_NEIGHBOUR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_lldp_neighbour_field_value_parser( array[j++], i, p_lldp_if, p_neigh));
    }

    return PM_E_NONE;
}

int32
ds_lldp_neighbour_dump_one(ds_lldp_neighbour_t *p_neigh, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_LLDP_NEIGHBOUR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_KEY].name,
            cdb_lldp_msap_id_val2str(buf, MAX_CMD_STR_LEN, &p_neigh->key));
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX].name,
            p_neigh->rx_ifindex);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR].name,
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_neigh->mac_addr));
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_RX_TTL].name,
            p_neigh->rx_ttl);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE].name,
            p_neigh->chassis_id_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE].name,
            p_neigh->port_id_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME].name,
            p_neigh->system_name);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC].name,
            p_neigh->system_desc);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PORT_DESC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PORT_DESC].name,
            p_neigh->port_desc);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IFNAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_IFNAME].name,
            p_neigh->ifname);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME].name,
            p_neigh->vlan_name);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_VLAN_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_VLAN_ID].name,
            p_neigh->vlan_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_SYS_CAP].name,
            p_neigh->sys_cap);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN].name,
            p_neigh->sys_cap_en);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR].name,
            p_neigh->mant_addr);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_OID].name,
            p_neigh->oid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OID_LEN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_OID_LEN].name,
            p_neigh->oid_len);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE].name,
            p_neigh->mgmt_addr_sub_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING].name,
            p_neigh->if_numbering);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER].name,
            p_neigh->if_number);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PVID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PVID].name,
            p_neigh->pvid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG].name,
            p_neigh->ppvid_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PPVID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PPVID].name,
            p_neigh->ppvid);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID].name,
            p_neigh->protocol_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT].name,
            p_neigh->autonego_support);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP].name,
            p_neigh->autonego_cap);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE].name,
            p_neigh->oper_mau_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS].name,
            p_neigh->link_aggr_status);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID].name,
            p_neigh->link_aggr_id);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE].name,
            p_neigh->max_frame_size);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG].name,
            p_neigh->power_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR].name,
            p_neigh->power_pair);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS].name,
            p_neigh->power_class);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY].name,
            p_neigh->med_capbility);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE].name,
            p_neigh->med_dev_type);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG].name,
            p_neigh->med_policy_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG].name,
            p_neigh->med_power_flag);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE].name,
            p_neigh->med_power_value);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT].name,
            p_neigh->med_lci_format);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER].name,
            p_neigh->med_hard_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER].name,
            p_neigh->med_firm_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER].name,
            p_neigh->med_soft_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER].name,
            p_neigh->med_serial_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER].name,
            p_neigh->med_manufac_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER].name,
            p_neigh->med_moname_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER].name,
            p_neigh->med_asset_ver);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE].name,
            p_neigh->med_lci_coordinate);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN].name,
            p_neigh->med_lci_elin);
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC].name,
            cdb_lldp_civic_address_t_val2str(buf, MAX_CMD_STR_LEN, &p_neigh->med_lci_civic));
    }
    if (FLD_MATCH(DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_neigh->rx_ttl_timer));
    }

    return PM_E_NONE;
}

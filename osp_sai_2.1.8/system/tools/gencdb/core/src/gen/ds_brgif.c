
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_brgif_add_brgif_sync(tbl_interface_t *p_if, ds_brgif_t *p_brgif, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_brgif = p_db_if->brgif;
    if (p_db_brgif)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_brgif = XCALLOC(MEM_DS_BRGIF, sizeof(ds_brgif_t));
    if (NULL == p_db_brgif)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_brgif, p_brgif, sizeof(ds_brgif_t));

    /* 4. add to db */
    p_db_if->brgif = p_db_brgif;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_BRGIF, p_db_if, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_BRGIF, p_db_if, p_db_brgif);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_brgif_del_brgif_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_BRGIF, p_db_if, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_BRGIF, p_db_if, p_db_brgif);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->brgif = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_BRGIF, p_db_brgif);

    return PM_E_NONE;
}

int32
ds_brgif_set_brgif_field_sync(tbl_interface_t *p_if, ds_brgif_t *p_brgif, ds_brgif_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_brgif = ds_brgif_get_brgif(p_if);
    if (NULL == p_db_brgif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_BRGIF_FLD_PVID:
        p_db_brgif->pvid = p_brgif->pvid;
        break;

    case DS_BRGIF_FLD_INNER_NATIVE_VID:
        p_db_brgif->inner_native_vid = p_brgif->inner_native_vid;
        break;

    case DS_BRGIF_FLD_QINQ_TYPE:
        p_db_brgif->qinq_type = p_brgif->qinq_type;
        break;

    case DS_BRGIF_FLD_QINQ_DEFAULT_FWD:
        p_db_brgif->qinq_default_fwd = p_brgif->qinq_default_fwd;
        break;

    case DS_BRGIF_FLD_VLAN_TRANSLATION_EN:
        p_db_brgif->vlan_translation_en = p_brgif->vlan_translation_en;
        break;

    case DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD:
        p_db_brgif->vlan_trans_default_fwd = p_brgif->vlan_trans_default_fwd;
        break;

    case DS_BRGIF_FLD_VLAN_MAPPING_TABLE:
        sal_memcpy(p_db_brgif->vlan_mapping_table, p_brgif->vlan_mapping_table, sizeof(p_brgif->vlan_mapping_table));
        break;

    case DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN:
        p_db_brgif->dot1q_tag_native_en = p_brgif->dot1q_tag_native_en;
        break;

    case DS_BRGIF_FLD_ALLOWED_VLAN:
        sal_memcpy(&p_db_brgif->allowed_vlan, &p_brgif->allowed_vlan, sizeof(p_brgif->allowed_vlan));
        break;

    case DS_BRGIF_FLD_INGRESS_FILTER:
        p_db_brgif->ingress_filter = p_brgif->ingress_filter;
        break;

    case DS_BRGIF_FLD_EGRESS_FILTER:
        p_db_brgif->egress_filter = p_brgif->egress_filter;
        break;

    case DS_BRGIF_FLD_TPID_TYPE:
        p_db_brgif->tpid_type = p_brgif->tpid_type;
        break;

    case DS_BRGIF_FLD_MAC_LEARNING_EN:
        p_db_brgif->mac_learning_en = p_brgif->mac_learning_en;
        break;

    case DS_BRGIF_FLD_OPER_MAC_LEARNING_EN:
        p_db_brgif->oper_mac_learning_en = p_brgif->oper_mac_learning_en;
        break;

    case DS_BRGIF_FLD_PORT_SECURITY_EN:
        p_db_brgif->port_security_en = p_brgif->port_security_en;
        break;

    case DS_BRGIF_FLD_VIOLATE_MODE:
        p_db_brgif->violate_mode = p_brgif->violate_mode;
        break;

    case DS_BRGIF_FLD_MLAG_IS_PEER_LINK:
        p_db_brgif->mlag_is_peer_link = p_brgif->mlag_is_peer_link;
        break;

    case DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK:
        p_db_brgif->mlag_is_port_block = p_brgif->mlag_is_port_block;
        break;

    case DS_BRGIF_FLD_MLAG_IS_GROUP:
        p_db_brgif->mlag_is_group = p_brgif->mlag_is_group;
        break;

    case DS_BRGIF_FLD_MAX_MAC:
        p_db_brgif->max_mac = p_brgif->max_mac;
        break;

    case DS_BRGIF_FLD_UNTAGGED_VLAN:
        sal_memcpy(&p_db_brgif->untagged_vlan, &p_brgif->untagged_vlan, sizeof(p_brgif->untagged_vlan));
        break;

    case DS_BRGIF_FLD_MAX:
        sal_memcpy(p_db_brgif, p_brgif, sizeof(ds_brgif_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_BRGIF, field_id, p_db_if, p_db_brgif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_BRGIF, field_id, p_db_if, p_db_brgif);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_brgif_add_brgif(tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{
    return ds_brgif_add_brgif_sync(p_if, p_brgif, TRUE);
}

int32
ds_brgif_del_brgif(tbl_interface_t *p_if)
{
    return ds_brgif_del_brgif_sync(p_if, TRUE);
}

int32
ds_brgif_set_brgif_field(tbl_interface_t *p_if, ds_brgif_t *p_brgif, ds_brgif_field_id_t field_id)
{
    return ds_brgif_set_brgif_field_sync(p_if, p_brgif, field_id, TRUE);
}

ds_brgif_t*
ds_brgif_get_brgif(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->brgif;
}

int32
ds_brgif_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_brgif_key_value_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_brgif_key_name_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_BRGIF);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_brgif_key_value_dump(p_if, p_brgif, ds_str));
    return str;
}

char*
ds_brgif_field_name_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_BRGIF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_BRGIF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_brgif_field_value_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_BRGIF_FLD_PVID, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->pvid);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INNER_NATIVE_VID, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->inner_native_vid);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_vlan_qinq_type_strs, GLB_VLAN_QINQ_TYPE_MAX, p_brgif->qinq_type));
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_DEFAULT_FWD, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->qinq_default_fwd);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANSLATION_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->vlan_translation_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->vlan_trans_default_fwd);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_MAPPING_TABLE, field_id))
    {
        sal_sprintf(str, "%s", p_brgif->vlan_mapping_table);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->dot1q_tag_native_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_ALLOWED_VLAN, field_id))
    {
        cdb_brg_allowed_vlan_val2str(str, MAX_CMD_STR_LEN, &p_brgif->allowed_vlan);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INGRESS_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->ingress_filter);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_EGRESS_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->egress_filter);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_TPID_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->tpid_type);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAC_LEARNING_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mac_learning_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_OPER_MAC_LEARNING_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->oper_mac_learning_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_PORT_SECURITY_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->port_security_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VIOLATE_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->violate_mode);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PEER_LINK, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_is_peer_link);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_is_port_block);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_GROUP, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->mlag_is_group);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAX_MAC, field_id))
    {
        sal_sprintf(str, "%u", p_brgif->max_mac);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_UNTAGGED_VLAN, field_id))
    {
        cdb_brg_allowed_vlan_val2str(str, MAX_CMD_STR_LEN, &p_brgif->untagged_vlan);
    }
    return str;
}

char**
ds_brgif_table_dump(tbl_interface_t *p_if, ds_brgif_t *p_brgif, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_brgif_key_name_dump(p_if, p_brgif, buf));
    for(i=1; i<DS_BRGIF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_brgif_field_name_dump(p_if, p_brgif, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_brgif_field_value_dump(p_if, p_brgif, i, buf));
    }
    return str;
}

int32
ds_brgif_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{
    int32 ret=0;
    if (FLD_MATCH(DS_BRGIF_FLD_PVID, field_id))
    {
        p_brgif->pvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INNER_NATIVE_VID, field_id))
    {
        p_brgif->inner_native_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_TYPE, field_id))
    {
        cdb_enum_str2val(glb_vlan_qinq_type_strs, GLB_VLAN_QINQ_TYPE_MAX, str);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_DEFAULT_FWD, field_id))
    {
        p_brgif->qinq_default_fwd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANSLATION_EN, field_id))
    {
        p_brgif->vlan_translation_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD, field_id))
    {
        p_brgif->vlan_trans_default_fwd = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_MAPPING_TABLE, field_id))
    {
        sal_strcpy(p_brgif->vlan_mapping_table, str);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN, field_id))
    {
        p_brgif->dot1q_tag_native_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_ALLOWED_VLAN, field_id))
    {
        cdb_brg_allowed_vlan_str2val(str, &p_brgif->allowed_vlan);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INGRESS_FILTER, field_id))
    {
        p_brgif->ingress_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_EGRESS_FILTER, field_id))
    {
        p_brgif->egress_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_TPID_TYPE, field_id))
    {
        p_brgif->tpid_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAC_LEARNING_EN, field_id))
    {
        p_brgif->mac_learning_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_OPER_MAC_LEARNING_EN, field_id))
    {
        p_brgif->oper_mac_learning_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_PORT_SECURITY_EN, field_id))
    {
        p_brgif->port_security_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VIOLATE_MODE, field_id))
    {
        p_brgif->violate_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PEER_LINK, field_id))
    {
        p_brgif->mlag_is_peer_link = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK, field_id))
    {
        p_brgif->mlag_is_port_block = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_GROUP, field_id))
    {
        p_brgif->mlag_is_group = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAX_MAC, field_id))
    {
        p_brgif->max_mac = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_UNTAGGED_VLAN, field_id))
    {
        cdb_brg_allowed_vlan_str2val(str, &p_brgif->untagged_vlan);
    }
    return ret;
}

int32
ds_brgif_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_BRGIF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_brgif_key_str2val(tbl_key_value, ds_key_value, p_if, p_brgif));

    for(i=1; i<DS_BRGIF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_brgif_field_value_parser( array[j++], i, p_if, p_brgif));
    }

    return PM_E_NONE;
}

int32
ds_brgif_dump_one(ds_brgif_t *p_brgif, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_BRGIF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_BRGIF_FLD_PVID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_PVID].name,
            p_brgif->pvid);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INNER_NATIVE_VID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_INNER_NATIVE_VID].name,
            p_brgif->inner_native_vid);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_BRGIF_FLD_QINQ_TYPE].name,
            cdb_enum_val2str(glb_vlan_qinq_type_strs, GLB_VLAN_QINQ_TYPE_MAX, p_brgif->qinq_type));
    }
    if (FLD_MATCH(DS_BRGIF_FLD_QINQ_DEFAULT_FWD, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_QINQ_DEFAULT_FWD].name,
            p_brgif->qinq_default_fwd);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANSLATION_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_VLAN_TRANSLATION_EN].name,
            p_brgif->vlan_translation_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD].name,
            p_brgif->vlan_trans_default_fwd);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VLAN_MAPPING_TABLE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_BRGIF_FLD_VLAN_MAPPING_TABLE].name,
            p_brgif->vlan_mapping_table);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN].name,
            p_brgif->dot1q_tag_native_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_ALLOWED_VLAN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_BRGIF_FLD_ALLOWED_VLAN].name,
            cdb_brg_allowed_vlan_val2str(buf, MAX_CMD_STR_LEN, &p_brgif->allowed_vlan));
    }
    if (FLD_MATCH(DS_BRGIF_FLD_INGRESS_FILTER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_INGRESS_FILTER].name,
            p_brgif->ingress_filter);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_EGRESS_FILTER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_EGRESS_FILTER].name,
            p_brgif->egress_filter);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_TPID_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_TPID_TYPE].name,
            p_brgif->tpid_type);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAC_LEARNING_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_MAC_LEARNING_EN].name,
            p_brgif->mac_learning_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_OPER_MAC_LEARNING_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_OPER_MAC_LEARNING_EN].name,
            p_brgif->oper_mac_learning_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_PORT_SECURITY_EN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_PORT_SECURITY_EN].name,
            p_brgif->port_security_en);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_VIOLATE_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_VIOLATE_MODE].name,
            p_brgif->violate_mode);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PEER_LINK, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_MLAG_IS_PEER_LINK].name,
            p_brgif->mlag_is_peer_link);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK].name,
            p_brgif->mlag_is_port_block);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MLAG_IS_GROUP, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_MLAG_IS_GROUP].name,
            p_brgif->mlag_is_group);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_MAX_MAC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_BRGIF_FLD_MAX_MAC].name,
            p_brgif->max_mac);
    }
    if (FLD_MATCH(DS_BRGIF_FLD_UNTAGGED_VLAN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_BRGIF_FLD_UNTAGGED_VLAN].name,
            cdb_brg_allowed_vlan_val2str(buf, MAX_CMD_STR_LEN, &p_brgif->untagged_vlan));
    }

    return PM_E_NONE;
}

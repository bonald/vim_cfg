
#include "sdb.h"
#include "gen/stbl_port_define.h"
#include "gen/stbl_port.h"
#include "sdb_data_cmp.h"

stbl_port_master_t *_g_p_stbl_port_master = NULL;

static uint32
_stbl_port_hash_make(stbl_port_t *p_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_port->key;
    for (index = 0; index < sizeof(p_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_port_t *p_port1 = (stbl_port_t*)p_arg1;
    stbl_port_t *p_port2 = (stbl_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_port1->key, &p_port2->key, sizeof(stbl_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_port_add_port_sync(stbl_port_t *p_port, uint32 sync)
{
    stbl_port_master_t *p_master = _g_p_stbl_port_master;
    stbl_port_t *p_db_port = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_port_get_port(&p_port->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port = XCALLOC(MEM_STBL_PORT, sizeof(stbl_port_t));
    if (NULL == p_db_port)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port, p_port, sizeof(stbl_port_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->port_hash, (void*)p_db_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_port_del_port_sync(stbl_port_key_t *p_port_key, uint32 sync)
{
    stbl_port_master_t *p_master = _g_p_stbl_port_master;
    stbl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = stbl_port_get_port(p_port_key);
    if (NULL == p_db_port)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->port_hash, (void*)p_db_port);

    /* 4. free db entry */
    XFREE(MEM_STBL_PORT, p_db_port);

    return SDB_E_NONE;
}

int32
stbl_port_set_port_field_sync(stbl_port_t *p_port, stbl_port_field_id_t field_id, uint32 sync)
{
    stbl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = stbl_port_get_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_PORT_FLD_FLAG:
        p_db_port->flag = p_port->flag;
        break;

    case STBL_PORT_FLD_GPORT:
        p_db_port->gport = p_port->gport;
        break;

    case STBL_PORT_FLD_LAG_ID:
        p_db_port->lag_id = p_port->lag_id;
        break;

    case STBL_PORT_FLD_TIMESTAMP_COUNT:
        p_db_port->timestamp_count = p_port->timestamp_count;
        break;

    case STBL_PORT_FLD_TIMESTAMP_NEXTHOPID:
        p_db_port->timestamp_nexthopid = p_port->timestamp_nexthopid;
        break;

    case STBL_PORT_FLD_GROUP_ID_IGS:
        p_db_port->group_id_igs = p_port->group_id_igs;
        break;

    case STBL_PORT_FLD_GROUP_ID_EGS:
        p_db_port->group_id_egs = p_port->group_id_egs;
        break;

    case STBL_PORT_FLD_TTP_GROUP_ID_IGS:
        p_db_port->ttp_group_id_igs = p_port->ttp_group_id_igs;
        break;

    case STBL_PORT_FLD_PORT_CLASS_ID:
        p_db_port->port_class_id = p_port->port_class_id;
        break;

    case STBL_PORT_FLD_VLAN_BMP:
        sal_memcpy(p_db_port->vlan_bmp, p_port->vlan_bmp, sizeof(p_port->vlan_bmp));
        break;

    case STBL_PORT_FLD_VLANCLASS_GROUP_ID:
        p_db_port->vlanclass_group_id = p_port->vlanclass_group_id;
        break;

    case STBL_PORT_FLD_VLANCLASS_DEFAULT_ACTION:
        p_db_port->vlanclass_default_action = p_port->vlanclass_default_action;
        break;

    case STBL_PORT_FLD_VLANCLASS_ACTIVE_TYPE:
        p_db_port->vlanclass_active_type = p_port->vlanclass_active_type;
        break;

    case STBL_PORT_FLD_ISOLATION_OID:
        p_db_port->isolation_oid = p_port->isolation_oid;
        break;

    case STBL_PORT_FLD_ISOLATION_DIR:
        p_db_port->isolation_dir = p_port->isolation_dir;
        break;

    case STBL_PORT_FLD_ISOLATION_TYPE:
        p_db_port->isolation_type = p_port->isolation_type;
        break;

    case STBL_PORT_FLD_TAG_CTRL:
        p_db_port->tag_ctrl = p_port->tag_ctrl;
        break;

    case STBL_PORT_FLD_DOT1Q_TYPE:
        p_db_port->dot1q_type = p_port->dot1q_type;
        break;

    case STBL_PORT_FLD_TRANSMIT_EN:
        p_db_port->transmit_en = p_port->transmit_en;
        break;

    case STBL_PORT_FLD_RECEIVE_EN:
        p_db_port->receive_en = p_port->receive_en;
        break;

    case STBL_PORT_FLD_KEEP_VLAN_TAG:
        p_db_port->keep_vlan_tag = p_port->keep_vlan_tag;
        break;

    case STBL_PORT_FLD_INGRESS_VLAN_FILTER:
        p_db_port->ingress_vlan_filter = p_port->ingress_vlan_filter;
        break;

    case STBL_PORT_FLD_EGRESS_VLAN_FILTER:
        p_db_port->egress_vlan_filter = p_port->egress_vlan_filter;
        break;

    case STBL_PORT_FLD_SRC_DISCARD:
        p_db_port->src_discard = p_port->src_discard;
        break;

    case STBL_PORT_FLD_SRC_OUTER_IS_SVLAN:
        p_db_port->src_outer_is_svlan = p_port->src_outer_is_svlan;
        break;

    case STBL_PORT_FLD_LAG_BIND_EN:
        p_db_port->lag_bind_en = p_port->lag_bind_en;
        break;

    case STBL_PORT_FLD_FLOWCTL_DIR:
        p_db_port->flowctl_dir = p_port->flowctl_dir;
        break;

    case STBL_PORT_FLD_FLOWCTL_EN:
        p_db_port->flowctl_en = p_port->flowctl_en;
        break;

    case STBL_PORT_FLD_TYPE_IS_ACCESS:
        p_db_port->type_is_access = p_port->type_is_access;
        break;

    case STBL_PORT_FLD_METADATA_EN:
        p_db_port->metadata_en = p_port->metadata_en;
        break;

    case STBL_PORT_FLD_EGRESS_ACL_EN:
        p_db_port->egress_acl_en = p_port->egress_acl_en;
        break;

    case STBL_PORT_FLD_REFLECTIVE_BRIDGE_EN:
        p_db_port->reflective_bridge_en = p_port->reflective_bridge_en;
        break;

    case STBL_PORT_FLD_DHCPRELAY_EXCEPTION:
        p_db_port->dhcprelay_exception = p_port->dhcprelay_exception;
        break;

    case STBL_PORT_FLD_MAC_LIMIT_NUM:
        p_db_port->mac_limit_num = p_port->mac_limit_num;
        break;

    case STBL_PORT_FLD_MAC_LIMIT_ACTION:
        p_db_port->mac_limit_action = p_port->mac_limit_action;
        break;

    case STBL_PORT_FLD_PORT_STATE:
        p_db_port->port_state = p_port->port_state;
        break;

    case STBL_PORT_FLD_RX_STATS_PTR:
        sal_memcpy(&p_db_port->rx_stats_ptr, &p_port->rx_stats_ptr, sizeof(p_port->rx_stats_ptr));
        break;

    case STBL_PORT_FLD_TX_STATS_PTR:
        sal_memcpy(&p_db_port->tx_stats_ptr, &p_port->tx_stats_ptr, sizeof(p_port->tx_stats_ptr));
        break;

    case STBL_PORT_FLD_MAX:
        sal_memcpy(p_db_port, p_port, sizeof(stbl_port_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_port_add_port(stbl_port_t *p_port)
{
    return stbl_port_add_port_sync(p_port, TRUE);
}

int32
stbl_port_del_port(stbl_port_key_t *p_port_key)
{
    return stbl_port_del_port_sync(p_port_key, TRUE);
}

int32
stbl_port_set_port_field(stbl_port_t *p_port, stbl_port_field_id_t field_id)
{
    return stbl_port_set_port_field_sync(p_port, field_id, TRUE);
}

stbl_port_t*
stbl_port_get_port(stbl_port_key_t *p_port_key)
{
    stbl_port_master_t *p_master = _g_p_stbl_port_master;
    stbl_port_t lkp;

    sal_memcpy(&lkp.key, p_port_key, sizeof(stbl_port_key_t));
    return ctclib_hash_lookup(p_master->port_hash, &lkp);
}

char*
stbl_port_key_val2str(stbl_port_t *p_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->key.port_oid));
    return str;
}

int32
stbl_port_dump_one(stbl_port_t *p_port, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_KEY].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->key.port_oid));
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_FLAG].name,
            p_port->flag);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_GPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_GPORT].name,
            p_port->gport);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_LAG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_LAG_ID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->lag_id));
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TIMESTAMP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[STBL_PORT_FLD_TIMESTAMP_COUNT].name,
            p_port->timestamp_count);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TIMESTAMP_NEXTHOPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_TIMESTAMP_NEXTHOPID].name,
            p_port->timestamp_nexthopid);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_GROUP_ID_IGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_GROUP_ID_IGS].name,
            p_port->group_id_igs);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_GROUP_ID_EGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_GROUP_ID_EGS].name,
            p_port->group_id_egs);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TTP_GROUP_ID_IGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_TTP_GROUP_ID_IGS].name,
            p_port->ttp_group_id_igs);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_PORT_CLASS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_PORT_CLASS_ID].name,
            p_port->port_class_id);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_VLAN_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_VLAN_BMP].name, 
            sdb_bitmap_val2str(buf, SDB_MAX_CMD_STR_LEN, p_port->vlan_bmp, sizeof(p_port->vlan_bmp)/4));
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_VLANCLASS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_VLANCLASS_GROUP_ID].name,
            p_port->vlanclass_group_id);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_VLANCLASS_DEFAULT_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_VLANCLASS_DEFAULT_ACTION].name,
            p_port->vlanclass_default_action);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_VLANCLASS_ACTIVE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_VLANCLASS_ACTIVE_TYPE].name,
            p_port->vlanclass_active_type);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_ISOLATION_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_ISOLATION_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->isolation_oid));
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_ISOLATION_DIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_ISOLATION_DIR].name,
            p_port->isolation_dir);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_ISOLATION_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_ISOLATION_TYPE].name,
            p_port->isolation_type);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TAG_CTRL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_TAG_CTRL].name,
            p_port->tag_ctrl);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_DOT1Q_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_DOT1Q_TYPE].name,
            p_port->dot1q_type);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TRANSMIT_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_TRANSMIT_EN].name,
            p_port->transmit_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_RECEIVE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_RECEIVE_EN].name,
            p_port->receive_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_KEEP_VLAN_TAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_KEEP_VLAN_TAG].name,
            p_port->keep_vlan_tag);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_INGRESS_VLAN_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_INGRESS_VLAN_FILTER].name,
            p_port->ingress_vlan_filter);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_EGRESS_VLAN_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_EGRESS_VLAN_FILTER].name,
            p_port->egress_vlan_filter);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_SRC_DISCARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_SRC_DISCARD].name,
            p_port->src_discard);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_SRC_OUTER_IS_SVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_SRC_OUTER_IS_SVLAN].name,
            p_port->src_outer_is_svlan);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_LAG_BIND_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_LAG_BIND_EN].name,
            p_port->lag_bind_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_FLOWCTL_DIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_FLOWCTL_DIR].name,
            p_port->flowctl_dir);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_FLOWCTL_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_FLOWCTL_EN].name,
            p_port->flowctl_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TYPE_IS_ACCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_TYPE_IS_ACCESS].name,
            p_port->type_is_access);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_METADATA_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_METADATA_EN].name,
            p_port->metadata_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_EGRESS_ACL_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_EGRESS_ACL_EN].name,
            p_port->egress_acl_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_REFLECTIVE_BRIDGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_REFLECTIVE_BRIDGE_EN].name,
            p_port->reflective_bridge_en);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_DHCPRELAY_EXCEPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_DHCPRELAY_EXCEPTION].name,
            p_port->dhcprelay_exception);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_MAC_LIMIT_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_MAC_LIMIT_NUM].name,
            p_port->mac_limit_num);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_MAC_LIMIT_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_MAC_LIMIT_ACTION].name,
            p_port->mac_limit_action);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_PORT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_PORT_FLD_PORT_STATE].name,
            p_port->port_state);
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_RX_STATS_PTR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_RX_STATS_PTR].name, 
            sdb_mac_rx_stats_ptr_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->rx_stats_ptr));
    }
    if (SDB_FLD_MATCH(STBL_PORT_FLD_TX_STATS_PTR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_PORT_FLD_TX_STATS_PTR].name, 
            sdb_mac_tx_stats_ptr_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->tx_stats_ptr));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_port_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_port_master_t *p_master = _g_p_stbl_port_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->port_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_port_master_t*
stbl_port_get_master()
{
    return _g_p_stbl_port_master;
}

stbl_port_master_t*
stbl_port_init_port()
{
    _g_p_stbl_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_port_master_t));
    _g_p_stbl_port_master->port_hash = ctclib_hash_create(_stbl_port_hash_make, _stbl_port_hash_cmp);
    return _g_p_stbl_port_master;
}


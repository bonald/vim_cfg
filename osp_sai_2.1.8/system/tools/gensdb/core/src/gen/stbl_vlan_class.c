
#include "sdb.h"
#include "gen/stbl_vlan_class_define.h"
#include "gen/stbl_vlan_class.h"
#include "sdb_data_cmp.h"

stbl_vlan_class_master_t *_g_p_stbl_vlan_class_master = NULL;

int32
stbl_vlan_class_add_vlan_class_sync(stbl_vlan_class_t *p_vclass, uint32 sync)
{
    stbl_vlan_class_master_t *p_master = _g_p_stbl_vlan_class_master;
    stbl_vlan_class_t *p_db_vclass = NULL;

    /* 1. lookup entry exist */
    if (stbl_vlan_class_get_vlan_class(&p_vclass->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vclass = XCALLOC(MEM_STBL_VLAN_CLASS, sizeof(stbl_vlan_class_t));
    if (NULL == p_db_vclass)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vclass, p_vclass, sizeof(stbl_vlan_class_t));

    /* 4. add to db */
    p_master->array[p_vclass->key.rule_id] = p_db_vclass;

    return SDB_E_NONE;
}

int32
stbl_vlan_class_del_vlan_class_sync(stbl_vlan_class_key_t *p_vclass_key, uint32 sync)
{
    stbl_vlan_class_master_t *p_master = _g_p_stbl_vlan_class_master;
    stbl_vlan_class_t *p_db_vclass = NULL;

    /* 1. lookup entry exist */
    p_db_vclass = stbl_vlan_class_get_vlan_class(p_vclass_key);
    if (NULL == p_db_vclass)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_vclass_key->rule_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_VLAN_CLASS, p_db_vclass);

    return SDB_E_NONE;
}

int32
stbl_vlan_class_set_vlan_class_field_sync(stbl_vlan_class_t *p_vclass, stbl_vlan_class_field_id_t field_id, uint32 sync)
{
    stbl_vlan_class_t *p_db_vclass = NULL;

    /* 1. lookup entry exist */
    p_db_vclass = stbl_vlan_class_get_vlan_class(&p_vclass->key);
    if (NULL == p_db_vclass)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_VLAN_CLASS_FLD_GROUP_OID:
        p_db_vclass->group_oid = p_vclass->group_oid;
        break;

    case STBL_VLAN_CLASS_FLD_MEMBER_OID:
        p_db_vclass->member_oid = p_vclass->member_oid;
        break;

    case STBL_VLAN_CLASS_FLD_ENTRY_TYPE:
        p_db_vclass->entry_type = p_vclass->entry_type;
        break;

    case STBL_VLAN_CLASS_FLD_PROTO_TYPE:
        p_db_vclass->proto_type = p_vclass->proto_type;
        break;

    case STBL_VLAN_CLASS_FLD_VLAN_ID:
        p_db_vclass->vlan_id = p_vclass->vlan_id;
        break;

    case STBL_VLAN_CLASS_FLD_IPADDR:
        sal_memcpy(&p_db_vclass->ipaddr, &p_vclass->ipaddr, sizeof(p_vclass->ipaddr));
        break;

    case STBL_VLAN_CLASS_FLD_MAC:
        sal_memcpy(p_db_vclass->mac, p_vclass->mac, sizeof(p_vclass->mac));
        break;

    case STBL_VLAN_CLASS_FLD_MAX:
        sal_memcpy(p_db_vclass, p_vclass, sizeof(stbl_vlan_class_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_vlan_class_add_vlan_class(stbl_vlan_class_t *p_vclass)
{
    return stbl_vlan_class_add_vlan_class_sync(p_vclass, TRUE);
}

int32
stbl_vlan_class_del_vlan_class(stbl_vlan_class_key_t *p_vclass_key)
{
    return stbl_vlan_class_del_vlan_class_sync(p_vclass_key, TRUE);
}

int32
stbl_vlan_class_set_vlan_class_field(stbl_vlan_class_t *p_vclass, stbl_vlan_class_field_id_t field_id)
{
    return stbl_vlan_class_set_vlan_class_field_sync(p_vclass, field_id, TRUE);
}

stbl_vlan_class_t*
stbl_vlan_class_get_vlan_class(stbl_vlan_class_key_t *p_vclass_key)
{
    stbl_vlan_class_master_t *p_master = _g_p_stbl_vlan_class_master;

    if (GLB_VLANCLASS_RULE_ID_MAX <= p_vclass_key->rule_id)
    {
        return NULL;
    }
    return (p_master->array[p_vclass_key->rule_id]);
}

char*
stbl_vlan_class_key_val2str(stbl_vlan_class_t *p_vclass, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vclass->key.rule_id);
    return str;
}

int32
stbl_vlan_class_dump_one(stbl_vlan_class_t *p_vclass, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_VLAN_CLASS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_CLASS_FLD_KEY].name,
            p_vclass->key.rule_id);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VLAN_CLASS_FLD_GROUP_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vclass->group_oid));
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_MEMBER_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VLAN_CLASS_FLD_MEMBER_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vclass->member_oid));
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_ENTRY_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_CLASS_FLD_ENTRY_TYPE].name,
            p_vclass->entry_type);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_PROTO_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_CLASS_FLD_PROTO_TYPE].name,
            p_vclass->proto_type);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_CLASS_FLD_VLAN_ID].name,
            p_vclass->vlan_id);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_IPADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VLAN_CLASS_FLD_IPADDR].name, 
            sdb_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vclass->ipaddr));
    }
    if (SDB_FLD_MATCH(STBL_VLAN_CLASS_FLD_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VLAN_CLASS_FLD_MAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_vclass->mac));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_vlan_class_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_vlan_class_master_t *p_master = _g_p_stbl_vlan_class_master;
    stbl_vlan_class_t *p_db_vclass = NULL;
    uint32 rule_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (rule_id = 0; rule_id < GLB_VLANCLASS_RULE_ID_MAX; rule_id++)
    {
        p_db_vclass = p_master->array[rule_id];
        if (NULL == p_db_vclass)
        {
            continue;
        }
        rc |= fn(p_db_vclass, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_vlan_class_master_t*
stbl_vlan_class_get_master()
{
    return _g_p_stbl_vlan_class_master;
}

stbl_vlan_class_master_t*
stbl_vlan_class_init_vlan_class()
{
    _g_p_stbl_vlan_class_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_vlan_class_master_t));
    return _g_p_stbl_vlan_class_master;
}


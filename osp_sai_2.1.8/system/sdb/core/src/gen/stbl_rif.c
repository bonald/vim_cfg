
#include "sdb.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_rif.h"
#include "sdb_data_cmp.h"

stbl_rif_master_t *_g_p_stbl_rif_master = NULL;

int32
stbl_rif_add_rif_sync(stbl_rif_t *p_rif, uint32 sync)
{
    stbl_rif_master_t *p_master = _g_p_stbl_rif_master;
    stbl_rif_t *p_db_rif = NULL;

    /* 1. lookup entry exist */
    if (stbl_rif_get_rif(&p_rif->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rif = XCALLOC(MEM_STBL_RIF, sizeof(stbl_rif_t));
    if (NULL == p_db_rif)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rif, p_rif, sizeof(stbl_rif_t));

    /* 4. add to db */
    p_master->array[p_rif->key.rif_id] = p_db_rif;

    return SDB_E_NONE;
}

int32
stbl_rif_del_rif_sync(stbl_rif_key_t *p_rif_key, uint32 sync)
{
    stbl_rif_master_t *p_master = _g_p_stbl_rif_master;
    stbl_rif_t *p_db_rif = NULL;

    /* 1. lookup entry exist */
    p_db_rif = stbl_rif_get_rif(p_rif_key);
    if (NULL == p_db_rif)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_rif_key->rif_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_RIF, p_db_rif);

    return SDB_E_NONE;
}

int32
stbl_rif_set_rif_field_sync(stbl_rif_t *p_rif, stbl_rif_field_id_t field_id, uint32 sync)
{
    stbl_rif_t *p_db_rif = NULL;

    /* 1. lookup entry exist */
    p_db_rif = stbl_rif_get_rif(&p_rif->key);
    if (NULL == p_db_rif)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_RIF_FLD_TYPE:
        p_db_rif->type = p_rif->type;
        break;

    case STBL_RIF_FLD_PORT_OID:
        p_db_rif->port_oid = p_rif->port_oid;
        break;

    case STBL_RIF_FLD_VR_OID:
        p_db_rif->vr_oid = p_rif->vr_oid;
        break;

    case STBL_RIF_FLD_VLAN_ID:
        p_db_rif->vlan_id = p_rif->vlan_id;
        break;

    case STBL_RIF_FLD_SMAC:
        sal_memcpy(p_db_rif->smac, p_rif->smac, sizeof(p_rif->smac));
        break;

    case STBL_RIF_FLD_NAT_ENABLE:
        p_db_rif->nat_enable = p_rif->nat_enable;
        break;

    case STBL_RIF_FLD_NAT_CATEGORY_ID:
        p_db_rif->nat_category_id = p_rif->nat_category_id;
        break;

    case STBL_RIF_FLD_MAX:
        sal_memcpy(p_db_rif, p_rif, sizeof(stbl_rif_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_rif_add_rif(stbl_rif_t *p_rif)
{
    return stbl_rif_add_rif_sync(p_rif, TRUE);
}

int32
stbl_rif_del_rif(stbl_rif_key_t *p_rif_key)
{
    return stbl_rif_del_rif_sync(p_rif_key, TRUE);
}

int32
stbl_rif_set_rif_field(stbl_rif_t *p_rif, stbl_rif_field_id_t field_id)
{
    return stbl_rif_set_rif_field_sync(p_rif, field_id, TRUE);
}

stbl_rif_t*
stbl_rif_get_rif(stbl_rif_key_t *p_rif_key)
{
    stbl_rif_master_t *p_master = _g_p_stbl_rif_master;

    if (GLB_MAX_L3IF_NUM <= p_rif_key->rif_id)
    {
        return NULL;
    }
    return (p_master->array[p_rif_key->rif_id]);
}

char*
stbl_rif_key_val2str(stbl_rif_t *p_rif, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rif->key.rif_id);
    return str;
}

int32
stbl_rif_dump_one(stbl_rif_t *p_rif, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_RIF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_RIF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_RIF_FLD_KEY].name,
            p_rif->key.rif_id);
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_RIF_FLD_TYPE].name,
            p_rif->type);
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_PORT_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_RIF_FLD_PORT_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_rif->port_oid));
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_VR_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_RIF_FLD_VR_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_rif->vr_oid));
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_RIF_FLD_VLAN_ID].name,
            p_rif->vlan_id);
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_SMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_RIF_FLD_SMAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_rif->smac));
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_NAT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_RIF_FLD_NAT_ENABLE].name,
            p_rif->nat_enable);
    }
    if (SDB_FLD_MATCH(STBL_RIF_FLD_NAT_CATEGORY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_RIF_FLD_NAT_CATEGORY_ID].name,
            p_rif->nat_category_id);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_rif_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_rif_master_t *p_master = _g_p_stbl_rif_master;
    stbl_rif_t *p_db_rif = NULL;
    uint32 rif_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (rif_id = 0; rif_id < GLB_MAX_L3IF_NUM; rif_id++)
    {
        p_db_rif = p_master->array[rif_id];
        if (NULL == p_db_rif)
        {
            continue;
        }
        rc |= fn(p_db_rif, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_rif_master_t*
stbl_rif_get_master()
{
    return _g_p_stbl_rif_master;
}

stbl_rif_master_t*
stbl_rif_init_rif()
{
    _g_p_stbl_rif_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_rif_master_t));
    return _g_p_stbl_rif_master;
}


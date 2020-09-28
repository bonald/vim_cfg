
#include "sdb.h"
#include "gen/stbl_vlan_define.h"
#include "gen/stbl_vlan.h"
#include "sdb_data_cmp.h"

stbl_vlan_master_t *_g_p_stbl_vlan_master = NULL;

int32
stbl_vlan_add_vlan_sync(stbl_vlan_t *p_vlan, uint32 sync)
{
    stbl_vlan_master_t *p_master = _g_p_stbl_vlan_master;
    stbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    if (stbl_vlan_get_vlan(&p_vlan->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan = XCALLOC(MEM_STBL_VLAN, sizeof(stbl_vlan_t));
    if (NULL == p_db_vlan)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan, p_vlan, sizeof(stbl_vlan_t));

    /* 4. add to db */
    p_master->array[p_vlan->key.vid] = p_db_vlan;

    return SDB_E_NONE;
}

int32
stbl_vlan_del_vlan_sync(stbl_vlan_key_t *p_vlan_key, uint32 sync)
{
    stbl_vlan_master_t *p_master = _g_p_stbl_vlan_master;
    stbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = stbl_vlan_get_vlan(p_vlan_key);
    if (NULL == p_db_vlan)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_vlan_key->vid] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_VLAN, p_db_vlan);

    return SDB_E_NONE;
}

int32
stbl_vlan_set_vlan_field_sync(stbl_vlan_t *p_vlan, stbl_vlan_field_id_t field_id, uint32 sync)
{
    stbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = stbl_vlan_get_vlan(&p_vlan->key);
    if (NULL == p_db_vlan)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_VLAN_FLD_LEARN_DISABLE:
        p_db_vlan->learn_disable = p_vlan->learn_disable;
        break;

    case STBL_VLAN_FLD_STP_OID:
        p_db_vlan->stp_oid = p_vlan->stp_oid;
        break;

    case STBL_VLAN_FLD_STATS_ID_L2_IN:
        p_db_vlan->stats_id_l2_in = p_vlan->stats_id_l2_in;
        break;

    case STBL_VLAN_FLD_STATS_ID_L2_OUT:
        p_db_vlan->stats_id_l2_out = p_vlan->stats_id_l2_out;
        break;

    case STBL_VLAN_FLD_STATS_ID_L3_IN:
        p_db_vlan->stats_id_l3_in = p_vlan->stats_id_l3_in;
        break;

    case STBL_VLAN_FLD_STATS_ID_L3_OUT:
        p_db_vlan->stats_id_l3_out = p_vlan->stats_id_l3_out;
        break;

    case STBL_VLAN_FLD_MAX:
        sal_memcpy(p_db_vlan, p_vlan, sizeof(stbl_vlan_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_vlan_add_vlan(stbl_vlan_t *p_vlan)
{
    return stbl_vlan_add_vlan_sync(p_vlan, TRUE);
}

int32
stbl_vlan_del_vlan(stbl_vlan_key_t *p_vlan_key)
{
    return stbl_vlan_del_vlan_sync(p_vlan_key, TRUE);
}

int32
stbl_vlan_set_vlan_field(stbl_vlan_t *p_vlan, stbl_vlan_field_id_t field_id)
{
    return stbl_vlan_set_vlan_field_sync(p_vlan, field_id, TRUE);
}

stbl_vlan_t*
stbl_vlan_get_vlan(stbl_vlan_key_t *p_vlan_key)
{
    stbl_vlan_master_t *p_master = _g_p_stbl_vlan_master;

    if (GLB_MAX_VLAN_ID <= p_vlan_key->vid)
    {
        return NULL;
    }
    return (p_master->array[p_vlan_key->vid]);
}

char*
stbl_vlan_key_val2str(stbl_vlan_t *p_vlan, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlan->key.vid);
    return str;
}

int32
stbl_vlan_dump_one(stbl_vlan_t *p_vlan, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_VLAN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_VLAN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_KEY].name,
            p_vlan->key.vid);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_LEARN_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_LEARN_DISABLE].name,
            p_vlan->learn_disable);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_STP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VLAN_FLD_STP_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vlan->stp_oid));
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_STATS_ID_L2_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_STATS_ID_L2_IN].name,
            p_vlan->stats_id_l2_in);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_STATS_ID_L2_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_STATS_ID_L2_OUT].name,
            p_vlan->stats_id_l2_out);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_STATS_ID_L3_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_STATS_ID_L3_IN].name,
            p_vlan->stats_id_l3_in);
    }
    if (SDB_FLD_MATCH(STBL_VLAN_FLD_STATS_ID_L3_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VLAN_FLD_STATS_ID_L3_OUT].name,
            p_vlan->stats_id_l3_out);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_vlan_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_vlan_master_t *p_master = _g_p_stbl_vlan_master;
    stbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        rc |= fn(p_db_vlan, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_vlan_master_t*
stbl_vlan_get_master()
{
    return _g_p_stbl_vlan_master;
}

stbl_vlan_master_t*
stbl_vlan_init_vlan()
{
    _g_p_stbl_vlan_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_vlan_master_t));
    return _g_p_stbl_vlan_master;
}


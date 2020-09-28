
#include "sdb.h"
#include "gen/stbl_vr_define.h"
#include "gen/stbl_vr.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_rif.h"
#include "sdb_data_cmp.h"

stbl_vr_master_t *_g_p_stbl_vr_master = NULL;

int32
stbl_vr_add_vr_sync(stbl_vr_t *p_vr, uint32 sync)
{
    stbl_vr_master_t *p_master = _g_p_stbl_vr_master;
    stbl_vr_t *p_db_vr = NULL;

    /* 1. lookup entry exist */
    if (stbl_vr_get_vr(&p_vr->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vr = XCALLOC(MEM_STBL_VR, sizeof(stbl_vr_t));
    if (NULL == p_db_vr)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vr, p_vr, sizeof(stbl_vr_t));
    sdb_reference_list_init(&p_db_vr->rif_list, SDB_NODE_TBL, STBL_RIF, stbl_rif_cmp);

    /* 4. add to db */
    p_master->array[p_vr->key.vrf_id] = p_db_vr;

    return SDB_E_NONE;
}

int32
stbl_vr_del_vr_sync(stbl_vr_key_t *p_vr_key, uint32 sync)
{
    stbl_vr_master_t *p_master = _g_p_stbl_vr_master;
    stbl_vr_t *p_db_vr = NULL;

    /* 1. lookup entry exist */
    p_db_vr = stbl_vr_get_vr(p_vr_key);
    if (NULL == p_db_vr)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_vr_key->vrf_id] = NULL;
    sdb_reference_list_deinit(&p_db_vr->rif_list);

    /* 4. free db entry */
    XFREE(MEM_STBL_VR, p_db_vr);

    return SDB_E_NONE;
}

int32
stbl_vr_set_vr_field_sync(stbl_vr_t *p_vr, stbl_vr_field_id_t field_id, uint32 sync)
{
    stbl_vr_t *p_db_vr = NULL;

    /* 1. lookup entry exist */
    p_db_vr = stbl_vr_get_vr(&p_vr->key);
    if (NULL == p_db_vr)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_VR_FLD_VR_OID:
        p_db_vr->vr_oid = p_vr->vr_oid;
        break;

    case STBL_VR_FLD_SMAC:
        sal_memcpy(p_db_vr->smac, p_vr->smac, sizeof(p_vr->smac));
        break;

    case STBL_VR_FLD_V4_ADMIN:
        p_db_vr->v4_admin = p_vr->v4_admin;
        break;

    case STBL_VR_FLD_V6_ADMIN:
        p_db_vr->v6_admin = p_vr->v6_admin;
        break;

    case STBL_VR_FLD_MAX:
        sal_memcpy(p_db_vr, p_vr, sizeof(stbl_vr_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_vr_add_vr(stbl_vr_t *p_vr)
{
    return stbl_vr_add_vr_sync(p_vr, TRUE);
}

int32
stbl_vr_del_vr(stbl_vr_key_t *p_vr_key)
{
    return stbl_vr_del_vr_sync(p_vr_key, TRUE);
}

int32
stbl_vr_set_vr_field(stbl_vr_t *p_vr, stbl_vr_field_id_t field_id)
{
    return stbl_vr_set_vr_field_sync(p_vr, field_id, TRUE);
}

stbl_vr_t*
stbl_vr_get_vr(stbl_vr_key_t *p_vr_key)
{
    stbl_vr_master_t *p_master = _g_p_stbl_vr_master;

    if (GLB_MAX_VRFID <= p_vr_key->vrf_id)
    {
        return NULL;
    }
    return (p_master->array[p_vr_key->vrf_id]);
}

char*
stbl_vr_key_val2str(stbl_vr_t *p_vr, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vr->key.vrf_id);
    return str;
}

int32
stbl_vr_dump_one(stbl_vr_t *p_vr, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_VR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_VR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VR_FLD_KEY].name,
            p_vr->key.vrf_id);
    }
    if (SDB_FLD_MATCH(STBL_VR_FLD_VR_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VR_FLD_VR_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vr->vr_oid));
    }
    if (SDB_FLD_MATCH(STBL_VR_FLD_SMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VR_FLD_SMAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_vr->smac));
    }
    if (SDB_FLD_MATCH(STBL_VR_FLD_V4_ADMIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VR_FLD_V4_ADMIN].name,
            p_vr->v4_admin);
    }
    if (SDB_FLD_MATCH(STBL_VR_FLD_V6_ADMIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_VR_FLD_V6_ADMIN].name,
            p_vr->v6_admin);
    }
    if (SDB_FLD_MATCH(STBL_VR_FLD_RIF_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[STBL_VR_FLD_RIF_LIST].name,
            sdb_reference_list_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vr->rif_list));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_vr_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_vr_master_t *p_master = _g_p_stbl_vr_master;
    stbl_vr_t *p_db_vr = NULL;
    uint32 vrf_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (vrf_id = 0; vrf_id < GLB_MAX_VRFID; vrf_id++)
    {
        p_db_vr = p_master->array[vrf_id];
        if (NULL == p_db_vr)
        {
            continue;
        }
        rc |= fn(p_db_vr, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_vr_master_t*
stbl_vr_get_master()
{
    return _g_p_stbl_vr_master;
}

stbl_vr_master_t*
stbl_vr_init_vr()
{
    _g_p_stbl_vr_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_vr_master_t));
    return _g_p_stbl_vr_master;
}

int32
stbl_vr_set_rif_list_sync(stbl_vr_t *p_vr, uint32 add, void *p_object, uint32 sync)
{
    stbl_vr_t *p_db_vr = NULL;

    /* 1. lookup entry exist */
    p_db_vr = stbl_vr_get_vr(&p_vr->key);
    if (NULL == p_db_vr)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        sdb_reference_list_add_object(&p_db_vr->rif_list, p_object);
    }
    else
    {
        sdb_reference_list_del_object(&p_db_vr->rif_list, p_object);
    }

    return SDB_E_NONE;
}

int32
stbl_vr_set_rif_list(stbl_vr_t *p_vr, uint32 add, void *p_object)
{
    return stbl_vr_set_rif_list_sync(p_vr, add, p_object, TRUE);
}


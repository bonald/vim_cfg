
#include "sdb.h"
#include "gen/stbl_stp_define.h"
#include "gen/stbl_stp.h"
#include "sdb_data_cmp.h"

stbl_stp_master_t *_g_p_stbl_stp_master = NULL;

int32
stbl_stp_add_stp_sync(stbl_stp_t *p_stp, uint32 sync)
{
    stbl_stp_master_t *p_master = _g_p_stbl_stp_master;
    stbl_stp_t *p_db_stp = NULL;

    /* 1. lookup entry exist */
    if (stbl_stp_get_stp(&p_stp->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_stp = XCALLOC(MEM_STBL_STP, sizeof(stbl_stp_t));
    if (NULL == p_db_stp)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_stp, p_stp, sizeof(stbl_stp_t));

    /* 4. add to db */
    p_master->array[p_stp->key.stp_id] = p_db_stp;

    return SDB_E_NONE;
}

int32
stbl_stp_del_stp_sync(stbl_stp_key_t *p_stp_key, uint32 sync)
{
    stbl_stp_master_t *p_master = _g_p_stbl_stp_master;
    stbl_stp_t *p_db_stp = NULL;

    /* 1. lookup entry exist */
    p_db_stp = stbl_stp_get_stp(p_stp_key);
    if (NULL == p_db_stp)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_stp_key->stp_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_STP, p_db_stp);

    return SDB_E_NONE;
}

int32
stbl_stp_set_stp_field_sync(stbl_stp_t *p_stp, stbl_stp_field_id_t field_id, uint32 sync)
{
    stbl_stp_t *p_db_stp = NULL;

    /* 1. lookup entry exist */
    p_db_stp = stbl_stp_get_stp(&p_stp->key);
    if (NULL == p_db_stp)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_STP_FLD_STP_OID:
        p_db_stp->stp_oid = p_stp->stp_oid;
        break;

    case STBL_STP_FLD_VLAN_BMP:
        sal_memcpy(p_db_stp->vlan_bmp, p_stp->vlan_bmp, sizeof(p_stp->vlan_bmp));
        break;

    case STBL_STP_FLD_MAX:
        sal_memcpy(p_db_stp, p_stp, sizeof(stbl_stp_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_stp_add_stp(stbl_stp_t *p_stp)
{
    return stbl_stp_add_stp_sync(p_stp, TRUE);
}

int32
stbl_stp_del_stp(stbl_stp_key_t *p_stp_key)
{
    return stbl_stp_del_stp_sync(p_stp_key, TRUE);
}

int32
stbl_stp_set_stp_field(stbl_stp_t *p_stp, stbl_stp_field_id_t field_id)
{
    return stbl_stp_set_stp_field_sync(p_stp, field_id, TRUE);
}

stbl_stp_t*
stbl_stp_get_stp(stbl_stp_key_t *p_stp_key)
{
    stbl_stp_master_t *p_master = _g_p_stbl_stp_master;

    if (GLB_STP_INSTANCE_MAX <= p_stp_key->stp_id)
    {
        return NULL;
    }
    return (p_master->array[p_stp_key->stp_id]);
}

char*
stbl_stp_key_val2str(stbl_stp_t *p_stp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_stp->key.stp_id);
    return str;
}

int32
stbl_stp_dump_one(stbl_stp_t *p_stp, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_STP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_STP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_STP_FLD_KEY].name,
            p_stp->key.stp_id);
    }
    if (SDB_FLD_MATCH(STBL_STP_FLD_STP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_STP_FLD_STP_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_stp->stp_oid));
    }
    if (SDB_FLD_MATCH(STBL_STP_FLD_VLAN_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_STP_FLD_VLAN_BMP].name, 
            sdb_bitmap_val2str(buf, SDB_MAX_CMD_STR_LEN, p_stp->vlan_bmp, sizeof(p_stp->vlan_bmp)/4));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_stp_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_stp_master_t *p_master = _g_p_stbl_stp_master;
    stbl_stp_t *p_db_stp = NULL;
    uint32 stp_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (stp_id = 0; stp_id < GLB_STP_INSTANCE_MAX; stp_id++)
    {
        p_db_stp = p_master->array[stp_id];
        if (NULL == p_db_stp)
        {
            continue;
        }
        rc |= fn(p_db_stp, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_stp_master_t*
stbl_stp_get_master()
{
    return _g_p_stbl_stp_master;
}

stbl_stp_master_t*
stbl_stp_init_stp()
{
    _g_p_stbl_stp_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_stp_master_t));
    return _g_p_stbl_stp_master;
}


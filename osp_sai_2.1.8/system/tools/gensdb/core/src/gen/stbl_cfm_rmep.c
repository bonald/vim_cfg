
#include "sdb.h"
#include "gen/stbl_cfm_rmep_define.h"
#include "gen/stbl_cfm_rmep.h"
#include "sdb_data_cmp.h"

stbl_cfm_rmep_master_t *_g_p_stbl_cfm_rmep_master = NULL;

static uint32
_stbl_cfm_rmep_hash_make(stbl_cfm_rmep_t *p_rmep)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmep->key;
    for (index = 0; index < sizeof(p_rmep->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_cfm_rmep_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_cfm_rmep_t *p_rmep1 = (stbl_cfm_rmep_t*)p_arg1;
    stbl_cfm_rmep_t *p_rmep2 = (stbl_cfm_rmep_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmep1->key, &p_rmep2->key, sizeof(stbl_cfm_rmep_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_cfm_rmep_add_cfm_rmep_sync(stbl_cfm_rmep_t *p_rmep, uint32 sync)
{
    stbl_cfm_rmep_master_t *p_master = _g_p_stbl_cfm_rmep_master;
    stbl_cfm_rmep_t *p_db_rmep = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_cfm_rmep_get_cfm_rmep(&p_rmep->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmep = XCALLOC(MEM_STBL_CFM_RMEP, sizeof(stbl_cfm_rmep_t));
    if (NULL == p_db_rmep)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmep, p_rmep, sizeof(stbl_cfm_rmep_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_rmep_hash, (void*)p_db_rmep, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_rmep_list, p_db_rmep);

    return SDB_E_NONE;
}

int32
stbl_cfm_rmep_del_cfm_rmep_sync(stbl_cfm_rmep_key_t *p_rmep_key, uint32 sync)
{
    stbl_cfm_rmep_master_t *p_master = _g_p_stbl_cfm_rmep_master;
    stbl_cfm_rmep_t *p_db_rmep = NULL;

    /* 1. lookup entry exist */
    p_db_rmep = stbl_cfm_rmep_get_cfm_rmep(p_rmep_key);
    if (NULL == p_db_rmep)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_rmep_hash, (void*)p_db_rmep);
    ctclib_slistnode_delete(p_master->cfm_rmep_list, p_db_rmep);

    /* 4. free db entry */
    XFREE(MEM_STBL_CFM_RMEP, p_db_rmep);

    return SDB_E_NONE;
}

int32
stbl_cfm_rmep_set_cfm_rmep_field_sync(stbl_cfm_rmep_t *p_rmep, stbl_cfm_rmep_field_id_t field_id, uint32 sync)
{
    stbl_cfm_rmep_t *p_db_rmep = NULL;

    /* 1. lookup entry exist */
    p_db_rmep = stbl_cfm_rmep_get_cfm_rmep(&p_rmep->key);
    if (NULL == p_db_rmep)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_CFM_RMEP_FLD_GPORT:
        p_db_rmep->gport = p_rmep->gport;
        break;

    case STBL_CFM_RMEP_FLD_VLAN_ID:
        p_db_rmep->vlan_id = p_rmep->vlan_id;
        break;

    case STBL_CFM_RMEP_FLD_MD_LEVEL:
        p_db_rmep->md_level = p_rmep->md_level;
        break;

    case STBL_CFM_RMEP_FLD_RMEP_ID:
        p_db_rmep->rmep_id = p_rmep->rmep_id;
        break;

    case STBL_CFM_RMEP_FLD_RMEP_MAC:
        sal_memcpy(p_db_rmep->rmep_mac, p_rmep->rmep_mac, sizeof(p_rmep->rmep_mac));
        break;

    case STBL_CFM_RMEP_FLD_IFINDEX:
        p_db_rmep->ifindex = p_rmep->ifindex;
        break;

    case STBL_CFM_RMEP_FLD_FIRST_PKT_RX:
        p_db_rmep->first_pkt_rx = p_rmep->first_pkt_rx;
        break;

    case STBL_CFM_RMEP_FLD_D_LOC:
        p_db_rmep->d_loc = p_rmep->d_loc;
        break;

    case STBL_CFM_RMEP_FLD_D_UNEXP_PERIOD:
        p_db_rmep->d_unexp_period = p_rmep->d_unexp_period;
        break;

    case STBL_CFM_RMEP_FLD_MA_SA_MISMATCH:
        p_db_rmep->ma_sa_mismatch = p_rmep->ma_sa_mismatch;
        break;

    case STBL_CFM_RMEP_FLD_RMEP_LAST_RDI:
        p_db_rmep->rmep_last_rdi = p_rmep->rmep_last_rdi;
        break;

    case STBL_CFM_RMEP_FLD_MAX:
        sal_memcpy(p_db_rmep, p_rmep, sizeof(stbl_cfm_rmep_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_cfm_rmep_add_cfm_rmep(stbl_cfm_rmep_t *p_rmep)
{
    return stbl_cfm_rmep_add_cfm_rmep_sync(p_rmep, TRUE);
}

int32
stbl_cfm_rmep_del_cfm_rmep(stbl_cfm_rmep_key_t *p_rmep_key)
{
    return stbl_cfm_rmep_del_cfm_rmep_sync(p_rmep_key, TRUE);
}

int32
stbl_cfm_rmep_set_cfm_rmep_field(stbl_cfm_rmep_t *p_rmep, stbl_cfm_rmep_field_id_t field_id)
{
    return stbl_cfm_rmep_set_cfm_rmep_field_sync(p_rmep, field_id, TRUE);
}

stbl_cfm_rmep_t*
stbl_cfm_rmep_get_cfm_rmep(stbl_cfm_rmep_key_t *p_rmep_key)
{
    stbl_cfm_rmep_master_t *p_master = _g_p_stbl_cfm_rmep_master;
    stbl_cfm_rmep_t lkp;

    sal_memcpy(&lkp.key, p_rmep_key, sizeof(stbl_cfm_rmep_key_t));
    return ctclib_hash_lookup(p_master->cfm_rmep_hash, &lkp);
}

char*
stbl_cfm_rmep_key_val2str(stbl_cfm_rmep_t *p_rmep, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_rmep->key.index);
    return str;
}

int32
stbl_cfm_rmep_dump_one(stbl_cfm_rmep_t *p_rmep, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_CFM_RMEP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_KEY].name,
            p_rmep->key.index);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_GPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_GPORT].name,
            p_rmep->gport);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_VLAN_ID].name,
            p_rmep->vlan_id);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_MD_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_MD_LEVEL].name,
            p_rmep->md_level);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_RMEP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_RMEP_ID].name,
            p_rmep->rmep_id);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_RMEP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_CFM_RMEP_FLD_RMEP_MAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_rmep->rmep_mac));
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_IFINDEX].name,
            p_rmep->ifindex);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_FIRST_PKT_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_FIRST_PKT_RX].name,
            p_rmep->first_pkt_rx);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_D_LOC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_D_LOC].name,
            p_rmep->d_loc);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_D_UNEXP_PERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_D_UNEXP_PERIOD].name,
            p_rmep->d_unexp_period);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_MA_SA_MISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_MA_SA_MISMATCH].name,
            p_rmep->ma_sa_mismatch);
    }
    if (SDB_FLD_MATCH(STBL_CFM_RMEP_FLD_RMEP_LAST_RDI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_RMEP_FLD_RMEP_LAST_RDI].name,
            p_rmep->rmep_last_rdi);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_cfm_rmep_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_cfm_rmep_master_t *p_master = _g_p_stbl_cfm_rmep_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    stbl_cfm_rmep_t *p_db_rmep = NULL;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_rmep_list, p_db_rmep, listnode, next)
    {
        rc |= fn(p_db_rmep, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_cfm_rmep_master_t*
stbl_cfm_rmep_get_master()
{
    return _g_p_stbl_cfm_rmep_master;
}

stbl_cfm_rmep_master_t*
stbl_cfm_rmep_init_cfm_rmep()
{
    _g_p_stbl_cfm_rmep_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_cfm_rmep_master_t));
    _g_p_stbl_cfm_rmep_master->cfm_rmep_hash = ctclib_hash_create(_stbl_cfm_rmep_hash_make, _stbl_cfm_rmep_hash_cmp);
    _g_p_stbl_cfm_rmep_master->cfm_rmep_list = ctclib_slist_create(stbl_cfm_rmep_cmp, NULL);
    return _g_p_stbl_cfm_rmep_master;
}


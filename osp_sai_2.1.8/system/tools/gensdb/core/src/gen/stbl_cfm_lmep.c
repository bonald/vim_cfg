
#include "sdb.h"
#include "gen/stbl_cfm_lmep_define.h"
#include "gen/stbl_cfm_lmep.h"
#include "sdb_data_cmp.h"

stbl_cfm_lmep_master_t *_g_p_stbl_cfm_lmep_master = NULL;

static uint32
_stbl_cfm_lmep_hash_make(stbl_cfm_lmep_t *p_lmep)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_lmep->key;
    for (index = 0; index < sizeof(p_lmep->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_cfm_lmep_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_cfm_lmep_t *p_lmep1 = (stbl_cfm_lmep_t*)p_arg1;
    stbl_cfm_lmep_t *p_lmep2 = (stbl_cfm_lmep_t*)p_arg2;

    if (0 == sal_memcmp(&p_lmep1->key, &p_lmep2->key, sizeof(stbl_cfm_lmep_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_cfm_lmep_add_cfm_lmep_sync(stbl_cfm_lmep_t *p_lmep, uint32 sync)
{
    stbl_cfm_lmep_master_t *p_master = _g_p_stbl_cfm_lmep_master;
    stbl_cfm_lmep_t *p_db_lmep = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_cfm_lmep_get_cfm_lmep(&p_lmep->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lmep = XCALLOC(MEM_STBL_CFM_LMEP, sizeof(stbl_cfm_lmep_t));
    if (NULL == p_db_lmep)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lmep, p_lmep, sizeof(stbl_cfm_lmep_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_lmep_hash, (void*)p_db_lmep, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_lmep_list, p_db_lmep);

    return SDB_E_NONE;
}

int32
stbl_cfm_lmep_del_cfm_lmep_sync(stbl_cfm_lmep_key_t *p_lmep_key, uint32 sync)
{
    stbl_cfm_lmep_master_t *p_master = _g_p_stbl_cfm_lmep_master;
    stbl_cfm_lmep_t *p_db_lmep = NULL;

    /* 1. lookup entry exist */
    p_db_lmep = stbl_cfm_lmep_get_cfm_lmep(p_lmep_key);
    if (NULL == p_db_lmep)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_lmep_hash, (void*)p_db_lmep);
    ctclib_slistnode_delete(p_master->cfm_lmep_list, p_db_lmep);

    /* 4. free db entry */
    XFREE(MEM_STBL_CFM_LMEP, p_db_lmep);

    return SDB_E_NONE;
}

int32
stbl_cfm_lmep_set_cfm_lmep_field_sync(stbl_cfm_lmep_t *p_lmep, stbl_cfm_lmep_field_id_t field_id, uint32 sync)
{
    stbl_cfm_lmep_t *p_db_lmep = NULL;

    /* 1. lookup entry exist */
    p_db_lmep = stbl_cfm_lmep_get_cfm_lmep(&p_lmep->key);
    if (NULL == p_db_lmep)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_CFM_LMEP_FLD_GPORT:
        p_db_lmep->gport = p_lmep->gport;
        break;

    case STBL_CFM_LMEP_FLD_VLAN_ID:
        p_db_lmep->vlan_id = p_lmep->vlan_id;
        break;

    case STBL_CFM_LMEP_FLD_MD_LEVEL:
        p_db_lmep->md_level = p_lmep->md_level;
        break;

    case STBL_CFM_LMEP_FLD_LMEP_NUM:
        p_db_lmep->lmep_num = p_lmep->lmep_num;
        break;

    case STBL_CFM_LMEP_FLD_MEP_ID:
        p_db_lmep->mep_id = p_lmep->mep_id;
        break;

    case STBL_CFM_LMEP_FLD_MAID:
        sal_memcpy(p_db_lmep->maid, p_lmep->maid, sizeof(p_lmep->maid));
        break;

    case STBL_CFM_LMEP_FLD_DIRECTION:
        p_db_lmep->direction = p_lmep->direction;
        break;

    case STBL_CFM_LMEP_FLD_D_UNEXP_MEP:
        p_db_lmep->d_unexp_mep = p_lmep->d_unexp_mep;
        break;

    case STBL_CFM_LMEP_FLD_D_MISMERGE:
        p_db_lmep->d_mismerge = p_lmep->d_mismerge;
        break;

    case STBL_CFM_LMEP_FLD_D_MEG_LVL:
        p_db_lmep->d_meg_lvl = p_lmep->d_meg_lvl;
        break;

    case STBL_CFM_LMEP_FLD_CCI_ENABLED:
        p_db_lmep->cci_enabled = p_lmep->cci_enabled;
        break;

    case STBL_CFM_LMEP_FLD_CCI_INTERVAL:
        p_db_lmep->cci_interval = p_lmep->cci_interval;
        break;

    case STBL_CFM_LMEP_FLD_RDI:
        p_db_lmep->rdi = p_lmep->rdi;
        break;

    case STBL_CFM_LMEP_FLD_CCM_VLAN_COS:
        p_db_lmep->ccm_vlan_cos = p_lmep->ccm_vlan_cos;
        break;

    case STBL_CFM_LMEP_FLD_IFINDEX:
        p_db_lmep->ifindex = p_lmep->ifindex;
        break;

    case STBL_CFM_LMEP_FLD_MODE:
        p_db_lmep->mode = p_lmep->mode;
        break;

    case STBL_CFM_LMEP_FLD_MAX:
        sal_memcpy(p_db_lmep, p_lmep, sizeof(stbl_cfm_lmep_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_cfm_lmep_add_cfm_lmep(stbl_cfm_lmep_t *p_lmep)
{
    return stbl_cfm_lmep_add_cfm_lmep_sync(p_lmep, TRUE);
}

int32
stbl_cfm_lmep_del_cfm_lmep(stbl_cfm_lmep_key_t *p_lmep_key)
{
    return stbl_cfm_lmep_del_cfm_lmep_sync(p_lmep_key, TRUE);
}

int32
stbl_cfm_lmep_set_cfm_lmep_field(stbl_cfm_lmep_t *p_lmep, stbl_cfm_lmep_field_id_t field_id)
{
    return stbl_cfm_lmep_set_cfm_lmep_field_sync(p_lmep, field_id, TRUE);
}

stbl_cfm_lmep_t*
stbl_cfm_lmep_get_cfm_lmep(stbl_cfm_lmep_key_t *p_lmep_key)
{
    stbl_cfm_lmep_master_t *p_master = _g_p_stbl_cfm_lmep_master;
    stbl_cfm_lmep_t lkp;

    sal_memcpy(&lkp.key, p_lmep_key, sizeof(stbl_cfm_lmep_key_t));
    return ctclib_hash_lookup(p_master->cfm_lmep_hash, &lkp);
}

char*
stbl_cfm_lmep_key_val2str(stbl_cfm_lmep_t *p_lmep, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_lmep->key.index);
    return str;
}

int32
stbl_cfm_lmep_dump_one(stbl_cfm_lmep_t *p_lmep, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_CFM_LMEP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_KEY].name,
            p_lmep->key.index);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_GPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_GPORT].name,
            p_lmep->gport);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_VLAN_ID].name,
            p_lmep->vlan_id);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_MD_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_MD_LEVEL].name,
            p_lmep->md_level);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_LMEP_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_LMEP_NUM].name,
            p_lmep->lmep_num);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_MEP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_MEP_ID].name,
            p_lmep->mep_id);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_MAID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_CFM_LMEP_FLD_MAID].name, 
            sdb_uint8_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_lmep->maid, sizeof(p_lmep->maid)));
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_DIRECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_DIRECTION].name,
            p_lmep->direction);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_D_UNEXP_MEP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_D_UNEXP_MEP].name,
            p_lmep->d_unexp_mep);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_D_MISMERGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_D_MISMERGE].name,
            p_lmep->d_mismerge);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_D_MEG_LVL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_D_MEG_LVL].name,
            p_lmep->d_meg_lvl);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_CCI_ENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_CCI_ENABLED].name,
            p_lmep->cci_enabled);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_CCI_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_CCI_INTERVAL].name,
            p_lmep->cci_interval);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_RDI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_RDI].name,
            p_lmep->rdi);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_CCM_VLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_CCM_VLAN_COS].name,
            p_lmep->ccm_vlan_cos);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_IFINDEX].name,
            p_lmep->ifindex);
    }
    if (SDB_FLD_MATCH(STBL_CFM_LMEP_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_CFM_LMEP_FLD_MODE].name,
            p_lmep->mode);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_cfm_lmep_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_cfm_lmep_master_t *p_master = _g_p_stbl_cfm_lmep_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    stbl_cfm_lmep_t *p_db_lmep = NULL;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_lmep_list, p_db_lmep, listnode, next)
    {
        rc |= fn(p_db_lmep, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_cfm_lmep_master_t*
stbl_cfm_lmep_get_master()
{
    return _g_p_stbl_cfm_lmep_master;
}

stbl_cfm_lmep_master_t*
stbl_cfm_lmep_init_cfm_lmep()
{
    _g_p_stbl_cfm_lmep_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_cfm_lmep_master_t));
    _g_p_stbl_cfm_lmep_master->cfm_lmep_hash = ctclib_hash_create(_stbl_cfm_lmep_hash_make, _stbl_cfm_lmep_hash_cmp);
    _g_p_stbl_cfm_lmep_master->cfm_lmep_list = ctclib_slist_create(stbl_cfm_lmep_cmp, NULL);
    return _g_p_stbl_cfm_lmep_master;
}


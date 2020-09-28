
#include "sdb.h"
#include "gen/stbl_neighbor_define.h"
#include "gen/stbl_neighbor.h"
#include "sdb_data_cmp.h"

stbl_neighbor_master_t *_g_p_stbl_neighbor_master = NULL;

static uint32
_stbl_neighbor_hash_make(stbl_neighbor_t *p_neigh)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_neigh->key;
    for (index = 0; index < sizeof(p_neigh->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_neighbor_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_neighbor_t *p_neigh1 = (stbl_neighbor_t*)p_arg1;
    stbl_neighbor_t *p_neigh2 = (stbl_neighbor_t*)p_arg2;

    if (0 == sal_memcmp(&p_neigh1->key, &p_neigh2->key, sizeof(stbl_neighbor_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_neighbor_add_neighbor_sync(stbl_neighbor_t *p_neigh, uint32 sync)
{
    stbl_neighbor_master_t *p_master = _g_p_stbl_neighbor_master;
    stbl_neighbor_t *p_db_neigh = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_neighbor_get_neighbor(&p_neigh->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_neigh = XCALLOC(MEM_STBL_NEIGHBOR, sizeof(stbl_neighbor_t));
    if (NULL == p_db_neigh)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_neigh, p_neigh, sizeof(stbl_neighbor_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->neigh_hash, (void*)p_db_neigh, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->neigh_list, p_db_neigh);

    return SDB_E_NONE;
}

int32
stbl_neighbor_del_neighbor_sync(stbl_neighbor_key_t *p_neigh_key, uint32 sync)
{
    stbl_neighbor_master_t *p_master = _g_p_stbl_neighbor_master;
    stbl_neighbor_t *p_db_neigh = NULL;

    /* 1. lookup entry exist */
    p_db_neigh = stbl_neighbor_get_neighbor(p_neigh_key);
    if (NULL == p_db_neigh)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->neigh_hash, (void*)p_db_neigh);
    ctclib_slistnode_delete(p_master->neigh_list, p_db_neigh);

    /* 4. free db entry */
    XFREE(MEM_STBL_NEIGHBOR, p_db_neigh);

    return SDB_E_NONE;
}

int32
stbl_neighbor_set_neighbor_field_sync(stbl_neighbor_t *p_neigh, stbl_neighbor_field_id_t field_id, uint32 sync)
{
    stbl_neighbor_t *p_db_neigh = NULL;

    /* 1. lookup entry exist */
    p_db_neigh = stbl_neighbor_get_neighbor(&p_neigh->key);
    if (NULL == p_db_neigh)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_NEIGHBOR_FLD_MAC:
        sal_memcpy(p_db_neigh->mac, p_neigh->mac, sizeof(p_neigh->mac));
        break;

    case STBL_NEIGHBOR_FLD_PORT_OID:
        p_db_neigh->port_oid = p_neigh->port_oid;
        break;

    case STBL_NEIGHBOR_FLD_NH_OID:
        p_db_neigh->nh_oid = p_neigh->nh_oid;
        break;

    case STBL_NEIGHBOR_FLD_ACTION:
        p_db_neigh->action = p_neigh->action;
        break;

    case STBL_NEIGHBOR_FLD_MAX:
        sal_memcpy(p_db_neigh, p_neigh, sizeof(stbl_neighbor_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_neighbor_add_neighbor(stbl_neighbor_t *p_neigh)
{
    return stbl_neighbor_add_neighbor_sync(p_neigh, TRUE);
}

int32
stbl_neighbor_del_neighbor(stbl_neighbor_key_t *p_neigh_key)
{
    return stbl_neighbor_del_neighbor_sync(p_neigh_key, TRUE);
}

int32
stbl_neighbor_set_neighbor_field(stbl_neighbor_t *p_neigh, stbl_neighbor_field_id_t field_id)
{
    return stbl_neighbor_set_neighbor_field_sync(p_neigh, field_id, TRUE);
}

stbl_neighbor_t*
stbl_neighbor_get_neighbor(stbl_neighbor_key_t *p_neigh_key)
{
    stbl_neighbor_master_t *p_master = _g_p_stbl_neighbor_master;
    stbl_neighbor_t lkp;

    sal_memcpy(&lkp.key, p_neigh_key, sizeof(stbl_neighbor_key_t));
    return ctclib_hash_lookup(p_master->neigh_hash, &lkp);
}

char*
stbl_neighbor_key_val2str(stbl_neighbor_t *p_neigh, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_neighbor_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_neigh->key));
    return str;
}

int32
stbl_neighbor_dump_one(stbl_neighbor_t *p_neigh, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_NEIGHBOR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_NEIGHBOR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEIGHBOR_FLD_KEY].name, 
            sdb_neighbor_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_neigh->key));
    }
    if (SDB_FLD_MATCH(STBL_NEIGHBOR_FLD_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEIGHBOR_FLD_MAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_neigh->mac));
    }
    if (SDB_FLD_MATCH(STBL_NEIGHBOR_FLD_PORT_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEIGHBOR_FLD_PORT_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_neigh->port_oid));
    }
    if (SDB_FLD_MATCH(STBL_NEIGHBOR_FLD_NH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEIGHBOR_FLD_NH_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_neigh->nh_oid));
    }
    if (SDB_FLD_MATCH(STBL_NEIGHBOR_FLD_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEIGHBOR_FLD_ACTION].name, 
            sdb_enum_val2str(sai_packet_action_strs, SAI_PACKET_ACTION_MAX, p_neigh->action));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_neighbor_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_neighbor_master_t *p_master = _g_p_stbl_neighbor_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    stbl_neighbor_t *p_db_neigh = NULL;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->neigh_list, p_db_neigh, listnode, next)
    {
        rc |= fn(p_db_neigh, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_neighbor_master_t*
stbl_neighbor_get_master()
{
    return _g_p_stbl_neighbor_master;
}

stbl_neighbor_master_t*
stbl_neighbor_init_neighbor()
{
    _g_p_stbl_neighbor_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_neighbor_master_t));
    _g_p_stbl_neighbor_master->neigh_hash = ctclib_hash_create(_stbl_neighbor_hash_make, _stbl_neighbor_hash_cmp);
    _g_p_stbl_neighbor_master->neigh_list = ctclib_slist_create(stbl_neighbor_cmp, NULL);
    return _g_p_stbl_neighbor_master;
}


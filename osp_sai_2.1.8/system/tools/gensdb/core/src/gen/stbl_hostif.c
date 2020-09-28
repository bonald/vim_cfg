
#include "sdb.h"
#include "gen/stbl_hostif_define.h"
#include "gen/stbl_hostif.h"
#include "sdb_data_cmp.h"

stbl_hostif_master_t *_g_p_stbl_hostif_master = NULL;

static uint32
_stbl_hostif_hash_make(stbl_hostif_t *p_hif)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_hif->key;
    for (index = 0; index < sizeof(p_hif->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_hostif_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_hostif_t *p_hif1 = (stbl_hostif_t*)p_arg1;
    stbl_hostif_t *p_hif2 = (stbl_hostif_t*)p_arg2;

    if (0 == sal_memcmp(&p_hif1->key, &p_hif2->key, sizeof(stbl_hostif_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_hostif_add_hostif_sync(stbl_hostif_t *p_hif, uint32 sync)
{
    stbl_hostif_master_t *p_master = _g_p_stbl_hostif_master;
    stbl_hostif_t *p_db_hif = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_hostif_get_hostif(&p_hif->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_hif = XCALLOC(MEM_STBL_HOSTIF, sizeof(stbl_hostif_t));
    if (NULL == p_db_hif)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_hif, p_hif, sizeof(stbl_hostif_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->hif_hash, (void*)p_db_hif, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_hif);

    return SDB_E_NONE;
}

int32
stbl_hostif_del_hostif_sync(stbl_hostif_key_t *p_hif_key, uint32 sync)
{
    stbl_hostif_master_t *p_master = _g_p_stbl_hostif_master;
    stbl_hostif_t *p_db_hif = NULL;

    /* 1. lookup entry exist */
    p_db_hif = stbl_hostif_get_hostif(p_hif_key);
    if (NULL == p_db_hif)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->hif_hash, (void*)p_db_hif);
    ctclib_slistnode_delete(p_master->if_list, p_db_hif);

    /* 4. free db entry */
    XFREE(MEM_STBL_HOSTIF, p_db_hif);

    return SDB_E_NONE;
}

int32
stbl_hostif_set_hostif_field_sync(stbl_hostif_t *p_hif, stbl_hostif_field_id_t field_id, uint32 sync)
{
    stbl_hostif_t *p_db_hif = NULL;

    /* 1. lookup entry exist */
    p_db_hif = stbl_hostif_get_hostif(&p_hif->key);
    if (NULL == p_db_hif)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_HOSTIF_FLD_PORT_OID:
        p_db_hif->port_oid = p_hif->port_oid;
        break;

    case STBL_HOSTIF_FLD_VID:
        p_db_hif->vid = p_hif->vid;
        break;

    case STBL_HOSTIF_FLD_FD:
        p_db_hif->fd = p_hif->fd;
        break;

    case STBL_HOSTIF_FLD_NEXTHOP_PTR:
        p_db_hif->nexthop_ptr = p_hif->nexthop_ptr;
        break;

    case STBL_HOSTIF_FLD_IFNAME:
        sal_memcpy(p_db_hif->ifname, p_hif->ifname, sizeof(p_hif->ifname));
        break;

    case STBL_HOSTIF_FLD_MAX:
        sal_memcpy(p_db_hif, p_hif, sizeof(stbl_hostif_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_hostif_add_hostif(stbl_hostif_t *p_hif)
{
    return stbl_hostif_add_hostif_sync(p_hif, TRUE);
}

int32
stbl_hostif_del_hostif(stbl_hostif_key_t *p_hif_key)
{
    return stbl_hostif_del_hostif_sync(p_hif_key, TRUE);
}

int32
stbl_hostif_set_hostif_field(stbl_hostif_t *p_hif, stbl_hostif_field_id_t field_id)
{
    return stbl_hostif_set_hostif_field_sync(p_hif, field_id, TRUE);
}

stbl_hostif_t*
stbl_hostif_get_hostif(stbl_hostif_key_t *p_hif_key)
{
    stbl_hostif_master_t *p_master = _g_p_stbl_hostif_master;
    stbl_hostif_t lkp;

    sal_memcpy(&lkp.key, p_hif_key, sizeof(stbl_hostif_key_t));
    return ctclib_hash_lookup(p_master->hif_hash, &lkp);
}

char*
stbl_hostif_key_val2str(stbl_hostif_t *p_hif, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_hif->key.hif_oid));
    return str;
}

int32
stbl_hostif_dump_one(stbl_hostif_t *p_hif, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_HOSTIF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_HOSTIF_FLD_KEY].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_hif->key.hif_oid));
    }
    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_PORT_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_HOSTIF_FLD_PORT_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_hif->port_oid));
    }
    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_HOSTIF_FLD_VID].name,
            p_hif->vid);
    }
    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_FD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[STBL_HOSTIF_FLD_FD].name,
            p_hif->fd);
    }
    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_NEXTHOP_PTR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_HOSTIF_FLD_NEXTHOP_PTR].name,
            p_hif->nexthop_ptr);
    }
    if (SDB_FLD_MATCH(STBL_HOSTIF_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_HOSTIF_FLD_IFNAME].name,
            p_hif->ifname);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_hostif_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_hostif_master_t *p_master = _g_p_stbl_hostif_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    stbl_hostif_t *p_db_hif = NULL;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_hif, listnode, next)
    {
        rc |= fn(p_db_hif, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_hostif_master_t*
stbl_hostif_get_master()
{
    return _g_p_stbl_hostif_master;
}

stbl_hostif_master_t*
stbl_hostif_init_hostif()
{
    _g_p_stbl_hostif_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_hostif_master_t));
    _g_p_stbl_hostif_master->hif_hash = ctclib_hash_create(_stbl_hostif_hash_make, _stbl_hostif_hash_cmp);
    _g_p_stbl_hostif_master->if_list = ctclib_slist_create(stbl_hostif_cmp, NULL);
    return _g_p_stbl_hostif_master;
}



#include "sdb.h"
#include "gen/stbl_acl_port_define.h"
#include "gen/stbl_acl_port.h"
#include "sdb_data_cmp.h"

stbl_acl_port_master_t *_g_p_stbl_acl_port_master = NULL;

int32
stbl_acl_port_add_acl_port_sync(stbl_acl_port_t *p_port, uint32 sync)
{
    stbl_acl_port_master_t *p_master = _g_p_stbl_acl_port_master;
    stbl_acl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    if (stbl_acl_port_get_acl_port(&p_port->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port = XCALLOC(MEM_STBL_ACL_PORT, sizeof(stbl_acl_port_t));
    if (NULL == p_db_port)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port, p_port, sizeof(stbl_acl_port_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->port_list, p_db_port);

    return SDB_E_NONE;
}

int32
stbl_acl_port_del_acl_port_sync(stbl_acl_port_key_t *p_port_key, uint32 sync)
{
    stbl_acl_port_master_t *p_master = _g_p_stbl_acl_port_master;
    stbl_acl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = stbl_acl_port_get_acl_port(p_port_key);
    if (NULL == p_db_port)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->port_list, p_db_port);

    /* 4. free db entry */
    XFREE(MEM_STBL_ACL_PORT, p_db_port);

    return SDB_E_NONE;
}

int32
stbl_acl_port_set_acl_port_field_sync(stbl_acl_port_t *p_port, stbl_acl_port_field_id_t field_id, uint32 sync)
{
    stbl_acl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = stbl_acl_port_get_acl_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_PORT_FLD_INGRESS_EN_CNT:
        p_db_port->ingress_en_cnt = p_port->ingress_en_cnt;
        break;

    case STBL_ACL_PORT_FLD_EGRESS_EN_CNT:
        p_db_port->egress_en_cnt = p_port->egress_en_cnt;
        break;

    case STBL_ACL_PORT_FLD_MAX:
        sal_memcpy(p_db_port, p_port, sizeof(stbl_acl_port_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_port_add_acl_port(stbl_acl_port_t *p_port)
{
    return stbl_acl_port_add_acl_port_sync(p_port, TRUE);
}

int32
stbl_acl_port_del_acl_port(stbl_acl_port_key_t *p_port_key)
{
    return stbl_acl_port_del_acl_port_sync(p_port_key, TRUE);
}

int32
stbl_acl_port_set_acl_port_field(stbl_acl_port_t *p_port, stbl_acl_port_field_id_t field_id)
{
    return stbl_acl_port_set_acl_port_field_sync(p_port, field_id, TRUE);
}

stbl_acl_port_t*
stbl_acl_port_get_acl_port(stbl_acl_port_key_t *p_port_key)
{
    stbl_acl_port_master_t *p_master = _g_p_stbl_acl_port_master;
    stbl_acl_port_t lkp;

    sal_memcpy(&lkp.key, p_port_key, sizeof(stbl_acl_port_key_t));
    return ctclib_slistdata_lookup(p_master->port_list, &lkp);
}

char*
stbl_acl_port_key_val2str(stbl_acl_port_t *p_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->key.port_oid));
    return str;
}

int32
stbl_acl_port_dump_one(stbl_acl_port_t *p_port, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ACL_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_PORT_FLD_KEY].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_port->key.port_oid));
    }
    if (SDB_FLD_MATCH(STBL_ACL_PORT_FLD_INGRESS_EN_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_PORT_FLD_INGRESS_EN_CNT].name,
            p_port->ingress_en_cnt);
    }
    if (SDB_FLD_MATCH(STBL_ACL_PORT_FLD_EGRESS_EN_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_PORT_FLD_EGRESS_EN_CNT].name,
            p_port->egress_en_cnt);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_port_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_port_master_t *p_master = _g_p_stbl_acl_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    stbl_acl_port_t *p_db_port = NULL;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_list, p_db_port, listnode, next)
    {
        rc |= fn(p_db_port, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_acl_port_master_t*
stbl_acl_port_get_master()
{
    return _g_p_stbl_acl_port_master;
}

stbl_acl_port_master_t*
stbl_acl_port_init_acl_port()
{
    _g_p_stbl_acl_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_acl_port_master_t));
    _g_p_stbl_acl_port_master->port_list = ctclib_slist_create(stbl_acl_port_cmp, NULL);
    return _g_p_stbl_acl_port_master;
}


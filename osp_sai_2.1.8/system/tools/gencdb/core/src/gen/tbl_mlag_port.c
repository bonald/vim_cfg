
#include "proto.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "cdb_data_cmp.h"

tbl_mlag_port_master_t *_g_p_tbl_mlag_port_master = NULL;

int32
tbl_mlag_port_add_mlag_port_sync(tbl_mlag_port_t *p_port, uint32 sync)
{
    tbl_mlag_port_master_t *p_master = _g_p_tbl_mlag_port_master;
    tbl_mlag_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    if (tbl_mlag_port_get_mlag_port(&p_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port = XCALLOC(MEM_TBL_MLAG_PORT, sizeof(tbl_mlag_port_t));
    if (NULL == p_db_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port, p_port, sizeof(tbl_mlag_port_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->port_list, p_db_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MLAG_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MLAG_PORT, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mlag_port_del_mlag_port_sync(tbl_mlag_port_key_t *p_port_key, uint32 sync)
{
    tbl_mlag_port_master_t *p_master = _g_p_tbl_mlag_port_master;
    tbl_mlag_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_mlag_port_get_mlag_port(p_port_key);
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MLAG_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MLAG_PORT, p_db_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->port_list, p_db_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_MLAG_PORT, p_db_port);

    return PM_E_NONE;
}

int32
tbl_mlag_port_set_mlag_port_field_sync(tbl_mlag_port_t *p_port, tbl_mlag_port_field_id_t field_id, uint32 sync)
{
    tbl_mlag_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_mlag_port_get_mlag_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MLAG_PORT_FLD_PEER_CONF:
        p_db_port->peer_conf = p_port->peer_conf;
        break;

    case TBL_MLAG_PORT_FLD_PEER_IF_UP:
        p_db_port->peer_if_up = p_port->peer_if_up;
        break;

    case TBL_MLAG_PORT_FLD_STP_PORT_ENABLE:
        p_db_port->stp_port_enable = p_port->stp_port_enable;
        break;

    case TBL_MLAG_PORT_FLD_IFINDEX:
        p_db_port->ifindex = p_port->ifindex;
        break;

    case TBL_MLAG_PORT_FLD_PROTECT_EN:
        p_db_port->protect_en = p_port->protect_en;
        break;

    case TBL_MLAG_PORT_FLD_MAX:
        sal_memcpy(p_db_port, p_port, sizeof(tbl_mlag_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MLAG_PORT, field_id, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MLAG_PORT, field_id, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mlag_port_add_mlag_port(tbl_mlag_port_t *p_port)
{
    return tbl_mlag_port_add_mlag_port_sync(p_port, TRUE);
}

int32
tbl_mlag_port_del_mlag_port(tbl_mlag_port_key_t *p_port_key)
{
    return tbl_mlag_port_del_mlag_port_sync(p_port_key, TRUE);
}

int32
tbl_mlag_port_set_mlag_port_field(tbl_mlag_port_t *p_port, tbl_mlag_port_field_id_t field_id)
{
    return tbl_mlag_port_set_mlag_port_field_sync(p_port, field_id, TRUE);
}

tbl_mlag_port_t*
tbl_mlag_port_get_mlag_port(tbl_mlag_port_key_t *p_port_key)
{
    tbl_mlag_port_master_t *p_master = _g_p_tbl_mlag_port_master;
    tbl_mlag_port_t lkp;

    sal_memcpy(&lkp.key, p_port_key, sizeof(tbl_mlag_port_key_t));
    return ctclib_slistdata_lookup(p_master->port_list, &lkp);
}

char*
tbl_mlag_port_key_val2str(tbl_mlag_port_t *p_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_port->key.id);
    return str;
}

int32
tbl_mlag_port_key_str2val(char *str, tbl_mlag_port_t *p_port)
{
    int32 ret = 0;

    p_port->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_mlag_port_key_name_dump(tbl_mlag_port_t *p_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_port->key.id);
    return str;
}

char*
tbl_mlag_port_key_value_dump(tbl_mlag_port_t *p_port, char *str)
{
    sal_sprintf(str, "%u", p_port->key.id);
    return str;
}

char*
tbl_mlag_port_field_name_dump(tbl_mlag_port_t *p_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MLAG_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mlag_port_field_value_dump(tbl_mlag_port_t *p_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_port->key.id);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_CONF, field_id))
    {
        sal_sprintf(str, "%u", p_port->peer_conf);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_IF_UP, field_id))
    {
        sal_sprintf(str, "%u", p_port->peer_if_up);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_STP_PORT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_port->stp_port_enable);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_port->ifindex);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PROTECT_EN, field_id))
    {
        sal_sprintf(str, "%u", p_port->protect_en);
    }
    return str;
}

char**
tbl_mlag_port_table_dump(tbl_mlag_port_t *p_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mlag_port_key_name_dump(p_port, buf));
    for(i=1; i<TBL_MLAG_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mlag_port_field_name_dump(p_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mlag_port_field_value_dump(p_port, i, buf));
    }
    return str;
}

int32
tbl_mlag_port_field_value_parser(char *str, int32 field_id, tbl_mlag_port_t *p_port)
{
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_KEY, field_id))
    {
        int32 ret;
        p_port->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_CONF, field_id))
    {
        int32 ret;
        p_port->peer_conf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_IF_UP, field_id))
    {
        int32 ret;
        p_port->peer_if_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_STP_PORT_ENABLE, field_id))
    {
        int32 ret;
        p_port->stp_port_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_port->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PROTECT_EN, field_id))
    {
        int32 ret;
        p_port->protect_en = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_mlag_port_table_parser(char** array, char* key_value,tbl_mlag_port_t *p_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mlag_port_key_str2val(key_value, p_port));

    for(i=1; i<TBL_MLAG_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mlag_port_field_value_parser( array[j++], i, p_port));
    }

    return PM_E_NONE;
}

int32
tbl_mlag_port_dump_one(tbl_mlag_port_t *p_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MLAG_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_MLAG_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_KEY].name,
            p_port->key.id);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_CONF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_PEER_CONF].name,
            p_port->peer_conf);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PEER_IF_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_PEER_IF_UP].name,
            p_port->peer_if_up);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_STP_PORT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_STP_PORT_ENABLE].name,
            p_port->stp_port_enable);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_IFINDEX].name,
            p_port->ifindex);
    }
    if (FLD_MATCH(TBL_MLAG_PORT_FLD_PROTECT_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MLAG_PORT_FLD_PROTECT_EN].name,
            p_port->protect_en);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mlag_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mlag_port_master_t *p_master = _g_p_tbl_mlag_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mlag_port_t *p_db_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_list, p_db_port, listnode, next)
    {
        rc |= fn(p_db_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mlag_port_master_t*
tbl_mlag_port_get_master()
{
    return _g_p_tbl_mlag_port_master;
}

tbl_mlag_port_master_t*
tbl_mlag_port_init_mlag_port()
{
    _g_p_tbl_mlag_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mlag_port_master_t));
    _g_p_tbl_mlag_port_master->port_list = ctclib_slist_create(tbl_mlag_port_cmp, NULL);
    return _g_p_tbl_mlag_port_master;
}


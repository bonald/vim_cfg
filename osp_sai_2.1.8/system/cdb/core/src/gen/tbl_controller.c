
#include "proto.h"
#include "gen/tbl_controller_define.h"
#include "gen/tbl_controller.h"
#include "cdb_data_cmp.h"

tbl_controller_master_t *_g_p_tbl_controller_master = NULL;

int32
tbl_controller_add_controller_sync(tbl_controller_t *p_controller, uint32 sync)
{
    tbl_controller_master_t *p_master = _g_p_tbl_controller_master;
    tbl_controller_t *p_db_controller = NULL;

    /* 1. lookup entry exist */
    if (tbl_controller_get_controller(&p_controller->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_controller = XCALLOC(MEM_TBL_CONTROLLER, sizeof(tbl_controller_t));
    if (NULL == p_db_controller)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_controller, p_controller, sizeof(tbl_controller_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->controller_list, p_db_controller);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CONTROLLER, p_db_controller);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CONTROLLER, p_db_controller);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_controller_del_controller_sync(tbl_controller_key_t *p_controller_key, uint32 sync)
{
    tbl_controller_master_t *p_master = _g_p_tbl_controller_master;
    tbl_controller_t *p_db_controller = NULL;

    /* 1. lookup entry exist */
    p_db_controller = tbl_controller_get_controller(p_controller_key);
    if (NULL == p_db_controller)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CONTROLLER, p_db_controller);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CONTROLLER, p_db_controller);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->controller_list, p_db_controller);

    /* 4. free db entry */
    XFREE(MEM_TBL_CONTROLLER, p_db_controller);

    return PM_E_NONE;
}

int32
tbl_controller_set_controller_field_sync(tbl_controller_t *p_controller, tbl_controller_field_id_t field_id, uint32 sync)
{
    tbl_controller_t *p_db_controller = NULL;

    /* 1. lookup entry exist */
    p_db_controller = tbl_controller_get_controller(&p_controller->key);
    if (NULL == p_db_controller)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CONTROLLER_FLD_IP:
        sal_memcpy(&p_db_controller->ip, &p_controller->ip, sizeof(p_controller->ip));
        break;

    case TBL_CONTROLLER_FLD_PORT:
        p_db_controller->port = p_controller->port;
        break;

    case TBL_CONTROLLER_FLD_MAX:
        sal_memcpy(p_db_controller, p_controller, sizeof(tbl_controller_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CONTROLLER, field_id, p_db_controller);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CONTROLLER, field_id, p_db_controller);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_controller_add_controller(tbl_controller_t *p_controller)
{
    return tbl_controller_add_controller_sync(p_controller, TRUE);
}

int32
tbl_controller_del_controller(tbl_controller_key_t *p_controller_key)
{
    return tbl_controller_del_controller_sync(p_controller_key, TRUE);
}

int32
tbl_controller_set_controller_field(tbl_controller_t *p_controller, tbl_controller_field_id_t field_id)
{
    return tbl_controller_set_controller_field_sync(p_controller, field_id, TRUE);
}

tbl_controller_t*
tbl_controller_get_controller(tbl_controller_key_t *p_controller_key)
{
    tbl_controller_master_t *p_master = _g_p_tbl_controller_master;
    tbl_controller_t lkp;

    sal_memcpy(&lkp.key, p_controller_key, sizeof(tbl_controller_key_t));
    return ctclib_slistdata_lookup(p_master->controller_list, &lkp);
}

char*
tbl_controller_key_val2str(tbl_controller_t *p_controller, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_controller->key.name);
    return str;
}

int32
tbl_controller_key_str2val(char *str, tbl_controller_t *p_controller)
{
    int32 ret = 0;

    sal_strcpy(p_controller->key.name, str);
    return ret;
}

char*
tbl_controller_key_name_dump(tbl_controller_t *p_controller, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_controller->key.name);
    return str;
}

char*
tbl_controller_key_value_dump(tbl_controller_t *p_controller, char *str)
{
    sal_sprintf(str, "%s", p_controller->key.name);
    return str;
}

char*
tbl_controller_field_name_dump(tbl_controller_t *p_controller, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CONTROLLER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_controller_field_value_dump(tbl_controller_t *p_controller, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CONTROLLER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_controller->key.name);
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_IP, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_controller->ip);
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_controller->port);
    }
    return str;
}

char**
tbl_controller_table_dump(tbl_controller_t *p_controller, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_controller_key_name_dump(p_controller, buf));
    for(i=1; i<TBL_CONTROLLER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_controller_field_name_dump(p_controller, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_controller_field_value_dump(p_controller, i, buf));
    }
    return str;
}

int32
tbl_controller_field_value_parser(char *str, int32 field_id, tbl_controller_t *p_controller)
{
    if (FLD_MATCH(TBL_CONTROLLER_FLD_KEY, field_id))
    {
        sal_strcpy(p_controller->key.name, str);
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_IP, field_id))
    {
        cdb_addr_ipv4_str2val(&p_controller->ip, str, 0);
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_PORT, field_id))
    {
        int32 ret;
        p_controller->port = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_controller_table_parser(char** array, char* key_value,tbl_controller_t *p_controller)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_controller_key_str2val(key_value, p_controller));

    for(i=1; i<TBL_CONTROLLER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_controller_field_value_parser( array[j++], i, p_controller));
    }

    return PM_E_NONE;
}

int32
tbl_controller_dump_one(tbl_controller_t *p_controller, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONTROLLER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CONTROLLER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONTROLLER_FLD_KEY].name,
            p_controller->key.name);
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONTROLLER_FLD_IP].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_controller->ip));
    }
    if (FLD_MATCH(TBL_CONTROLLER_FLD_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONTROLLER_FLD_PORT].name,
            p_controller->port);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_controller_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_controller_master_t *p_master = _g_p_tbl_controller_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_controller_t *p_db_controller = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->controller_list, p_db_controller, listnode, next)
    {
        rc |= fn(p_db_controller, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_controller_master_t*
tbl_controller_get_master()
{
    return _g_p_tbl_controller_master;
}

tbl_controller_master_t*
tbl_controller_init_controller()
{
    _g_p_tbl_controller_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_controller_master_t));
    _g_p_tbl_controller_master->controller_list = ctclib_slist_create(tbl_controller_cmp, NULL);
    return _g_p_tbl_controller_master;
}


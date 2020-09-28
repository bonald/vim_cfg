
#include "proto.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "cdb_data_cmp.h"

tbl_snmp_trap_master_t *_g_p_tbl_snmp_trap_master = NULL;

int32
tbl_snmp_trap_add_snmp_trap_sync(tbl_snmp_trap_t *p_snmp_trap, uint32 sync)
{
    tbl_snmp_trap_master_t *p_master = _g_p_tbl_snmp_trap_master;
    tbl_snmp_trap_t *p_db_snmp_trap = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_trap_get_snmp_trap(&p_snmp_trap->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_trap = XCALLOC(MEM_TBL_SNMP_TRAP, sizeof(tbl_snmp_trap_t));
    if (NULL == p_db_snmp_trap)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_trap, p_snmp_trap, sizeof(tbl_snmp_trap_t));

    /* 4. add to db */
    ctclib_slistnode_insert(p_master->snmp_trap_list, p_db_snmp_trap);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_TRAP, p_db_snmp_trap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_TRAP, p_db_snmp_trap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_trap_del_snmp_trap_sync(tbl_snmp_trap_key_t *p_snmp_trap_key, uint32 sync)
{
    tbl_snmp_trap_master_t *p_master = _g_p_tbl_snmp_trap_master;
    tbl_snmp_trap_t *p_db_snmp_trap = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_trap = tbl_snmp_trap_get_snmp_trap(p_snmp_trap_key);
    if (NULL == p_db_snmp_trap)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_TRAP, p_db_snmp_trap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_TRAP, p_db_snmp_trap);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->snmp_trap_list, p_db_snmp_trap);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_TRAP, p_db_snmp_trap);

    return PM_E_NONE;
}

int32
tbl_snmp_trap_set_snmp_trap_field_sync(tbl_snmp_trap_t *p_snmp_trap, tbl_snmp_trap_field_id_t field_id, uint32 sync)
{
    tbl_snmp_trap_t *p_db_snmp_trap = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_trap = tbl_snmp_trap_get_snmp_trap(&p_snmp_trap->key);
    if (NULL == p_db_snmp_trap)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_TRAP_FLD_IS_INBAND:
        p_db_snmp_trap->is_inband = p_snmp_trap->is_inband;
        break;

    case TBL_SNMP_TRAP_FLD_MAX:
        sal_memcpy(p_db_snmp_trap, p_snmp_trap, sizeof(tbl_snmp_trap_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_TRAP, field_id, p_db_snmp_trap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_TRAP, field_id, p_db_snmp_trap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_trap_add_snmp_trap(tbl_snmp_trap_t *p_snmp_trap)
{
    return tbl_snmp_trap_add_snmp_trap_sync(p_snmp_trap, TRUE);
}

int32
tbl_snmp_trap_del_snmp_trap(tbl_snmp_trap_key_t *p_snmp_trap_key)
{
    return tbl_snmp_trap_del_snmp_trap_sync(p_snmp_trap_key, TRUE);
}

int32
tbl_snmp_trap_set_snmp_trap_field(tbl_snmp_trap_t *p_snmp_trap, tbl_snmp_trap_field_id_t field_id)
{
    return tbl_snmp_trap_set_snmp_trap_field_sync(p_snmp_trap, field_id, TRUE);
}

tbl_snmp_trap_t*
tbl_snmp_trap_get_snmp_trap(tbl_snmp_trap_key_t *p_snmp_trap_key)
{
    tbl_snmp_trap_master_t *p_master = _g_p_tbl_snmp_trap_master;
    tbl_snmp_trap_t lkp;

    sal_memcpy(&lkp.key, p_snmp_trap_key, sizeof(tbl_snmp_trap_key_t));
    return ctclib_slistdata_lookup(p_master->snmp_trap_list, &lkp);
}

char*
tbl_snmp_trap_key_val2str(tbl_snmp_trap_t *p_snmp_trap, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_snmp_trap_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_trap->key.trap));
    return str;
}

int32
tbl_snmp_trap_key_str2val(char *str, tbl_snmp_trap_t *p_snmp_trap)
{
    int32 ret = 0;

    ret = cdb_snmp_trap_str2val(str, &p_snmp_trap->key.trap);
    return ret;
}

char*
tbl_snmp_trap_key_name_dump(tbl_snmp_trap_t *p_snmp_trap, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_snmp_trap_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_trap->key.trap));
    return str;
}

char*
tbl_snmp_trap_key_value_dump(tbl_snmp_trap_t *p_snmp_trap, char *str)
{

    cdb_snmp_trap_val2str(str, MAX_CMD_STR_LEN, &p_snmp_trap->key.trap);
    return str;
}

char*
tbl_snmp_trap_field_name_dump(tbl_snmp_trap_t *p_snmp_trap, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_TRAP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_trap_field_value_dump(tbl_snmp_trap_t *p_snmp_trap, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_KEY, field_id))
    {
        cdb_snmp_trap_val2str(str, MAX_CMD_STR_LEN, &p_snmp_trap->key.trap);
    }
    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_trap->is_inband);
    }
    return str;
}

char**
tbl_snmp_trap_table_dump(tbl_snmp_trap_t *p_snmp_trap, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_trap_key_name_dump(p_snmp_trap, buf));
    for(i=1; i<TBL_SNMP_TRAP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_trap_field_name_dump(p_snmp_trap, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_trap_field_value_dump(p_snmp_trap, i, buf));
    }
    return str;
}

int32
tbl_snmp_trap_field_value_parser(char *str, int32 field_id, tbl_snmp_trap_t *p_snmp_trap)
{
    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_KEY, field_id))
    {
        cdb_snmp_trap_str2val(str, &p_snmp_trap->key.trap);
    }
    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_snmp_trap->is_inband = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_trap_table_parser(char** array, char* key_value,tbl_snmp_trap_t *p_snmp_trap)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_trap_key_str2val(key_value, p_snmp_trap));

    for(i=1; i<TBL_SNMP_TRAP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_trap_field_value_parser( array[j++], i, p_snmp_trap));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_trap_dump_one(tbl_snmp_trap_t *p_snmp_trap, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TRAP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TRAP_FLD_KEY].name, 
            cdb_snmp_trap_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_trap->key.trap));
    }
    if (FLD_MATCH(TBL_SNMP_TRAP_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TRAP_FLD_IS_INBAND].name,
            p_snmp_trap->is_inband);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_trap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_trap_master_t *p_master = _g_p_tbl_snmp_trap_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_trap_t *p_db_snmp_trap = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_trap_list, p_db_snmp_trap, listnode, next)
    {
        rc |= fn(p_db_snmp_trap, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_trap_master_t*
tbl_snmp_trap_get_master()
{
    return _g_p_tbl_snmp_trap_master;
}

tbl_snmp_trap_master_t*
tbl_snmp_trap_init_snmp_trap()
{
    _g_p_tbl_snmp_trap_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_trap_master_t));
    _g_p_tbl_snmp_trap_master->snmp_trap_list = ctclib_slist_create(tbl_snmp_trap_cmp, NULL);
    return _g_p_tbl_snmp_trap_master;
}


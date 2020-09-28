
#include "proto.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "cdb_data_cmp.h"

tbl_vlan_mapping_table_master_t *_g_p_tbl_vlan_mapping_table_master = NULL;

static uint32
_tbl_vlan_mapping_table_hash_make(tbl_vlan_mapping_table_t *p_vlan_mapping_table)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vlan_mapping_table->key;
    for (index = 0; index < sizeof(p_vlan_mapping_table->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_vlan_mapping_table_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_vlan_mapping_table_t *p_vlan_mapping_table1 = (tbl_vlan_mapping_table_t*)p_arg1;
    tbl_vlan_mapping_table_t *p_vlan_mapping_table2 = (tbl_vlan_mapping_table_t*)p_arg2;

    if (0 == sal_memcmp(&p_vlan_mapping_table1->key, &p_vlan_mapping_table2->key, sizeof(tbl_vlan_mapping_table_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_vlan_mapping_table_add_vlan_mapping_table_sync(tbl_vlan_mapping_table_t *p_vlan_mapping_table, uint32 sync)
{
    tbl_vlan_mapping_table_master_t *p_master = _g_p_tbl_vlan_mapping_table_master;
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlan_mapping_table_get_vlan_mapping_table(&p_vlan_mapping_table->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan_mapping_table = XCALLOC(MEM_TBL_VLAN_MAPPING_TABLE, sizeof(tbl_vlan_mapping_table_t));
    if (NULL == p_db_vlan_mapping_table)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan_mapping_table, p_vlan_mapping_table, sizeof(tbl_vlan_mapping_table_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->vlan_mapping_table_hash, (void*)p_db_vlan_mapping_table, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->vlan_mapping_table_list, p_db_vlan_mapping_table);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLAN_MAPPING_TABLE, p_db_vlan_mapping_table);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLAN_MAPPING_TABLE, p_db_vlan_mapping_table);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_del_vlan_mapping_table_sync(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key, uint32 sync)
{
    tbl_vlan_mapping_table_master_t *p_master = _g_p_tbl_vlan_mapping_table_master;
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_mapping_table = tbl_vlan_mapping_table_get_vlan_mapping_table(p_vlan_mapping_table_key);
    if (NULL == p_db_vlan_mapping_table)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLAN_MAPPING_TABLE, p_db_vlan_mapping_table);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLAN_MAPPING_TABLE, p_db_vlan_mapping_table);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->vlan_mapping_table_hash, (void*)p_db_vlan_mapping_table);
    ctclib_slistnode_delete(p_master->vlan_mapping_table_list, p_db_vlan_mapping_table);

    /* 4. free db entry */
    XFREE(MEM_TBL_VLAN_MAPPING_TABLE, p_db_vlan_mapping_table);

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_set_vlan_mapping_table_field_sync(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_vlan_mapping_table_field_id_t field_id, uint32 sync)
{
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_mapping_table = tbl_vlan_mapping_table_get_vlan_mapping_table(&p_vlan_mapping_table->key);
    if (NULL == p_db_vlan_mapping_table)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS:
        sal_memcpy(p_db_vlan_mapping_table->applied_ports, p_vlan_mapping_table->applied_ports, sizeof(p_vlan_mapping_table->applied_ports));
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM:
        p_db_vlan_mapping_table->entry_num = p_vlan_mapping_table->entry_num;
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM:
        p_db_vlan_mapping_table->egress_entry_num = p_vlan_mapping_table->egress_entry_num;
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID:
        p_db_vlan_mapping_table->group_id = p_vlan_mapping_table->group_id;
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID:
        p_db_vlan_mapping_table->group_mapped_svid = p_vlan_mapping_table->group_mapped_svid;
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME:
        sal_memcpy(p_db_vlan_mapping_table->group_evc_entry_name, p_vlan_mapping_table->group_evc_entry_name, sizeof(p_vlan_mapping_table->group_evc_entry_name));
        break;

    case TBL_VLAN_MAPPING_TABLE_FLD_MAX:
        sal_memcpy(p_db_vlan_mapping_table, p_vlan_mapping_table, sizeof(tbl_vlan_mapping_table_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLAN_MAPPING_TABLE, field_id, p_db_vlan_mapping_table);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLAN_MAPPING_TABLE, field_id, p_db_vlan_mapping_table);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_add_vlan_mapping_table(tbl_vlan_mapping_table_t *p_vlan_mapping_table)
{
    return tbl_vlan_mapping_table_add_vlan_mapping_table_sync(p_vlan_mapping_table, TRUE);
}

int32
tbl_vlan_mapping_table_del_vlan_mapping_table(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key)
{
    return tbl_vlan_mapping_table_del_vlan_mapping_table_sync(p_vlan_mapping_table_key, TRUE);
}

int32
tbl_vlan_mapping_table_set_vlan_mapping_table_field(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_vlan_mapping_table_field_id_t field_id)
{
    return tbl_vlan_mapping_table_set_vlan_mapping_table_field_sync(p_vlan_mapping_table, field_id, TRUE);
}

tbl_vlan_mapping_table_t*
tbl_vlan_mapping_table_get_vlan_mapping_table(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key)
{
    tbl_vlan_mapping_table_master_t *p_master = _g_p_tbl_vlan_mapping_table_master;
    tbl_vlan_mapping_table_t lkp;

    sal_memcpy(&lkp.key, p_vlan_mapping_table_key, sizeof(tbl_vlan_mapping_table_key_t));
    return ctclib_hash_lookup(p_master->vlan_mapping_table_hash, &lkp);
}

char*
tbl_vlan_mapping_table_key_val2str(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_vlan_mapping_table->key.name);
    return str;
}

int32
tbl_vlan_mapping_table_key_str2val(char *str, tbl_vlan_mapping_table_t *p_vlan_mapping_table)
{
    int32 ret = 0;

    sal_strcpy(p_vlan_mapping_table->key.name, str);
    return ret;
}

char*
tbl_vlan_mapping_table_key_name_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_TABLE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_vlan_mapping_table->key.name);
    return str;
}

char*
tbl_vlan_mapping_table_key_value_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char *str)
{
    sal_sprintf(str, "%s", p_vlan_mapping_table->key.name);
    return str;
}

char*
tbl_vlan_mapping_table_field_name_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_TABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLAN_MAPPING_TABLE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlan_mapping_table_field_value_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_vlan_mapping_table->key.name);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_vlan_mapping_table->applied_ports, sizeof(p_vlan_mapping_table->applied_ports)/4);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_table->entry_num);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_table->egress_entry_num);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_table->group_id);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_table->group_mapped_svid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_vlan_mapping_table->group_evc_entry_name);
    }
    return str;
}

char**
tbl_vlan_mapping_table_table_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlan_mapping_table_key_name_dump(p_vlan_mapping_table, buf));
    for(i=1; i<TBL_VLAN_MAPPING_TABLE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlan_mapping_table_field_name_dump(p_vlan_mapping_table, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlan_mapping_table_field_value_dump(p_vlan_mapping_table, i, buf));
    }
    return str;
}

int32
tbl_vlan_mapping_table_field_value_parser(char *str, int32 field_id, tbl_vlan_mapping_table_t *p_vlan_mapping_table)
{
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_KEY, field_id))
    {
        sal_strcpy(p_vlan_mapping_table->key.name, str);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_vlan_mapping_table->applied_ports, sizeof(p_vlan_mapping_table->applied_ports)/4, err);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_vlan_mapping_table->entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_vlan_mapping_table->egress_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID, field_id))
    {
        int32 ret;
        p_vlan_mapping_table->group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID, field_id))
    {
        int32 ret;
        p_vlan_mapping_table->group_mapped_svid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME, field_id))
    {
        sal_strcpy(p_vlan_mapping_table->group_evc_entry_name, str);
    }
    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_table_parser(char** array, char* key_value,tbl_vlan_mapping_table_t *p_vlan_mapping_table)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_TABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlan_mapping_table_key_str2val(key_value, p_vlan_mapping_table));

    for(i=1; i<TBL_VLAN_MAPPING_TABLE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlan_mapping_table_field_value_parser( array[j++], i, p_vlan_mapping_table));
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_dump_one(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_TABLE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_KEY].name,
            p_vlan_mapping_table->key.name);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_vlan_mapping_table->applied_ports, sizeof(p_vlan_mapping_table->applied_ports)/4));
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM].name,
            p_vlan_mapping_table->entry_num);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM].name,
            p_vlan_mapping_table->egress_entry_num);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID].name,
            p_vlan_mapping_table->group_id);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID].name,
            p_vlan_mapping_table->group_mapped_svid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME].name,
            p_vlan_mapping_table->group_evc_entry_name);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_table_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlan_mapping_table_master_t *p_master = _g_p_tbl_vlan_mapping_table_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_table_list, p_db_vlan_mapping_table, listnode, next)
    {
        rc |= fn(p_db_vlan_mapping_table, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlan_mapping_table_master_t*
tbl_vlan_mapping_table_get_master()
{
    return _g_p_tbl_vlan_mapping_table_master;
}

tbl_vlan_mapping_table_master_t*
tbl_vlan_mapping_table_init_vlan_mapping_table()
{
    _g_p_tbl_vlan_mapping_table_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlan_mapping_table_master_t));
    _g_p_tbl_vlan_mapping_table_master->vlan_mapping_table_hash = ctclib_hash_create(_tbl_vlan_mapping_table_hash_make, _tbl_vlan_mapping_table_hash_cmp);
    _g_p_tbl_vlan_mapping_table_master->vlan_mapping_table_list = ctclib_slist_create(tbl_vlan_mapping_table_cmp, NULL);
    return _g_p_tbl_vlan_mapping_table_master;
}


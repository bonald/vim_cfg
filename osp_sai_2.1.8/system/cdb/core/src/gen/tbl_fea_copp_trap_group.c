
#include "proto.h"
#include "gen/tbl_fea_copp_trap_group_define.h"
#include "gen/tbl_fea_copp_trap_group.h"
#include "cdb_data_cmp.h"

tbl_fea_copp_trap_group_master_t *_g_p_tbl_fea_copp_trap_group_master = NULL;

static uint32
_tbl_fea_copp_trap_group_hash_make(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fea_copp_trap_group->key;
    for (index = 0; index < sizeof(p_fea_copp_trap_group->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_copp_trap_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_copp_trap_group_t *p_fea_copp_trap_group1 = (tbl_fea_copp_trap_group_t*)p_arg1;
    tbl_fea_copp_trap_group_t *p_fea_copp_trap_group2 = (tbl_fea_copp_trap_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_fea_copp_trap_group1->key, &p_fea_copp_trap_group2->key, sizeof(tbl_fea_copp_trap_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, uint32 sync)
{
    tbl_fea_copp_trap_group_master_t *p_master = _g_p_tbl_fea_copp_trap_group_master;
    tbl_fea_copp_trap_group_t *p_db_fea_copp_trap_group = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_copp_trap_group_get_fea_copp_trap_group(&p_fea_copp_trap_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_copp_trap_group = XCALLOC(MEM_TBL_FEA_COPP_TRAP_GROUP, sizeof(tbl_fea_copp_trap_group_t));
    if (NULL == p_db_fea_copp_trap_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_copp_trap_group, p_fea_copp_trap_group, sizeof(tbl_fea_copp_trap_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fea_copp_trap_group_hash, (void*)p_db_fea_copp_trap_group, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->fea_copp_trap_group_list, p_db_fea_copp_trap_group);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_COPP_TRAP_GROUP, p_db_fea_copp_trap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_COPP_TRAP_GROUP, p_db_fea_copp_trap_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_del_fea_copp_trap_group_sync(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key, uint32 sync)
{
    tbl_fea_copp_trap_group_master_t *p_master = _g_p_tbl_fea_copp_trap_group_master;
    tbl_fea_copp_trap_group_t *p_db_fea_copp_trap_group = NULL;

    /* 1. lookup entry exist */
    p_db_fea_copp_trap_group = tbl_fea_copp_trap_group_get_fea_copp_trap_group(p_fea_copp_trap_group_key);
    if (NULL == p_db_fea_copp_trap_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_COPP_TRAP_GROUP, p_db_fea_copp_trap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_COPP_TRAP_GROUP, p_db_fea_copp_trap_group);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fea_copp_trap_group_hash, (void*)p_db_fea_copp_trap_group);
    ctclib_slistnode_delete(p_master->fea_copp_trap_group_list, p_db_fea_copp_trap_group);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_COPP_TRAP_GROUP, p_db_fea_copp_trap_group);

    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_set_fea_copp_trap_group_field_sync(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_fea_copp_trap_group_field_id_t field_id, uint32 sync)
{
    tbl_fea_copp_trap_group_t *p_db_fea_copp_trap_group = NULL;

    /* 1. lookup entry exist */
    p_db_fea_copp_trap_group = tbl_fea_copp_trap_group_get_fea_copp_trap_group(&p_fea_copp_trap_group->key);
    if (NULL == p_db_fea_copp_trap_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID:
        p_db_fea_copp_trap_group->policer_oid = p_fea_copp_trap_group->policer_oid;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID:
        p_db_fea_copp_trap_group->trap_oid = p_fea_copp_trap_group->trap_oid;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID:
        p_db_fea_copp_trap_group->queue_id = p_fea_copp_trap_group->queue_id;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET:
        p_db_fea_copp_trap_group->green_packet = p_fea_copp_trap_group->green_packet;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE:
        p_db_fea_copp_trap_group->green_byte = p_fea_copp_trap_group->green_byte;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET:
        p_db_fea_copp_trap_group->yellow_packet = p_fea_copp_trap_group->yellow_packet;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE:
        p_db_fea_copp_trap_group->yellow_byte = p_fea_copp_trap_group->yellow_byte;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET:
        p_db_fea_copp_trap_group->red_packet = p_fea_copp_trap_group->red_packet;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE:
        p_db_fea_copp_trap_group->red_byte = p_fea_copp_trap_group->red_byte;
        break;

    case TBL_FEA_COPP_TRAP_GROUP_FLD_MAX:
        sal_memcpy(p_db_fea_copp_trap_group, p_fea_copp_trap_group, sizeof(tbl_fea_copp_trap_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_COPP_TRAP_GROUP, field_id, p_db_fea_copp_trap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_COPP_TRAP_GROUP, field_id, p_db_fea_copp_trap_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_add_fea_copp_trap_group(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group)
{
    return tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(p_fea_copp_trap_group, TRUE);
}

int32
tbl_fea_copp_trap_group_del_fea_copp_trap_group(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key)
{
    return tbl_fea_copp_trap_group_del_fea_copp_trap_group_sync(p_fea_copp_trap_group_key, TRUE);
}

int32
tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_fea_copp_trap_group_field_id_t field_id)
{
    return tbl_fea_copp_trap_group_set_fea_copp_trap_group_field_sync(p_fea_copp_trap_group, field_id, TRUE);
}

tbl_fea_copp_trap_group_t*
tbl_fea_copp_trap_group_get_fea_copp_trap_group(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key)
{
    tbl_fea_copp_trap_group_master_t *p_master = _g_p_tbl_fea_copp_trap_group_master;
    tbl_fea_copp_trap_group_t lkp;

    sal_memcpy(&lkp.key, p_fea_copp_trap_group_key, sizeof(tbl_fea_copp_trap_group_key_t));
    return ctclib_hash_lookup(p_master->fea_copp_trap_group_hash, &lkp);
}

char*
tbl_fea_copp_trap_group_key_val2str(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_fea_copp_trap_group->key.copp_trap_reason_id);
    return str;
}

int32
tbl_fea_copp_trap_group_key_str2val(char *str, tbl_fea_copp_trap_group_t *p_fea_copp_trap_group)
{
    int32 ret = 0;

    p_fea_copp_trap_group->key.copp_trap_reason_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_copp_trap_group_key_name_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_COPP_TRAP_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_fea_copp_trap_group->key.copp_trap_reason_id);
    return str;
}

char*
tbl_fea_copp_trap_group_key_value_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char *str)
{
    sal_sprintf(str, "%u", p_fea_copp_trap_group->key.copp_trap_reason_id);
    return str;
}

char*
tbl_fea_copp_trap_group_field_name_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_COPP_TRAP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_COPP_TRAP_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_copp_trap_group_field_value_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_fea_copp_trap_group->key.copp_trap_reason_id);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->policer_oid);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->trap_oid);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_fea_copp_trap_group->queue_id);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->green_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->green_byte);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->yellow_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->yellow_byte);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->red_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_copp_trap_group->red_byte);
    }
    return str;
}

char**
tbl_fea_copp_trap_group_table_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_copp_trap_group_key_name_dump(p_fea_copp_trap_group, buf));
    for(i=1; i<TBL_FEA_COPP_TRAP_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_copp_trap_group_field_name_dump(p_fea_copp_trap_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_copp_trap_group_field_value_dump(p_fea_copp_trap_group, i, buf));
    }
    return str;
}

int32
tbl_fea_copp_trap_group_field_value_parser(char *str, int32 field_id, tbl_fea_copp_trap_group_t *p_fea_copp_trap_group)
{
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->key.copp_trap_reason_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->policer_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->trap_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->queue_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->green_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->green_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->yellow_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->yellow_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->red_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE, field_id))
    {
        int32 ret;
        p_fea_copp_trap_group->red_byte = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_table_parser(char** array, char* key_value,tbl_fea_copp_trap_group_t *p_fea_copp_trap_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_COPP_TRAP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_copp_trap_group_key_str2val(key_value, p_fea_copp_trap_group));

    for(i=1; i<TBL_FEA_COPP_TRAP_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_copp_trap_group_field_value_parser( array[j++], i, p_fea_copp_trap_group));
    }

    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_dump_one(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_COPP_TRAP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_KEY].name,
            p_fea_copp_trap_group->key.copp_trap_reason_id);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID].name,
            p_fea_copp_trap_group->policer_oid);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID].name,
            p_fea_copp_trap_group->trap_oid);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID].name,
            p_fea_copp_trap_group->queue_id);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET].name,
            p_fea_copp_trap_group->green_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE].name,
            p_fea_copp_trap_group->green_byte);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET].name,
            p_fea_copp_trap_group->yellow_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE].name,
            p_fea_copp_trap_group->yellow_byte);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET].name,
            p_fea_copp_trap_group->red_packet);
    }
    if (FLD_MATCH(TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE].name,
            p_fea_copp_trap_group->red_byte);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_copp_trap_group_master_t *p_master = _g_p_tbl_fea_copp_trap_group_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fea_copp_trap_group_t *p_db_fea_copp_trap_group = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->fea_copp_trap_group_list, p_db_fea_copp_trap_group, listnode, next)
    {
        rc |= fn(p_db_fea_copp_trap_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_copp_trap_group_master_t*
tbl_fea_copp_trap_group_get_master()
{
    return _g_p_tbl_fea_copp_trap_group_master;
}

tbl_fea_copp_trap_group_master_t*
tbl_fea_copp_trap_group_init_fea_copp_trap_group()
{
    _g_p_tbl_fea_copp_trap_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_copp_trap_group_master_t));
    _g_p_tbl_fea_copp_trap_group_master->fea_copp_trap_group_hash = ctclib_hash_create(_tbl_fea_copp_trap_group_hash_make, _tbl_fea_copp_trap_group_hash_cmp);
    _g_p_tbl_fea_copp_trap_group_master->fea_copp_trap_group_list = ctclib_slist_create(tbl_fea_copp_trap_group_cmp, NULL);
    return _g_p_tbl_fea_copp_trap_group_master;
}


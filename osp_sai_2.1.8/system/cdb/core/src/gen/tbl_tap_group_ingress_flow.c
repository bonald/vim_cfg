
#include "proto.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "cdb_data_cmp.h"

tbl_tap_group_ingress_flow_master_t *_g_p_tbl_tap_group_ingress_flow_master = NULL;

static uint32
_tbl_tap_group_ingress_flow_hash_make(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tap_group_ingress_flow->key;
    for (index = 0; index < sizeof(p_tap_group_ingress_flow->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_tap_group_ingress_flow_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow1 = (tbl_tap_group_ingress_flow_t*)p_arg1;
    tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow2 = (tbl_tap_group_ingress_flow_t*)p_arg2;

    if (0 == sal_memcmp(&p_tap_group_ingress_flow1->key, &p_tap_group_ingress_flow2->key, sizeof(tbl_tap_group_ingress_flow_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_tap_group_ingress_flow_add_tap_group_ingress_flow_sync(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, uint32 sync)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_tap_group_ingress_flow_get_tap_group_ingress_flow(&p_tap_group_ingress_flow->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tap_group_ingress_flow = XCALLOC(MEM_TBL_TAP_GROUP_INGRESS_FLOW, sizeof(tbl_tap_group_ingress_flow_t));
    if (NULL == p_db_tap_group_ingress_flow)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tap_group_ingress_flow, p_tap_group_ingress_flow, sizeof(tbl_tap_group_ingress_flow_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tap_group_ingress_hash, (void*)p_db_tap_group_ingress_flow, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TAP_GROUP_INGRESS_FLOW, p_db_tap_group_ingress_flow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TAP_GROUP_INGRESS_FLOW, p_db_tap_group_ingress_flow);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_del_tap_group_ingress_flow_sync(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key, uint32 sync)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group_ingress_flow = tbl_tap_group_ingress_flow_get_tap_group_ingress_flow(p_tap_group_ingress_flow_key);
    if (NULL == p_db_tap_group_ingress_flow)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TAP_GROUP_INGRESS_FLOW, p_db_tap_group_ingress_flow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TAP_GROUP_INGRESS_FLOW, p_db_tap_group_ingress_flow);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tap_group_ingress_hash, (void*)p_db_tap_group_ingress_flow);
    ctclib_slistnode_delete(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow);

    /* 4. free db entry */
    XFREE(MEM_TBL_TAP_GROUP_INGRESS_FLOW, p_db_tap_group_ingress_flow);

    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field_sync(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_tap_group_ingress_flow_field_id_t field_id, uint32 sync)
{
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group_ingress_flow = tbl_tap_group_ingress_flow_get_tap_group_ingress_flow(&p_tap_group_ingress_flow->key);
    if (NULL == p_db_tap_group_ingress_flow)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID:
        p_db_tap_group_ingress_flow->tap_grp_id = p_tap_group_ingress_flow->tap_grp_id;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX:
        p_db_tap_group_ingress_flow->if_idx = p_tap_group_ingress_flow->if_idx;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN:
        p_db_tap_group_ingress_flow->ing_untag_en = p_tap_group_ingress_flow->ing_untag_en;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN:
        p_db_tap_group_ingress_flow->ing_mark_vlan_en = p_tap_group_ingress_flow->ing_mark_vlan_en;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID:
        p_db_tap_group_ingress_flow->ing_mark_vlan_vid = p_tap_group_ingress_flow->ing_mark_vlan_vid;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID:
        p_db_tap_group_ingress_flow->ing_tap_group_member_oid = p_tap_group_ingress_flow->ing_tap_group_member_oid;
        break;

    case TBL_TAP_GROUP_INGRESS_FLOW_FLD_MAX:
        sal_memcpy(p_db_tap_group_ingress_flow, p_tap_group_ingress_flow, sizeof(tbl_tap_group_ingress_flow_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAP_GROUP_INGRESS_FLOW, field_id, p_db_tap_group_ingress_flow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAP_GROUP_INGRESS_FLOW, field_id, p_db_tap_group_ingress_flow);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_add_tap_group_ingress_flow(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow)
{
    return tbl_tap_group_ingress_flow_add_tap_group_ingress_flow_sync(p_tap_group_ingress_flow, TRUE);
}

int32
tbl_tap_group_ingress_flow_del_tap_group_ingress_flow(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key)
{
    return tbl_tap_group_ingress_flow_del_tap_group_ingress_flow_sync(p_tap_group_ingress_flow_key, TRUE);
}

int32
tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_tap_group_ingress_flow_field_id_t field_id)
{
    return tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field_sync(p_tap_group_ingress_flow, field_id, TRUE);
}

tbl_tap_group_ingress_flow_t*
tbl_tap_group_ingress_flow_get_tap_group_ingress_flow(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    tbl_tap_group_ingress_flow_t lkp;

    sal_memcpy(&lkp.key, p_tap_group_ingress_flow_key, sizeof(tbl_tap_group_ingress_flow_key_t));
    return ctclib_hash_lookup(p_master->tap_group_ingress_hash, &lkp);
}

char*
tbl_tap_group_ingress_flow_key_val2str(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_tap_group_ingress_flow_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_ingress_flow->key));
    return str;
}

int32
tbl_tap_group_ingress_flow_key_str2val(char *str, tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow)
{
    int32 ret = 0;

    ret = cdb_tap_group_ingress_flow_key_str2val(str, &p_tap_group_ingress_flow->key);
    return ret;
}

char*
tbl_tap_group_ingress_flow_key_name_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_INGRESS_FLOW);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_tap_group_ingress_flow_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_ingress_flow->key));
    return str;
}

char*
tbl_tap_group_ingress_flow_key_value_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char *str)
{

    cdb_tap_group_ingress_flow_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_group_ingress_flow->key);
    return str;
}

char*
tbl_tap_group_ingress_flow_field_name_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_INGRESS_FLOW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAP_GROUP_INGRESS_FLOW_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tap_group_ingress_flow_field_value_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_KEY, field_id))
    {
        cdb_tap_group_ingress_flow_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_group_ingress_flow->key);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_ingress_flow->tap_grp_id);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_ingress_flow->if_idx);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_ingress_flow->ing_untag_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_ingress_flow->ing_mark_vlan_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_ingress_flow->ing_mark_vlan_vid);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_tap_group_ingress_flow->ing_tap_group_member_oid);
    }
    return str;
}

char**
tbl_tap_group_ingress_flow_table_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tap_group_ingress_flow_key_name_dump(p_tap_group_ingress_flow, buf));
    for(i=1; i<TBL_TAP_GROUP_INGRESS_FLOW_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_ingress_flow_field_name_dump(p_tap_group_ingress_flow, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_ingress_flow_field_value_dump(p_tap_group_ingress_flow, i, buf));
    }
    return str;
}

int32
tbl_tap_group_ingress_flow_field_value_parser(char *str, int32 field_id, tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow)
{
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_KEY, field_id))
    {
        cdb_tap_group_ingress_flow_key_str2val(str, &p_tap_group_ingress_flow->key);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->tap_grp_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->if_idx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->ing_untag_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->ing_mark_vlan_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->ing_mark_vlan_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID, field_id))
    {
        int32 ret;
        p_tap_group_ingress_flow->ing_tap_group_member_oid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_table_parser(char** array, char* key_value,tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_INGRESS_FLOW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tap_group_ingress_flow_key_str2val(key_value, p_tap_group_ingress_flow));

    for(i=1; i<TBL_TAP_GROUP_INGRESS_FLOW_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tap_group_ingress_flow_field_value_parser( array[j++], i, p_tap_group_ingress_flow));
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_dump_one(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_INGRESS_FLOW);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_KEY].name, 
            cdb_tap_group_ingress_flow_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_ingress_flow->key));
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_TAP_GRP_ID].name,
            p_tap_group_ingress_flow->tap_grp_id);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_IF_IDX].name,
            p_tap_group_ingress_flow->if_idx);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_UNTAG_EN].name,
            p_tap_group_ingress_flow->ing_untag_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_EN].name,
            p_tap_group_ingress_flow->ing_mark_vlan_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_MARK_VLAN_VID].name,
            p_tap_group_ingress_flow->ing_mark_vlan_vid);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_TAP_GROUP_INGRESS_FLOW_FLD_ING_TAP_GROUP_MEMBER_OID].name,
            p_tap_group_ingress_flow->ing_tap_group_member_oid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tap_group_ingress_flow_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        rc |= fn(p_db_tap_group_ingress_flow, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tap_group_ingress_flow_master_t*
tbl_tap_group_ingress_flow_get_master()
{
    return _g_p_tbl_tap_group_ingress_flow_master;
}

tbl_tap_group_ingress_flow_master_t*
tbl_tap_group_ingress_flow_init_tap_group_ingress_flow()
{
    _g_p_tbl_tap_group_ingress_flow_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_tap_group_ingress_flow_master_t));
    _g_p_tbl_tap_group_ingress_flow_master->tap_group_ingress_hash = ctclib_hash_create(_tbl_tap_group_ingress_flow_hash_make, _tbl_tap_group_ingress_flow_hash_cmp);
    _g_p_tbl_tap_group_ingress_flow_master->tap_group_ingress_list = ctclib_slist_create(tbl_tap_group_ingress_flow_cmp, NULL);
    return _g_p_tbl_tap_group_ingress_flow_master;
}

int32
tbl_tap_group_ingress_flow_get_if_exist(const char* tap_grp, const char* if_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
tbl_tap_group_ingress_flow_get_group_if_exist(const char* tap_grp, const char* if_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE)
            && 0 == sal_memcmp(p_db_tap_group_ingress_flow->key.tap_group, tap_grp, (sizeof(char))*TAP_NAME_SIZE))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}

int32
tbl_tap_group_ingress_flow_get_if_flow_exist(const char* if_name, const char* flow_name)
{
    tbl_tap_group_ingress_flow_master_t *p_master = _g_p_tbl_tap_group_ingress_flow_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_ingress_flow_t *p_db_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_ingress_list, p_db_tap_group_ingress_flow, listnode, next)
    {
        if (0 == sal_memcmp(p_db_tap_group_ingress_flow->key.if_name, if_name, (sizeof(char))*IFNAME_SIZE)
            && 0 == sal_strcmp(p_db_tap_group_ingress_flow->key.flow_name, flow_name))
        {
            return PM_E_EXIST;
        }
    }

    return rc;
}






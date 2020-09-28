
#include "proto.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "cdb_data_cmp.h"

tbl_snmp_view_master_t *_g_p_tbl_snmp_view_master = NULL;

int32
tbl_snmp_view_add_snmp_view_sync(tbl_snmp_view_t *p_snmp_view, uint32 sync)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_view_get_snmp_view(&p_snmp_view->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_view = XCALLOC(MEM_TBL_SNMP_VIEW, sizeof(tbl_snmp_view_t));
    if (NULL == p_db_snmp_view)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_view, p_snmp_view, sizeof(tbl_snmp_view_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->snmp_view_list, p_db_snmp_view);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_VIEW, p_db_snmp_view);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_VIEW, p_db_snmp_view);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_view_del_snmp_view_sync(tbl_snmp_view_key_t *p_snmp_view_key, uint32 sync)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_view = tbl_snmp_view_get_snmp_view(p_snmp_view_key);
    if (NULL == p_db_snmp_view)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_VIEW, p_db_snmp_view);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_VIEW, p_db_snmp_view);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->snmp_view_list, p_db_snmp_view);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_VIEW, p_db_snmp_view);

    return PM_E_NONE;
}

int32
tbl_snmp_view_set_snmp_view_field_sync(tbl_snmp_view_t *p_snmp_view, tbl_snmp_view_field_id_t field_id, uint32 sync)
{
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_view = tbl_snmp_view_get_snmp_view(&p_snmp_view->key);
    if (NULL == p_db_snmp_view)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_VIEW_FLD_TYPE:
        p_db_snmp_view->type = p_snmp_view->type;
        break;

    case TBL_SNMP_VIEW_FLD_MASK:
        sal_memcpy(p_db_snmp_view->mask, p_snmp_view->mask, sizeof(p_snmp_view->mask));
        break;

    case TBL_SNMP_VIEW_FLD_ACCESS_CNT:
        p_db_snmp_view->access_cnt = p_snmp_view->access_cnt;
        break;

    case TBL_SNMP_VIEW_FLD_REFCNT:
        p_db_snmp_view->refcnt = p_snmp_view->refcnt;
        break;

    case TBL_SNMP_VIEW_FLD_MAX:
        sal_memcpy(p_db_snmp_view, p_snmp_view, sizeof(tbl_snmp_view_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_VIEW, field_id, p_db_snmp_view);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_VIEW, field_id, p_db_snmp_view);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_view_add_snmp_view(tbl_snmp_view_t *p_snmp_view)
{
    return tbl_snmp_view_add_snmp_view_sync(p_snmp_view, TRUE);
}

int32
tbl_snmp_view_del_snmp_view(tbl_snmp_view_key_t *p_snmp_view_key)
{
    return tbl_snmp_view_del_snmp_view_sync(p_snmp_view_key, TRUE);
}

int32
tbl_snmp_view_set_snmp_view_field(tbl_snmp_view_t *p_snmp_view, tbl_snmp_view_field_id_t field_id)
{
    return tbl_snmp_view_set_snmp_view_field_sync(p_snmp_view, field_id, TRUE);
}

tbl_snmp_view_t*
tbl_snmp_view_get_snmp_view(tbl_snmp_view_key_t *p_snmp_view_key)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    tbl_snmp_view_t lkp;

    sal_memcpy(&lkp.key, p_snmp_view_key, sizeof(tbl_snmp_view_key_t));
    return ctclib_slistdata_lookup(p_master->snmp_view_list, &lkp);
}

char*
tbl_snmp_view_key_val2str(tbl_snmp_view_t *p_snmp_view, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_snmp_view_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_view->key.view));
    return str;
}

int32
tbl_snmp_view_key_str2val(char *str, tbl_snmp_view_t *p_snmp_view)
{
    int32 ret = 0;

    ret = cdb_snmp_view_str2val(str, &p_snmp_view->key.view);
    return ret;
}

char*
tbl_snmp_view_key_name_dump(tbl_snmp_view_t *p_snmp_view, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_VIEW);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_snmp_view_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_view->key.view));
    return str;
}

char*
tbl_snmp_view_key_value_dump(tbl_snmp_view_t *p_snmp_view, char *str)
{

    cdb_snmp_view_val2str(str, MAX_CMD_STR_LEN, &p_snmp_view->key.view);
    return str;
}

char*
tbl_snmp_view_field_name_dump(tbl_snmp_view_t *p_snmp_view, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_VIEW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_VIEW_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_view_field_value_dump(tbl_snmp_view_t *p_snmp_view, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_KEY, field_id))
    {
        cdb_snmp_view_val2str(str, MAX_CMD_STR_LEN, &p_snmp_view->key.view);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(snmp_view_class_strs, SNMP_VIEW_CLASS_MAX, p_snmp_view->type));
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_MASK, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_view->mask);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_ACCESS_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_view->access_cnt);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_REFCNT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_view->refcnt);
    }
    return str;
}

char**
tbl_snmp_view_table_dump(tbl_snmp_view_t *p_snmp_view, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_view_key_name_dump(p_snmp_view, buf));
    for(i=1; i<TBL_SNMP_VIEW_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_view_field_name_dump(p_snmp_view, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_view_field_value_dump(p_snmp_view, i, buf));
    }
    return str;
}

int32
tbl_snmp_view_field_value_parser(char *str, int32 field_id, tbl_snmp_view_t *p_snmp_view)
{
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_KEY, field_id))
    {
        cdb_snmp_view_str2val(str, &p_snmp_view->key.view);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(snmp_view_class_strs, SNMP_VIEW_CLASS_MAX, str);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_MASK, field_id))
    {
        sal_strcpy(p_snmp_view->mask, str);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_ACCESS_CNT, field_id))
    {
        int32 ret;
        p_snmp_view->access_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_REFCNT, field_id))
    {
        int32 ret;
        p_snmp_view->refcnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_view_table_parser(char** array, char* key_value,tbl_snmp_view_t *p_snmp_view)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_VIEW);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_view_key_str2val(key_value, p_snmp_view));

    for(i=1; i<TBL_SNMP_VIEW_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_view_field_value_parser( array[j++], i, p_snmp_view));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_view_dump_one(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_VIEW);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_VIEW_FLD_KEY].name, 
            cdb_snmp_view_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_view->key.view));
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_VIEW_FLD_TYPE].name, 
            cdb_enum_val2str(snmp_view_class_strs, SNMP_VIEW_CLASS_MAX, p_snmp_view->type));
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_VIEW_FLD_MASK].name,
            p_snmp_view->mask);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_ACCESS_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_VIEW_FLD_ACCESS_CNT].name,
            p_snmp_view->access_cnt);
    }
    if (FLD_MATCH(TBL_SNMP_VIEW_FLD_REFCNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_VIEW_FLD_REFCNT].name,
            p_snmp_view->refcnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_view_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        rc |= fn(p_db_snmp_view, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_view_master_t*
tbl_snmp_view_get_master()
{
    return _g_p_tbl_snmp_view_master;
}

tbl_snmp_view_master_t*
tbl_snmp_view_init_snmp_view()
{
    _g_p_tbl_snmp_view_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_view_master_t));
    _g_p_tbl_snmp_view_master->snmp_view_list = ctclib_slist_create(tbl_snmp_view_cmp, NULL);
    return _g_p_tbl_snmp_view_master;
}

int32
tbl_snmp_view_num_get_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    int32 count = 0;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return 0;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            count++;
        }
    }
    return count;
}


int32
tbl_snmp_view_refcnt_add_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            p_db_snmp_view->refcnt++;
        }
    }
    return PM_E_NONE;
}

int32
tbl_snmp_view_refcnt_del_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            if (p_db_snmp_view->refcnt > 0)
            {
                p_db_snmp_view->refcnt--;
            }
        }
    }
    return PM_E_NONE;
}




#include "proto.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "cdb_data_cmp.h"

tbl_snmp_community_master_t *_g_p_tbl_snmp_community_master = NULL;

int32
tbl_snmp_community_add_snmp_community_sync(tbl_snmp_community_t *p_snmp_community, uint32 sync)
{
    tbl_snmp_community_master_t *p_master = _g_p_tbl_snmp_community_master;
    tbl_snmp_community_t *p_db_snmp_community = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_community_get_snmp_community(&p_snmp_community->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_community = XCALLOC(MEM_TBL_SNMP_COMMUNITY, sizeof(tbl_snmp_community_t));
    if (NULL == p_db_snmp_community)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_community, p_snmp_community, sizeof(tbl_snmp_community_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->snmp_community_list, p_db_snmp_community);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_COMMUNITY, p_db_snmp_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_COMMUNITY, p_db_snmp_community);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_community_del_snmp_community_sync(tbl_snmp_community_key_t *p_snmp_community_key, uint32 sync)
{
    tbl_snmp_community_master_t *p_master = _g_p_tbl_snmp_community_master;
    tbl_snmp_community_t *p_db_snmp_community = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_community = tbl_snmp_community_get_snmp_community(p_snmp_community_key);
    if (NULL == p_db_snmp_community)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_COMMUNITY, p_db_snmp_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_COMMUNITY, p_db_snmp_community);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->snmp_community_list, p_db_snmp_community);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_COMMUNITY, p_db_snmp_community);

    return PM_E_NONE;
}

int32
tbl_snmp_community_set_snmp_community_field_sync(tbl_snmp_community_t *p_snmp_community, tbl_snmp_community_field_id_t field_id, uint32 sync)
{
    tbl_snmp_community_t *p_db_snmp_community = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_community = tbl_snmp_community_get_snmp_community(&p_snmp_community->key);
    if (NULL == p_db_snmp_community)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_COMMUNITY_FLD_TYPE:
        p_db_snmp_community->type = p_snmp_community->type;
        break;

    case TBL_SNMP_COMMUNITY_FLD_VIEW:
        sal_memcpy(p_db_snmp_community->view, p_snmp_community->view, sizeof(p_snmp_community->view));
        break;

    case TBL_SNMP_COMMUNITY_FLD_SECIDX:
        p_db_snmp_community->secidx = p_snmp_community->secidx;
        break;

    case TBL_SNMP_COMMUNITY_FLD_MAX:
        sal_memcpy(p_db_snmp_community, p_snmp_community, sizeof(tbl_snmp_community_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_COMMUNITY, field_id, p_db_snmp_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_COMMUNITY, field_id, p_db_snmp_community);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_community_add_snmp_community(tbl_snmp_community_t *p_snmp_community)
{
    return tbl_snmp_community_add_snmp_community_sync(p_snmp_community, TRUE);
}

int32
tbl_snmp_community_del_snmp_community(tbl_snmp_community_key_t *p_snmp_community_key)
{
    return tbl_snmp_community_del_snmp_community_sync(p_snmp_community_key, TRUE);
}

int32
tbl_snmp_community_set_snmp_community_field(tbl_snmp_community_t *p_snmp_community, tbl_snmp_community_field_id_t field_id)
{
    return tbl_snmp_community_set_snmp_community_field_sync(p_snmp_community, field_id, TRUE);
}

tbl_snmp_community_t*
tbl_snmp_community_get_snmp_community(tbl_snmp_community_key_t *p_snmp_community_key)
{
    tbl_snmp_community_master_t *p_master = _g_p_tbl_snmp_community_master;
    tbl_snmp_community_t lkp;

    sal_memcpy(&lkp.key, p_snmp_community_key, sizeof(tbl_snmp_community_key_t));
    return ctclib_slistdata_lookup(p_master->snmp_community_list, &lkp);
}

char*
tbl_snmp_community_key_val2str(tbl_snmp_community_t *p_snmp_community, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_snmp_community->key.name);
    return str;
}

int32
tbl_snmp_community_key_str2val(char *str, tbl_snmp_community_t *p_snmp_community)
{
    int32 ret = 0;

    sal_strcpy(p_snmp_community->key.name, str);
    return ret;
}

char*
tbl_snmp_community_key_name_dump(tbl_snmp_community_t *p_snmp_community, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_snmp_community->key.name);
    return str;
}

char*
tbl_snmp_community_key_value_dump(tbl_snmp_community_t *p_snmp_community, char *str)
{
    sal_sprintf(str, "%s", p_snmp_community->key.name);
    return str;
}

char*
tbl_snmp_community_field_name_dump(tbl_snmp_community_t *p_snmp_community, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_COMMUNITY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_community_field_value_dump(tbl_snmp_community_t *p_snmp_community, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_community->key.name);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(snmp_community_strs, SNMP_COMMUNITY_MAX, p_snmp_community->type));
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_VIEW, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_community->view);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_SECIDX, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_community->secidx);
    }
    return str;
}

char**
tbl_snmp_community_table_dump(tbl_snmp_community_t *p_snmp_community, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_community_key_name_dump(p_snmp_community, buf));
    for(i=1; i<TBL_SNMP_COMMUNITY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_community_field_name_dump(p_snmp_community, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_community_field_value_dump(p_snmp_community, i, buf));
    }
    return str;
}

int32
tbl_snmp_community_field_value_parser(char *str, int32 field_id, tbl_snmp_community_t *p_snmp_community)
{
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_KEY, field_id))
    {
        sal_strcpy(p_snmp_community->key.name, str);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(snmp_community_strs, SNMP_COMMUNITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_VIEW, field_id))
    {
        sal_strcpy(p_snmp_community->view, str);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_SECIDX, field_id))
    {
        int32 ret;
        p_snmp_community->secidx = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_community_table_parser(char** array, char* key_value,tbl_snmp_community_t *p_snmp_community)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_community_key_str2val(key_value, p_snmp_community));

    for(i=1; i<TBL_SNMP_COMMUNITY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_community_field_value_parser( array[j++], i, p_snmp_community));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_community_dump_one(tbl_snmp_community_t *p_snmp_community, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_COMMUNITY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_COMMUNITY_FLD_KEY].name,
            p_snmp_community->key.name);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_COMMUNITY_FLD_TYPE].name, 
            cdb_enum_val2str(snmp_community_strs, SNMP_COMMUNITY_MAX, p_snmp_community->type));
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_VIEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_COMMUNITY_FLD_VIEW].name,
            p_snmp_community->view);
    }
    if (FLD_MATCH(TBL_SNMP_COMMUNITY_FLD_SECIDX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_COMMUNITY_FLD_SECIDX].name,
            p_snmp_community->secidx);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_community_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_community_master_t *p_master = _g_p_tbl_snmp_community_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_community_t *p_db_snmp_community = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_community_list, p_db_snmp_community, listnode, next)
    {
        rc |= fn(p_db_snmp_community, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_community_master_t*
tbl_snmp_community_get_master()
{
    return _g_p_tbl_snmp_community_master;
}

tbl_snmp_community_master_t*
tbl_snmp_community_init_snmp_community()
{
    _g_p_tbl_snmp_community_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_community_master_t));
    _g_p_tbl_snmp_community_master->snmp_community_list = ctclib_slist_create(tbl_snmp_community_cmp, NULL);
    return _g_p_tbl_snmp_community_master;
}


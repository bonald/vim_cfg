
#include "proto.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_ace.h"
#include "cdb_data_cmp.h"

tbl_ntp_ace_master_t *_g_p_tbl_ntp_ace_master = NULL;

int32
tbl_ntp_ace_add_ntp_ace_sync(tbl_ntp_ace_t *p_ntp_ace, uint32 sync)
{
    tbl_ntp_ace_master_t *p_master = _g_p_tbl_ntp_ace_master;
    tbl_ntp_ace_t *p_db_ntp_ace = NULL;

    /* 1. lookup entry exist */
    if (tbl_ntp_ace_get_ntp_ace(&p_ntp_ace->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ntp_ace = XCALLOC(MEM_TBL_NTP_ACE, sizeof(tbl_ntp_ace_t));
    if (NULL == p_db_ntp_ace)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ntp_ace, p_ntp_ace, sizeof(tbl_ntp_ace_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->ntp_ace_list, p_db_ntp_ace);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NTP_ACE, p_db_ntp_ace);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NTP_ACE, p_db_ntp_ace);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_ace_del_ntp_ace_sync(tbl_ntp_ace_key_t *p_ntp_ace_key, uint32 sync)
{
    tbl_ntp_ace_master_t *p_master = _g_p_tbl_ntp_ace_master;
    tbl_ntp_ace_t *p_db_ntp_ace = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_ace = tbl_ntp_ace_get_ntp_ace(p_ntp_ace_key);
    if (NULL == p_db_ntp_ace)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NTP_ACE, p_db_ntp_ace);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NTP_ACE, p_db_ntp_ace);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->ntp_ace_list, p_db_ntp_ace);

    /* 4. free db entry */
    XFREE(MEM_TBL_NTP_ACE, p_db_ntp_ace);

    return PM_E_NONE;
}

int32
tbl_ntp_ace_set_ntp_ace_field_sync(tbl_ntp_ace_t *p_ntp_ace, tbl_ntp_ace_field_id_t field_id, uint32 sync)
{
    tbl_ntp_ace_t *p_db_ntp_ace = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_ace = tbl_ntp_ace_get_ntp_ace(&p_ntp_ace->key);
    if (NULL == p_db_ntp_ace)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_ACE_FLD_OPTION:
        p_db_ntp_ace->option = p_ntp_ace->option;
        break;

    case TBL_NTP_ACE_FLD_FAMILY:
        p_db_ntp_ace->family = p_ntp_ace->family;
        break;

    case TBL_NTP_ACE_FLD_MASKLEN:
        p_db_ntp_ace->masklen = p_ntp_ace->masklen;
        break;

    case TBL_NTP_ACE_FLD_MAX:
        sal_memcpy(p_db_ntp_ace, p_ntp_ace, sizeof(tbl_ntp_ace_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_ACE, field_id, p_db_ntp_ace);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_ACE, field_id, p_db_ntp_ace);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_ace_add_ntp_ace(tbl_ntp_ace_t *p_ntp_ace)
{
    return tbl_ntp_ace_add_ntp_ace_sync(p_ntp_ace, TRUE);
}

int32
tbl_ntp_ace_del_ntp_ace(tbl_ntp_ace_key_t *p_ntp_ace_key)
{
    return tbl_ntp_ace_del_ntp_ace_sync(p_ntp_ace_key, TRUE);
}

int32
tbl_ntp_ace_set_ntp_ace_field(tbl_ntp_ace_t *p_ntp_ace, tbl_ntp_ace_field_id_t field_id)
{
    return tbl_ntp_ace_set_ntp_ace_field_sync(p_ntp_ace, field_id, TRUE);
}

tbl_ntp_ace_t*
tbl_ntp_ace_get_ntp_ace(tbl_ntp_ace_key_t *p_ntp_ace_key)
{
    tbl_ntp_ace_master_t *p_master = _g_p_tbl_ntp_ace_master;
    tbl_ntp_ace_t lkp;

    sal_memcpy(&lkp.key, p_ntp_ace_key, sizeof(tbl_ntp_ace_key_t));
    return ctclib_slistdata_lookup(p_master->ntp_ace_list, &lkp);
}

char*
tbl_ntp_ace_key_val2str(tbl_ntp_ace_t *p_ntp_ace, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_ntp_ace->key.host);
    return str;
}

int32
tbl_ntp_ace_key_str2val(char *str, tbl_ntp_ace_t *p_ntp_ace)
{
    int32 ret = 0;

    sal_strcpy(p_ntp_ace->key.host, str);
    return ret;
}

char*
tbl_ntp_ace_key_name_dump(tbl_ntp_ace_t *p_ntp_ace, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_ACE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_ntp_ace->key.host);
    return str;
}

char*
tbl_ntp_ace_key_value_dump(tbl_ntp_ace_t *p_ntp_ace, char *str)
{
    sal_sprintf(str, "%s", p_ntp_ace->key.host);
    return str;
}

char*
tbl_ntp_ace_field_name_dump(tbl_ntp_ace_t *p_ntp_ace, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_ACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_ACE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_ace_field_value_dump(tbl_ntp_ace_t *p_ntp_ace, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_ACE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_ace->key.host);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_OPTION, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_ace->option);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_FAMILY, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_ace->family);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_MASKLEN, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_ace->masklen);
    }
    return str;
}

char**
tbl_ntp_ace_table_dump(tbl_ntp_ace_t *p_ntp_ace, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_ace_key_name_dump(p_ntp_ace, buf));
    for(i=1; i<TBL_NTP_ACE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_ace_field_name_dump(p_ntp_ace, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_ace_field_value_dump(p_ntp_ace, i, buf));
    }
    return str;
}

int32
tbl_ntp_ace_field_value_parser(char *str, int32 field_id, tbl_ntp_ace_t *p_ntp_ace)
{
    if (FLD_MATCH(TBL_NTP_ACE_FLD_KEY, field_id))
    {
        sal_strcpy(p_ntp_ace->key.host, str);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_OPTION, field_id))
    {
        int32 ret;
        p_ntp_ace->option = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_FAMILY, field_id))
    {
        int32 ret;
        p_ntp_ace->family = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_MASKLEN, field_id))
    {
        int32 ret;
        p_ntp_ace->masklen = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ntp_ace_table_parser(char** array, char* key_value,tbl_ntp_ace_t *p_ntp_ace)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_ACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_ace_key_str2val(key_value, p_ntp_ace));

    for(i=1; i<TBL_NTP_ACE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_ace_field_value_parser( array[j++], i, p_ntp_ace));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_ace_dump_one(tbl_ntp_ace_t *p_ntp_ace, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_ACE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_ACE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_ACE_FLD_KEY].name,
            p_ntp_ace->key.host);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_OPTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_ACE_FLD_OPTION].name,
            p_ntp_ace->option);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_FAMILY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_ACE_FLD_FAMILY].name,
            p_ntp_ace->family);
    }
    if (FLD_MATCH(TBL_NTP_ACE_FLD_MASKLEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_ACE_FLD_MASKLEN].name,
            p_ntp_ace->masklen);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_ace_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_ace_master_t *p_master = _g_p_tbl_ntp_ace_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ntp_ace_t *p_db_ntp_ace = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ntp_ace_list, p_db_ntp_ace, listnode, next)
    {
        rc |= fn(p_db_ntp_ace, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_ace_master_t*
tbl_ntp_ace_get_master()
{
    return _g_p_tbl_ntp_ace_master;
}

tbl_ntp_ace_master_t*
tbl_ntp_ace_init_ntp_ace()
{
    _g_p_tbl_ntp_ace_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ntp_ace_master_t));
    _g_p_tbl_ntp_ace_master->ntp_ace_list = ctclib_slist_create(tbl_ntp_ace_cmp, NULL);
    return _g_p_tbl_ntp_ace_master;
}


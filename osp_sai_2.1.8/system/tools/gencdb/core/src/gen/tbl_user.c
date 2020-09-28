
#include "proto.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "cdb_data_cmp.h"

tbl_user_master_t *_g_p_tbl_user_master = NULL;

int32
tbl_user_add_user_sync(tbl_user_t *p_user, uint32 sync)
{
    tbl_user_master_t *p_master = _g_p_tbl_user_master;
    tbl_user_t *p_db_user = NULL;

    /* 1. lookup entry exist */
    if (tbl_user_get_user(&p_user->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_user = XCALLOC(MEM_TBL_USER, sizeof(tbl_user_t));
    if (NULL == p_db_user)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_user, p_user, sizeof(tbl_user_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->user_list, p_db_user);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_USER, p_db_user);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_USER, p_db_user);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_user_del_user_sync(tbl_user_key_t *p_user_key, uint32 sync)
{
    tbl_user_master_t *p_master = _g_p_tbl_user_master;
    tbl_user_t *p_db_user = NULL;

    /* 1. lookup entry exist */
    p_db_user = tbl_user_get_user(p_user_key);
    if (NULL == p_db_user)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_USER, p_db_user);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_USER, p_db_user);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->user_list, p_db_user);

    /* 4. free db entry */
    XFREE(MEM_TBL_USER, p_db_user);

    return PM_E_NONE;
}

int32
tbl_user_set_user_field_sync(tbl_user_t *p_user, tbl_user_field_id_t field_id, uint32 sync)
{
    tbl_user_t *p_db_user = NULL;

    /* 1. lookup entry exist */
    p_db_user = tbl_user_get_user(&p_user->key);
    if (NULL == p_db_user)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_USER_FLD_UID:
        p_db_user->uid = p_user->uid;
        break;

    case TBL_USER_FLD_PRIVILEGE:
        p_db_user->privilege = p_user->privilege;
        break;

    case TBL_USER_FLD_ENC_PASSWD:
        sal_memcpy(p_db_user->enc_passwd, p_user->enc_passwd, sizeof(p_user->enc_passwd));
        break;

    case TBL_USER_FLD_PASSWD:
        sal_memcpy(p_db_user->passwd, p_user->passwd, sizeof(p_user->passwd));
        break;

    case TBL_USER_FLD_RSAKEY:
        sal_memcpy(p_db_user->rsakey, p_user->rsakey, sizeof(p_user->rsakey));
        break;

    case TBL_USER_FLD_SERVICE_TYPE:
        p_db_user->service_type = p_user->service_type;
        break;

    case TBL_USER_FLD_MAX:
        sal_memcpy(p_db_user, p_user, sizeof(tbl_user_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_USER, field_id, p_db_user);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_USER, field_id, p_db_user);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_user_add_user(tbl_user_t *p_user)
{
    return tbl_user_add_user_sync(p_user, TRUE);
}

int32
tbl_user_del_user(tbl_user_key_t *p_user_key)
{
    return tbl_user_del_user_sync(p_user_key, TRUE);
}

int32
tbl_user_set_user_field(tbl_user_t *p_user, tbl_user_field_id_t field_id)
{
    return tbl_user_set_user_field_sync(p_user, field_id, TRUE);
}

tbl_user_t*
tbl_user_get_user(tbl_user_key_t *p_user_key)
{
    tbl_user_master_t *p_master = _g_p_tbl_user_master;
    tbl_user_t lkp;

    sal_memcpy(&lkp.key, p_user_key, sizeof(tbl_user_key_t));
    return ctclib_slistdata_lookup(p_master->user_list, &lkp);
}

char*
tbl_user_key_val2str(tbl_user_t *p_user, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_user->key.name);
    return str;
}

int32
tbl_user_key_str2val(char *str, tbl_user_t *p_user)
{
    int32 ret = 0;

    sal_strcpy(p_user->key.name, str);
    return ret;
}

char*
tbl_user_key_name_dump(tbl_user_t *p_user, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_USER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_user->key.name);
    return str;
}

char*
tbl_user_key_value_dump(tbl_user_t *p_user, char *str)
{
    sal_sprintf(str, "%s", p_user->key.name);
    return str;
}

char*
tbl_user_field_name_dump(tbl_user_t *p_user, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_USER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_USER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_user_field_value_dump(tbl_user_t *p_user, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_USER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_user->key.name);
    }
    if (FLD_MATCH(TBL_USER_FLD_UID, field_id))
    {
        sal_sprintf(str, "%u", p_user->uid);
    }
    if (FLD_MATCH(TBL_USER_FLD_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_user->privilege);
    }
    if (FLD_MATCH(TBL_USER_FLD_ENC_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_user->enc_passwd);
    }
    if (FLD_MATCH(TBL_USER_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_user->passwd);
    }
    if (FLD_MATCH(TBL_USER_FLD_RSAKEY, field_id))
    {
        sal_sprintf(str, "%s", p_user->rsakey);
    }
    if (FLD_MATCH(TBL_USER_FLD_SERVICE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_user->service_type);
    }
    return str;
}

char**
tbl_user_table_dump(tbl_user_t *p_user, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_user_key_name_dump(p_user, buf));
    for(i=1; i<TBL_USER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_user_field_name_dump(p_user, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_user_field_value_dump(p_user, i, buf));
    }
    return str;
}

int32
tbl_user_field_value_parser(char *str, int32 field_id, tbl_user_t *p_user)
{
    if (FLD_MATCH(TBL_USER_FLD_KEY, field_id))
    {
        sal_strcpy(p_user->key.name, str);
    }
    if (FLD_MATCH(TBL_USER_FLD_UID, field_id))
    {
        int32 ret;
        p_user->uid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_USER_FLD_PRIVILEGE, field_id))
    {
        int32 ret;
        p_user->privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_USER_FLD_ENC_PASSWD, field_id))
    {
        sal_strcpy(p_user->enc_passwd, str);
    }
    if (FLD_MATCH(TBL_USER_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_user->passwd, str);
    }
    if (FLD_MATCH(TBL_USER_FLD_RSAKEY, field_id))
    {
        sal_strcpy(p_user->rsakey, str);
    }
    if (FLD_MATCH(TBL_USER_FLD_SERVICE_TYPE, field_id))
    {
        int32 ret;
        p_user->service_type = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_user_table_parser(char** array, char* key_value,tbl_user_t *p_user)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_USER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_user_key_str2val(key_value, p_user));

    for(i=1; i<TBL_USER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_user_field_value_parser( array[j++], i, p_user));
    }

    return PM_E_NONE;
}

int32
tbl_user_dump_one(tbl_user_t *p_user, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_USER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_USER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_USER_FLD_KEY].name,
            p_user->key.name);
    }
    if (FLD_MATCH(TBL_USER_FLD_UID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_USER_FLD_UID].name,
            p_user->uid);
    }
    if (FLD_MATCH(TBL_USER_FLD_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_USER_FLD_PRIVILEGE].name,
            p_user->privilege);
    }
    if (FLD_MATCH(TBL_USER_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_USER_FLD_ENC_PASSWD].name,
            p_user->enc_passwd);
    }
    if (FLD_MATCH(TBL_USER_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_USER_FLD_PASSWD].name,
            p_user->passwd);
    }
    if (FLD_MATCH(TBL_USER_FLD_RSAKEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_USER_FLD_RSAKEY].name,
            p_user->rsakey);
    }
    if (FLD_MATCH(TBL_USER_FLD_SERVICE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_USER_FLD_SERVICE_TYPE].name,
            p_user->service_type);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_user_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_user_master_t *p_master = _g_p_tbl_user_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_user_t *p_db_user = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->user_list, p_db_user, listnode, next)
    {
        rc |= fn(p_db_user, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_user_master_t*
tbl_user_get_master()
{
    return _g_p_tbl_user_master;
}

tbl_user_master_t*
tbl_user_init_user()
{
    _g_p_tbl_user_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_user_master_t));
    _g_p_tbl_user_master->user_list = ctclib_slist_create(tbl_user_cmp, NULL);
    return _g_p_tbl_user_master;
}


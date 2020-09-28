
#include "proto.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "cdb_data_cmp.h"

tbl_auth_session_master_t *_g_p_tbl_auth_session_master = NULL;

static uint32
_tbl_auth_session_hash_make(tbl_auth_session_t *p_auth_session)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_auth_session->key;
    for (index = 0; index < sizeof(p_auth_session->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_auth_session_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_auth_session_t *p_auth_session1 = (tbl_auth_session_t*)p_arg1;
    tbl_auth_session_t *p_auth_session2 = (tbl_auth_session_t*)p_arg2;

    if (0 == sal_memcmp(&p_auth_session1->key, &p_auth_session2->key, sizeof(tbl_auth_session_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_auth_session_add_auth_session_sync(tbl_auth_session_t *p_auth_session, uint32 sync)
{
    tbl_auth_session_master_t *p_master = _g_p_tbl_auth_session_master;
    tbl_auth_session_t *p_db_auth_session = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_auth_session_get_auth_session(&p_auth_session->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_auth_session = XCALLOC(MEM_TBL_AUTH_SESSION, sizeof(tbl_auth_session_t));
    if (NULL == p_db_auth_session)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_auth_session, p_auth_session, sizeof(tbl_auth_session_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->auth_sessions_hash, (void*)p_db_auth_session, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->auth_sessions, p_db_auth_session);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_AUTH_SESSION, p_db_auth_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_AUTH_SESSION, p_db_auth_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_auth_session_del_auth_session_sync(tbl_auth_session_key_t *p_auth_session_key, uint32 sync)
{
    tbl_auth_session_master_t *p_master = _g_p_tbl_auth_session_master;
    tbl_auth_session_t *p_db_auth_session = NULL;

    /* 1. lookup entry exist */
    p_db_auth_session = tbl_auth_session_get_auth_session(p_auth_session_key);
    if (NULL == p_db_auth_session)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_AUTH_SESSION, p_db_auth_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_AUTH_SESSION, p_db_auth_session);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->auth_sessions_hash, (void*)p_db_auth_session);
    ctclib_slistnode_delete(p_master->auth_sessions, p_db_auth_session);

    /* 4. free db entry */
    XFREE(MEM_TBL_AUTH_SESSION, p_db_auth_session);

    return PM_E_NONE;
}

int32
tbl_auth_session_set_auth_session_field_sync(tbl_auth_session_t *p_auth_session, tbl_auth_session_field_id_t field_id, uint32 sync)
{
    tbl_auth_session_t *p_db_auth_session = NULL;

    /* 1. lookup entry exist */
    p_db_auth_session = tbl_auth_session_get_auth_session(&p_auth_session->key);
    if (NULL == p_db_auth_session)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTH_SESSION_FLD_USER:
        sal_memcpy(&p_db_auth_session->user, &p_auth_session->user, sizeof(p_auth_session->user));
        break;

    case TBL_AUTH_SESSION_FLD_PASSWD:
        sal_memcpy(p_db_auth_session->passwd, p_auth_session->passwd, sizeof(p_auth_session->passwd));
        break;

    case TBL_AUTH_SESSION_FLD_STATUS:
        p_db_auth_session->status = p_auth_session->status;
        break;

    case TBL_AUTH_SESSION_FLD_AUTHENTICATOR:
        sal_memcpy(p_db_auth_session->authenticator, p_auth_session->authenticator, sizeof(p_auth_session->authenticator));
        break;

    case TBL_AUTH_SESSION_FLD_OAUTHENTICATOR:
        sal_memcpy(p_db_auth_session->oauthenticator, p_auth_session->oauthenticator, sizeof(p_auth_session->oauthenticator));
        break;

    case TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX:
        p_db_auth_session->dot1x_mac_ifindex = p_auth_session->dot1x_mac_ifindex;
        break;

    case TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR:
        sal_memcpy(p_db_auth_session->dot1x_mac_addr, p_auth_session->dot1x_mac_addr, sizeof(p_auth_session->dot1x_mac_addr));
        break;

    case TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS:
        p_db_auth_session->is_auth_bypass = p_auth_session->is_auth_bypass;
        break;

    case TBL_AUTH_SESSION_FLD_AUTHOR_START:
        p_db_auth_session->author_start = p_auth_session->author_start;
        break;

    case TBL_AUTH_SESSION_FLD_ACCT_START:
        p_db_auth_session->acct_start = p_auth_session->acct_start;
        break;

    case TBL_AUTH_SESSION_FLD_ACCT_STOP:
        p_db_auth_session->acct_stop = p_auth_session->acct_stop;
        break;

    case TBL_AUTH_SESSION_FLD_ACCTCMD_START:
        p_db_auth_session->acctcmd_start = p_auth_session->acctcmd_start;
        break;

    case TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP:
        p_db_auth_session->next_acct_stop = p_auth_session->next_acct_stop;
        break;

    case TBL_AUTH_SESSION_FLD_MAX:
        sal_memcpy(p_db_auth_session, p_auth_session, sizeof(tbl_auth_session_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTH_SESSION, field_id, p_db_auth_session);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTH_SESSION, field_id, p_db_auth_session);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_auth_session_add_auth_session(tbl_auth_session_t *p_auth_session)
{
    return tbl_auth_session_add_auth_session_sync(p_auth_session, TRUE);
}

int32
tbl_auth_session_del_auth_session(tbl_auth_session_key_t *p_auth_session_key)
{
    return tbl_auth_session_del_auth_session_sync(p_auth_session_key, TRUE);
}

int32
tbl_auth_session_set_auth_session_field(tbl_auth_session_t *p_auth_session, tbl_auth_session_field_id_t field_id)
{
    return tbl_auth_session_set_auth_session_field_sync(p_auth_session, field_id, TRUE);
}

tbl_auth_session_t*
tbl_auth_session_get_auth_session(tbl_auth_session_key_t *p_auth_session_key)
{
    tbl_auth_session_master_t *p_master = _g_p_tbl_auth_session_master;
    tbl_auth_session_t lkp;

    sal_memcpy(&lkp.key, p_auth_session_key, sizeof(tbl_auth_session_key_t));
    return ctclib_hash_lookup(p_master->auth_sessions_hash, &lkp);
}

char*
tbl_auth_session_key_val2str(tbl_auth_session_t *p_auth_session, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_auth_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_session->key));
    return str;
}

int32
tbl_auth_session_key_str2val(char *str, tbl_auth_session_t *p_auth_session)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_auth_session_key_str2val(&p_auth_session->key, str, err);
    return ret;
}

char*
tbl_auth_session_key_name_dump(tbl_auth_session_t *p_auth_session, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SESSION);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_auth_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_session->key));
    return str;
}

char*
tbl_auth_session_key_value_dump(tbl_auth_session_t *p_auth_session, char *str)
{

    cdb_auth_session_key_val2str(str, MAX_CMD_STR_LEN, &p_auth_session->key);
    return str;
}

char*
tbl_auth_session_field_name_dump(tbl_auth_session_t *p_auth_session, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTH_SESSION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_auth_session_field_value_dump(tbl_auth_session_t *p_auth_session, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_KEY, field_id))
    {
        cdb_auth_session_key_val2str(str, MAX_CMD_STR_LEN, &p_auth_session->key);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_USER, field_id))
    {
        cdb_user_val2str(str, MAX_CMD_STR_LEN, &p_auth_session->user);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_auth_session->passwd);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(auth_session_status_strs, AUTH_SESSION_STATUS_MAX, p_auth_session->status));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHENTICATOR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_auth_session->authenticator, sizeof(p_auth_session->authenticator));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_OAUTHENTICATOR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_auth_session->oauthenticator, sizeof(p_auth_session->oauthenticator));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->dot1x_mac_ifindex);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_auth_session->dot1x_mac_addr);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->is_auth_bypass);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHOR_START, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->author_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_START, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->acct_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_STOP, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->acct_stop);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCTCMD_START, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->acctcmd_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP, field_id))
    {
        sal_sprintf(str, "%u", p_auth_session->next_acct_stop);
    }
    return str;
}

char**
tbl_auth_session_table_dump(tbl_auth_session_t *p_auth_session, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_auth_session_key_name_dump(p_auth_session, buf));
    for(i=1; i<TBL_AUTH_SESSION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_auth_session_field_name_dump(p_auth_session, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_auth_session_field_value_dump(p_auth_session, i, buf));
    }
    return str;
}

int32
tbl_auth_session_field_value_parser(char *str, int32 field_id, tbl_auth_session_t *p_auth_session)
{
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_auth_session_key_str2val(&p_auth_session->key, str, err);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_USER, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_user_str2val(&p_auth_session->user, str, err);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_auth_session->passwd, str);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_STATUS, field_id))
    {
        cdb_enum_str2val(auth_session_status_strs, AUTH_SESSION_STATUS_MAX, str);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHENTICATOR, field_id))
    {
        cdb_uint8_array_str2val(str, p_auth_session->authenticator);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_OAUTHENTICATOR, field_id))
    {
        cdb_uint8_array_str2val(str, p_auth_session->oauthenticator);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX, field_id))
    {
        int32 ret;
        p_auth_session->dot1x_mac_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_auth_session->dot1x_mac_addr, str);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS, field_id))
    {
        int32 ret;
        p_auth_session->is_auth_bypass = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHOR_START, field_id))
    {
        int32 ret;
        p_auth_session->author_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_START, field_id))
    {
        int32 ret;
        p_auth_session->acct_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_STOP, field_id))
    {
        int32 ret;
        p_auth_session->acct_stop = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCTCMD_START, field_id))
    {
        int32 ret;
        p_auth_session->acctcmd_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP, field_id))
    {
        int32 ret;
        p_auth_session->next_acct_stop = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_auth_session_table_parser(char** array, char* key_value,tbl_auth_session_t *p_auth_session)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SESSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_auth_session_key_str2val(key_value, p_auth_session));

    for(i=1; i<TBL_AUTH_SESSION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_auth_session_field_value_parser( array[j++], i, p_auth_session));
    }

    return PM_E_NONE;
}

int32
tbl_auth_session_dump_one(tbl_auth_session_t *p_auth_session, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SESSION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_KEY].name, 
            cdb_auth_session_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_session->key));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_USER].name, 
            cdb_user_val2str(buf, MAX_CMD_STR_LEN, &p_auth_session->user));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_PASSWD].name,
            p_auth_session->passwd);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_STATUS].name, 
            cdb_enum_val2str(auth_session_status_strs, AUTH_SESSION_STATUS_MAX, p_auth_session->status));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHENTICATOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_AUTHENTICATOR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_auth_session->authenticator, sizeof(p_auth_session->authenticator)));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_OAUTHENTICATOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_OAUTHENTICATOR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_auth_session->oauthenticator, sizeof(p_auth_session->oauthenticator)));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_DOT1X_MAC_IFINDEX].name,
            p_auth_session->dot1x_mac_ifindex);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SESSION_FLD_DOT1X_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_auth_session->dot1x_mac_addr));
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_IS_AUTH_BYPASS].name,
            p_auth_session->is_auth_bypass);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_AUTHOR_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_AUTHOR_START].name,
            p_auth_session->author_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_ACCT_START].name,
            p_auth_session->acct_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCT_STOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_ACCT_STOP].name,
            p_auth_session->acct_stop);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_ACCTCMD_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_ACCTCMD_START].name,
            p_auth_session->acctcmd_start);
    }
    if (FLD_MATCH(TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SESSION_FLD_NEXT_ACCT_STOP].name,
            p_auth_session->next_acct_stop);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_auth_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_auth_session_master_t *p_master = _g_p_tbl_auth_session_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_auth_session_t *p_db_auth_session = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->auth_sessions, p_db_auth_session, listnode, next)
    {
        rc |= fn(p_db_auth_session, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_auth_session_master_t*
tbl_auth_session_get_master()
{
    return _g_p_tbl_auth_session_master;
}

tbl_auth_session_master_t*
tbl_auth_session_init_auth_session()
{
    _g_p_tbl_auth_session_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_auth_session_master_t));
    _g_p_tbl_auth_session_master->auth_sessions_hash = ctclib_hash_create(_tbl_auth_session_hash_make, _tbl_auth_session_hash_cmp);
    _g_p_tbl_auth_session_master->auth_sessions = ctclib_slist_create(tbl_auth_session_cmp, NULL);
    return _g_p_tbl_auth_session_master;
}


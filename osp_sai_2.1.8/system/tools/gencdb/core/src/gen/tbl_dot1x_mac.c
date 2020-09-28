
#include "proto.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "cdb_data_cmp.h"

tbl_dot1x_mac_master_t *_g_p_tbl_dot1x_mac_master = NULL;

static uint32
_tbl_dot1x_mac_hash_make(tbl_dot1x_mac_t *p_dot1x_mac)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_dot1x_mac->key;
    for (index = 0; index < sizeof(p_dot1x_mac->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_dot1x_mac_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_dot1x_mac_t *p_dot1x_mac1 = (tbl_dot1x_mac_t*)p_arg1;
    tbl_dot1x_mac_t *p_dot1x_mac2 = (tbl_dot1x_mac_t*)p_arg2;

    if (0 == sal_memcmp(&p_dot1x_mac1->key, &p_dot1x_mac2->key, sizeof(tbl_dot1x_mac_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_dot1x_mac_add_dot1x_mac_sync(tbl_dot1x_mac_t *p_dot1x_mac, uint32 sync)
{
    tbl_dot1x_mac_master_t *p_master = _g_p_tbl_dot1x_mac_master;
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dot1x_mac = XCALLOC(MEM_TBL_DOT1X_MAC, sizeof(tbl_dot1x_mac_t));
    if (NULL == p_db_dot1x_mac)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dot1x_mac, p_dot1x_mac, sizeof(tbl_dot1x_mac_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->dot1x_mac_hash, (void*)p_db_dot1x_mac, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->dot1x_mac_list, p_db_dot1x_mac);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DOT1X_MAC, p_db_dot1x_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DOT1X_MAC, p_db_dot1x_mac);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_mac_del_dot1x_mac_sync(tbl_dot1x_mac_key_t *p_dot1x_mac_key, uint32 sync)
{
    tbl_dot1x_mac_master_t *p_master = _g_p_tbl_dot1x_mac_master;
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(p_dot1x_mac_key);
    if (NULL == p_db_dot1x_mac)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DOT1X_MAC, p_db_dot1x_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DOT1X_MAC, p_db_dot1x_mac);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->dot1x_mac_hash, (void*)p_db_dot1x_mac);
    ctclib_slistnode_delete(p_master->dot1x_mac_list, p_db_dot1x_mac);

    /* 4. free db entry */
    XFREE(MEM_TBL_DOT1X_MAC, p_db_dot1x_mac);

    return PM_E_NONE;
}

int32
tbl_dot1x_mac_set_dot1x_mac_field_sync(tbl_dot1x_mac_t *p_dot1x_mac, tbl_dot1x_mac_field_id_t field_id, uint32 sync)
{
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key);
    if (NULL == p_db_dot1x_mac)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DOT1X_MAC_FLD_USER_NAME:
        sal_memcpy(p_db_dot1x_mac->user_name, p_dot1x_mac->user_name, sizeof(p_dot1x_mac->user_name));
        break;

    case TBL_DOT1X_MAC_FLD_SESSION_ID:
        p_db_dot1x_mac->session_id = p_dot1x_mac->session_id;
        break;

    case TBL_DOT1X_MAC_FLD_CURRENT_RADIUS:
        p_db_dot1x_mac->current_radius = p_dot1x_mac->current_radius;
        break;

    case TBL_DOT1X_MAC_FLD_T_WAIT:
        p_db_dot1x_mac->t_wait = p_dot1x_mac->t_wait;
        break;

    case TBL_DOT1X_MAC_FLD_T_REAUTH:
        p_db_dot1x_mac->t_reauth = p_dot1x_mac->t_reauth;
        break;

    case TBL_DOT1X_MAC_FLD_T_REJECT:
        p_db_dot1x_mac->t_reject = p_dot1x_mac->t_reject;
        break;

    case TBL_DOT1X_MAC_FLD_T_DELETE:
        p_db_dot1x_mac->t_delete = p_dot1x_mac->t_delete;
        break;

    case TBL_DOT1X_MAC_FLD_ENTRY_ID:
        p_db_dot1x_mac->entry_id = p_dot1x_mac->entry_id;
        break;

    case TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS:
        p_db_dot1x_mac->is_auth_bypass = p_dot1x_mac->is_auth_bypass;
        break;

    case TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_mac, p_dot1x_mac, auth_mac_flag, AUTH_MAC_REJECT)
        break;

    case TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_mac, p_dot1x_mac, auth_mac_flag, AUTH_MAC_ACCEPT)
        break;

    case TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS:
        CDB_FLAG_CHECK_SET(p_db_dot1x_mac, p_dot1x_mac, auth_mac_flag, AUTH_MAC_BYPASS)
        break;

    case TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_mac, p_dot1x_mac, auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT)
        break;

    case TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH:
        CDB_FLAG_CHECK_SET(p_db_dot1x_mac, p_dot1x_mac, auth_mac_flag, AUTH_MAC_EAPOL_REAUTH)
        break;

    case TBL_DOT1X_MAC_FLD_MAX:
        sal_memcpy(p_db_dot1x_mac, p_dot1x_mac, sizeof(tbl_dot1x_mac_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DOT1X_MAC, field_id, p_db_dot1x_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DOT1X_MAC, field_id, p_db_dot1x_mac);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_mac_add_dot1x_mac(tbl_dot1x_mac_t *p_dot1x_mac)
{
    return tbl_dot1x_mac_add_dot1x_mac_sync(p_dot1x_mac, TRUE);
}

int32
tbl_dot1x_mac_del_dot1x_mac(tbl_dot1x_mac_key_t *p_dot1x_mac_key)
{
    return tbl_dot1x_mac_del_dot1x_mac_sync(p_dot1x_mac_key, TRUE);
}

int32
tbl_dot1x_mac_set_dot1x_mac_field(tbl_dot1x_mac_t *p_dot1x_mac, tbl_dot1x_mac_field_id_t field_id)
{
    return tbl_dot1x_mac_set_dot1x_mac_field_sync(p_dot1x_mac, field_id, TRUE);
}

tbl_dot1x_mac_t*
tbl_dot1x_mac_get_dot1x_mac(tbl_dot1x_mac_key_t *p_dot1x_mac_key)
{
    tbl_dot1x_mac_master_t *p_master = _g_p_tbl_dot1x_mac_master;
    tbl_dot1x_mac_t lkp;

    sal_memcpy(&lkp.key, p_dot1x_mac_key, sizeof(tbl_dot1x_mac_key_t));
    return ctclib_hash_lookup(p_master->dot1x_mac_hash, &lkp);
}

char*
tbl_dot1x_mac_key_val2str(tbl_dot1x_mac_t *p_dot1x_mac, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_dot1x_mac_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_mac->key));
    return str;
}

int32
tbl_dot1x_mac_key_str2val(char *str, tbl_dot1x_mac_t *p_dot1x_mac)
{
    int32 ret = 0;

    ret = cdb_dot1x_mac_key_str2val(str, &p_dot1x_mac->key);
    return ret;
}

char*
tbl_dot1x_mac_key_name_dump(tbl_dot1x_mac_t *p_dot1x_mac, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_MAC);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_dot1x_mac_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_mac->key));
    return str;
}

char*
tbl_dot1x_mac_key_value_dump(tbl_dot1x_mac_t *p_dot1x_mac, char *str)
{

    cdb_dot1x_mac_key_val2str(str, MAX_CMD_STR_LEN, &p_dot1x_mac->key);
    return str;
}

char*
tbl_dot1x_mac_field_name_dump(tbl_dot1x_mac_t *p_dot1x_mac, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_MAC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DOT1X_MAC_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dot1x_mac_field_value_dump(tbl_dot1x_mac_t *p_dot1x_mac, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_KEY, field_id))
    {
        cdb_dot1x_mac_key_val2str(str, MAX_CMD_STR_LEN, &p_dot1x_mac->key);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_USER_NAME, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_mac->user_name, sizeof(p_dot1x_mac->user_name));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_mac->session_id);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_CURRENT_RADIUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_WAIT, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_mac->t_wait);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REAUTH, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_mac->t_reauth);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REJECT, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_mac->t_reject);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_DELETE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_mac->t_delete);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_ENTRY_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_mac->entry_id);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_mac->is_auth_bypass);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH));
    }
    return str;
}

char**
tbl_dot1x_mac_table_dump(tbl_dot1x_mac_t *p_dot1x_mac, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dot1x_mac_key_name_dump(p_dot1x_mac, buf));
    for(i=1; i<TBL_DOT1X_MAC_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_mac_field_name_dump(p_dot1x_mac, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_mac_field_value_dump(p_dot1x_mac, i, buf));
    }
    return str;
}

int32
tbl_dot1x_mac_field_value_parser(char *str, int32 field_id, tbl_dot1x_mac_t *p_dot1x_mac)
{
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_KEY, field_id))
    {
        cdb_dot1x_mac_key_str2val(str, &p_dot1x_mac->key);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_USER_NAME, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_mac->user_name);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_SESSION_ID, field_id))
    {
        int32 ret;
        p_dot1x_mac->session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_CURRENT_RADIUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_WAIT, field_id))
    {
        ctc_task_str2val(str, p_dot1x_mac->t_wait);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REAUTH, field_id))
    {
        ctc_task_str2val(str, p_dot1x_mac->t_reauth);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REJECT, field_id))
    {
        ctc_task_str2val(str, p_dot1x_mac->t_reject);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_DELETE, field_id))
    {
        ctc_task_str2val(str, p_dot1x_mac->t_delete);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_ENTRY_ID, field_id))
    {
        int32 ret;
        p_dot1x_mac->entry_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS, field_id))
    {
        int32 ret;
        p_dot1x_mac->is_auth_bypass = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS, field_id))
    {
        GLB_SET_FLAG(p_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH, field_id))
    {
        GLB_SET_FLAG(p_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH);
    }
    return PM_E_NONE;
}

int32
tbl_dot1x_mac_table_parser(char** array, char* key_value,tbl_dot1x_mac_t *p_dot1x_mac)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_MAC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dot1x_mac_key_str2val(key_value, p_dot1x_mac));

    for(i=1; i<TBL_DOT1X_MAC_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dot1x_mac_field_value_parser( array[j++], i, p_dot1x_mac));
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_mac_dump_one(tbl_dot1x_mac_t *p_dot1x_mac, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_MAC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_KEY].name, 
            cdb_dot1x_mac_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_mac->key));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_USER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_USER_NAME].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_mac->user_name, sizeof(p_dot1x_mac->user_name)));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_SESSION_ID].name,
            p_dot1x_mac->session_id);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_CURRENT_RADIUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_DOT1X_MAC_FLD_CURRENT_RADIUS].name);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_WAIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_T_WAIT].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_mac->t_wait));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REAUTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_T_REAUTH].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_mac->t_reauth));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_REJECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_T_REJECT].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_mac->t_reject));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_T_DELETE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_MAC_FLD_T_DELETE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_mac->t_delete));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_ENTRY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_ENTRY_ID].name,
            p_dot1x_mac->entry_id);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_IS_AUTH_BYPASS].name,
            p_dot1x_mac->is_auth_bypass);
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT].name,
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT].name,
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_AUTH_MAC_BYPASS].name,
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT].name,
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT));
    }
    if (FLD_MATCH(TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_MAC_FLD_AUTH_MAC_EAPOL_REAUTH].name,
            GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dot1x_mac_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dot1x_mac_master_t *p_master = _g_p_tbl_dot1x_mac_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->dot1x_mac_list, p_db_dot1x_mac, listnode, next)
    {
        rc |= fn(p_db_dot1x_mac, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dot1x_mac_master_t*
tbl_dot1x_mac_get_master()
{
    return _g_p_tbl_dot1x_mac_master;
}

tbl_dot1x_mac_master_t*
tbl_dot1x_mac_init_dot1x_mac()
{
    _g_p_tbl_dot1x_mac_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dot1x_mac_master_t));
    _g_p_tbl_dot1x_mac_master->dot1x_mac_hash = ctclib_hash_create(_tbl_dot1x_mac_hash_make, _tbl_dot1x_mac_hash_cmp);
    _g_p_tbl_dot1x_mac_master->dot1x_mac_list = ctclib_slist_create(tbl_dot1x_mac_cmp, NULL);
    return _g_p_tbl_dot1x_mac_master;
}


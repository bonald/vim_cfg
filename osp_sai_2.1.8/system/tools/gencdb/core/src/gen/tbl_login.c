
#include "proto.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "cdb_data_cmp.h"

tbl_login_master_t *_g_p_tbl_login_master = NULL;

int32
tbl_login_add_login_sync(tbl_login_t *p_login, uint32 sync)
{
    tbl_login_master_t *p_master = _g_p_tbl_login_master;
    tbl_login_t *p_db_login = NULL;

    /* 1. lookup entry exist */
    if (tbl_login_get_login(&p_login->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_login = XCALLOC(MEM_TBL_LOGIN, sizeof(tbl_login_t));
    if (NULL == p_db_login)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_login, p_login, sizeof(tbl_login_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->login_list, p_db_login);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LOGIN, p_db_login);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LOGIN, p_db_login);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_login_del_login_sync(tbl_login_key_t *p_login_key, uint32 sync)
{
    tbl_login_master_t *p_master = _g_p_tbl_login_master;
    tbl_login_t *p_db_login = NULL;

    /* 1. lookup entry exist */
    p_db_login = tbl_login_get_login(p_login_key);
    if (NULL == p_db_login)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LOGIN, p_db_login);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LOGIN, p_db_login);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->login_list, p_db_login);

    /* 4. free db entry */
    XFREE(MEM_TBL_LOGIN, p_db_login);

    return PM_E_NONE;
}

int32
tbl_login_set_login_field_sync(tbl_login_t *p_login, tbl_login_field_id_t field_id, uint32 sync)
{
    tbl_login_t *p_db_login = NULL;

    /* 1. lookup entry exist */
    p_db_login = tbl_login_get_login(&p_login->key);
    if (NULL == p_db_login)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LOGIN_FLD_USER:
        sal_memcpy(&p_db_login->user, &p_login->user, sizeof(p_login->user));
        break;

    case TBL_LOGIN_FLD_VTY:
        sal_memcpy(p_db_login->vty, p_login->vty, sizeof(p_login->vty));
        break;

    case TBL_LOGIN_FLD_PRIVILEGE:
        p_db_login->privilege = p_login->privilege;
        break;

    case TBL_LOGIN_FLD_IPADDR:
        sal_memcpy(p_db_login->ipaddr, p_login->ipaddr, sizeof(p_login->ipaddr));
        break;

    case TBL_LOGIN_FLD_DEST_IPADDR:
        sal_memcpy(p_db_login->dest_ipaddr, p_login->dest_ipaddr, sizeof(p_login->dest_ipaddr));
        break;

    case TBL_LOGIN_FLD_REMOTE_PORT:
        p_db_login->remote_port = p_login->remote_port;
        break;

    case TBL_LOGIN_FLD_DEST_PORT:
        p_db_login->dest_port = p_login->dest_port;
        break;

    case TBL_LOGIN_FLD_PROTOCOL:
        sal_memcpy(p_db_login->protocol, p_login->protocol, sizeof(p_login->protocol));
        break;

    case TBL_LOGIN_FLD_EXPIRE_TIME:
        p_db_login->expire_time = p_login->expire_time;
        break;

    case TBL_LOGIN_FLD_PID:
        p_db_login->pid = p_login->pid;
        break;

    case TBL_LOGIN_FLD_MAX:
        sal_memcpy(p_db_login, p_login, sizeof(tbl_login_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LOGIN, field_id, p_db_login);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LOGIN, field_id, p_db_login);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_login_add_login(tbl_login_t *p_login)
{
    return tbl_login_add_login_sync(p_login, TRUE);
}

int32
tbl_login_del_login(tbl_login_key_t *p_login_key)
{
    return tbl_login_del_login_sync(p_login_key, TRUE);
}

int32
tbl_login_set_login_field(tbl_login_t *p_login, tbl_login_field_id_t field_id)
{
    return tbl_login_set_login_field_sync(p_login, field_id, TRUE);
}

tbl_login_t*
tbl_login_get_login(tbl_login_key_t *p_login_key)
{
    tbl_login_master_t *p_master = _g_p_tbl_login_master;
    tbl_login_t lkp;

    sal_memcpy(&lkp.key, p_login_key, sizeof(tbl_login_key_t));
    return ctclib_slistdata_lookup(p_master->login_list, &lkp);
}

char*
tbl_login_key_val2str(tbl_login_t *p_login, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_login->key.name);
    return str;
}

int32
tbl_login_key_str2val(char *str, tbl_login_t *p_login)
{
    int32 ret = 0;

    sal_strcpy(p_login->key.name, str);
    return ret;
}

char*
tbl_login_key_name_dump(tbl_login_t *p_login, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_login->key.name);
    return str;
}

char*
tbl_login_key_value_dump(tbl_login_t *p_login, char *str)
{
    sal_sprintf(str, "%s", p_login->key.name);
    return str;
}

char*
tbl_login_field_name_dump(tbl_login_t *p_login, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LOGIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_login_field_value_dump(tbl_login_t *p_login, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LOGIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_login->key.name);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_USER, field_id))
    {
        cdb_user_val2str(str, MAX_CMD_STR_LEN, &p_login->user);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_VTY, field_id))
    {
        sal_sprintf(str, "%s", p_login->vty);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_login->privilege);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_IPADDR, field_id))
    {
        sal_sprintf(str, "%s", p_login->ipaddr);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_IPADDR, field_id))
    {
        sal_sprintf(str, "%s", p_login->dest_ipaddr);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_REMOTE_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_login->remote_port);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_login->dest_port);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%s", p_login->protocol);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_EXPIRE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_login->expire_time);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PID, field_id))
    {
        sal_sprintf(str, "%u", p_login->pid);
    }
    return str;
}

char**
tbl_login_table_dump(tbl_login_t *p_login, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_login_key_name_dump(p_login, buf));
    for(i=1; i<TBL_LOGIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_login_field_name_dump(p_login, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_login_field_value_dump(p_login, i, buf));
    }
    return str;
}

int32
tbl_login_field_value_parser(char *str, int32 field_id, tbl_login_t *p_login)
{
    if (FLD_MATCH(TBL_LOGIN_FLD_KEY, field_id))
    {
        sal_strcpy(p_login->key.name, str);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_USER, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_user_str2val(&p_login->user, str, err);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_VTY, field_id))
    {
        sal_strcpy(p_login->vty, str);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PRIVILEGE, field_id))
    {
        int32 ret;
        p_login->privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_IPADDR, field_id))
    {
        sal_strcpy(p_login->ipaddr, str);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_IPADDR, field_id))
    {
        sal_strcpy(p_login->dest_ipaddr, str);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_REMOTE_PORT, field_id))
    {
        int32 ret;
        p_login->remote_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_PORT, field_id))
    {
        int32 ret;
        p_login->dest_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PROTOCOL, field_id))
    {
        sal_strcpy(p_login->protocol, str);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_EXPIRE_TIME, field_id))
    {
        int32 ret;
        p_login->expire_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PID, field_id))
    {
        int32 ret;
        p_login->pid = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_login_table_parser(char** array, char* key_value,tbl_login_t *p_login)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_login_key_str2val(key_value, p_login));

    for(i=1; i<TBL_LOGIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_login_field_value_parser( array[j++], i, p_login));
    }

    return PM_E_NONE;
}

int32
tbl_login_dump_one(tbl_login_t *p_login, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LOGIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_KEY].name,
            p_login->key.name);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_USER].name, 
            cdb_user_val2str(buf, MAX_CMD_STR_LEN, &p_login->user));
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_VTY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_VTY].name,
            p_login->vty);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_FLD_PRIVILEGE].name,
            p_login->privilege);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_IPADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_IPADDR].name,
            p_login->ipaddr);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_IPADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_DEST_IPADDR].name,
            p_login->dest_ipaddr);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_REMOTE_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_FLD_REMOTE_PORT].name,
            p_login->remote_port);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_DEST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_FLD_DEST_PORT].name,
            p_login->dest_port);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_FLD_PROTOCOL].name,
            p_login->protocol);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_EXPIRE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_FLD_EXPIRE_TIME].name,
            p_login->expire_time);
    }
    if (FLD_MATCH(TBL_LOGIN_FLD_PID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_FLD_PID].name,
            p_login->pid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_login_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_login_master_t *p_master = _g_p_tbl_login_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_t *p_db_login = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->login_list, p_db_login, listnode, next)
    {
        rc |= fn(p_db_login, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_login_master_t*
tbl_login_get_master()
{
    return _g_p_tbl_login_master;
}

tbl_login_master_t*
tbl_login_init_login()
{
    _g_p_tbl_login_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_login_master_t));
    _g_p_tbl_login_master->login_list = ctclib_slist_create(tbl_login_cmp, NULL);
    return _g_p_tbl_login_master;
}


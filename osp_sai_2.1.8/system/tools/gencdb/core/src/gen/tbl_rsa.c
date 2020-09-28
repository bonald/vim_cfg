
#include "proto.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "cdb_data_cmp.h"

tbl_rsa_master_t *_g_p_tbl_rsa_master = NULL;

int32
tbl_rsa_add_rsa_sync(tbl_rsa_t *p_rsa, uint32 sync)
{
    tbl_rsa_master_t *p_master = _g_p_tbl_rsa_master;
    tbl_rsa_t *p_db_rsa = NULL;

    /* 1. lookup entry exist */
    if (tbl_rsa_get_rsa(&p_rsa->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rsa = XCALLOC(MEM_TBL_RSA, sizeof(tbl_rsa_t));
    if (NULL == p_db_rsa)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rsa, p_rsa, sizeof(tbl_rsa_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->rsa_list, p_db_rsa);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RSA, p_db_rsa);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RSA, p_db_rsa);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rsa_del_rsa_sync(tbl_rsa_key_t *p_rsa_key, uint32 sync)
{
    tbl_rsa_master_t *p_master = _g_p_tbl_rsa_master;
    tbl_rsa_t *p_db_rsa = NULL;

    /* 1. lookup entry exist */
    p_db_rsa = tbl_rsa_get_rsa(p_rsa_key);
    if (NULL == p_db_rsa)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RSA, p_db_rsa);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RSA, p_db_rsa);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->rsa_list, p_db_rsa);

    /* 4. free db entry */
    XFREE(MEM_TBL_RSA, p_db_rsa);

    return PM_E_NONE;
}

int32
tbl_rsa_set_rsa_field_sync(tbl_rsa_t *p_rsa, tbl_rsa_field_id_t field_id, uint32 sync)
{
    tbl_rsa_t *p_db_rsa = NULL;

    /* 1. lookup entry exist */
    p_db_rsa = tbl_rsa_get_rsa(&p_rsa->key);
    if (NULL == p_db_rsa)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RSA_FLD_TYPE:
        p_db_rsa->type = p_rsa->type;
        break;

    case TBL_RSA_FLD_REFCNT:
        p_db_rsa->refcnt = p_rsa->refcnt;
        break;

    case TBL_RSA_FLD_PEM:
        sal_memcpy(&p_db_rsa->pem, &p_rsa->pem, sizeof(p_rsa->pem));
        break;

    case TBL_RSA_FLD_ENC_PASSWD:
        sal_memcpy(p_db_rsa->enc_passwd, p_rsa->enc_passwd, sizeof(p_rsa->enc_passwd));
        break;

    case TBL_RSA_FLD_PASSWD:
        sal_memcpy(p_db_rsa->passwd, p_rsa->passwd, sizeof(p_rsa->passwd));
        break;

    case TBL_RSA_FLD_MAX:
        sal_memcpy(p_db_rsa, p_rsa, sizeof(tbl_rsa_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RSA, field_id, p_db_rsa);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RSA, field_id, p_db_rsa);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rsa_add_rsa(tbl_rsa_t *p_rsa)
{
    return tbl_rsa_add_rsa_sync(p_rsa, TRUE);
}

int32
tbl_rsa_del_rsa(tbl_rsa_key_t *p_rsa_key)
{
    return tbl_rsa_del_rsa_sync(p_rsa_key, TRUE);
}

int32
tbl_rsa_set_rsa_field(tbl_rsa_t *p_rsa, tbl_rsa_field_id_t field_id)
{
    return tbl_rsa_set_rsa_field_sync(p_rsa, field_id, TRUE);
}

tbl_rsa_t*
tbl_rsa_get_rsa(tbl_rsa_key_t *p_rsa_key)
{
    tbl_rsa_master_t *p_master = _g_p_tbl_rsa_master;
    tbl_rsa_t lkp;

    sal_memcpy(&lkp.key, p_rsa_key, sizeof(tbl_rsa_key_t));
    return ctclib_slistdata_lookup(p_master->rsa_list, &lkp);
}

char*
tbl_rsa_key_val2str(tbl_rsa_t *p_rsa, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_rsa->key.name);
    return str;
}

int32
tbl_rsa_key_str2val(char *str, tbl_rsa_t *p_rsa)
{
    int32 ret = 0;

    sal_strcpy(p_rsa->key.name, str);
    return ret;
}

char*
tbl_rsa_key_name_dump(tbl_rsa_t *p_rsa, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RSA);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_rsa->key.name);
    return str;
}

char*
tbl_rsa_key_value_dump(tbl_rsa_t *p_rsa, char *str)
{
    sal_sprintf(str, "%s", p_rsa->key.name);
    return str;
}

char*
tbl_rsa_field_name_dump(tbl_rsa_t *p_rsa, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RSA);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RSA_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rsa_field_value_dump(tbl_rsa_t *p_rsa, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RSA_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_rsa->key.name);
    }
    if (FLD_MATCH(TBL_RSA_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(key_type_strs, KEY_TYPE_MAX, p_rsa->type));
    }
    if (FLD_MATCH(TBL_RSA_FLD_REFCNT, field_id))
    {
        sal_sprintf(str, "%u", p_rsa->refcnt);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PEM, field_id))
    {
        cdb_rsa_keystr_val2str(str, MAX_CMD_STR_LEN, &p_rsa->pem);
    }
    if (FLD_MATCH(TBL_RSA_FLD_ENC_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_rsa->enc_passwd);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_rsa->passwd);
    }
    return str;
}

char**
tbl_rsa_table_dump(tbl_rsa_t *p_rsa, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rsa_key_name_dump(p_rsa, buf));
    for(i=1; i<TBL_RSA_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rsa_field_name_dump(p_rsa, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rsa_field_value_dump(p_rsa, i, buf));
    }
    return str;
}

int32
tbl_rsa_field_value_parser(char *str, int32 field_id, tbl_rsa_t *p_rsa)
{
    if (FLD_MATCH(TBL_RSA_FLD_KEY, field_id))
    {
        sal_strcpy(p_rsa->key.name, str);
    }
    if (FLD_MATCH(TBL_RSA_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(key_type_strs, KEY_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_RSA_FLD_REFCNT, field_id))
    {
        int32 ret;
        p_rsa->refcnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PEM, field_id))
    {
        cdb_rsa_keystr_str2val(str, &p_rsa->pem);
    }
    if (FLD_MATCH(TBL_RSA_FLD_ENC_PASSWD, field_id))
    {
        sal_strcpy(p_rsa->enc_passwd, str);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_rsa->passwd, str);
    }
    return PM_E_NONE;
}

int32
tbl_rsa_table_parser(char** array, char* key_value,tbl_rsa_t *p_rsa)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RSA);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rsa_key_str2val(key_value, p_rsa));

    for(i=1; i<TBL_RSA_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rsa_field_value_parser( array[j++], i, p_rsa));
    }

    return PM_E_NONE;
}

int32
tbl_rsa_dump_one(tbl_rsa_t *p_rsa, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RSA);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RSA_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RSA_FLD_KEY].name,
            p_rsa->key.name);
    }
    if (FLD_MATCH(TBL_RSA_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RSA_FLD_TYPE].name, 
            cdb_enum_val2str(key_type_strs, KEY_TYPE_MAX, p_rsa->type));
    }
    if (FLD_MATCH(TBL_RSA_FLD_REFCNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RSA_FLD_REFCNT].name,
            p_rsa->refcnt);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PEM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RSA_FLD_PEM].name, 
            cdb_rsa_keystr_val2str(buf, MAX_CMD_STR_LEN, &p_rsa->pem));
    }
    if (FLD_MATCH(TBL_RSA_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RSA_FLD_ENC_PASSWD].name,
            p_rsa->enc_passwd);
    }
    if (FLD_MATCH(TBL_RSA_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RSA_FLD_PASSWD].name,
            p_rsa->passwd);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rsa_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rsa_master_t *p_master = _g_p_tbl_rsa_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rsa_t *p_db_rsa = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rsa_list, p_db_rsa, listnode, next)
    {
        rc |= fn(p_db_rsa, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rsa_master_t*
tbl_rsa_get_master()
{
    return _g_p_tbl_rsa_master;
}

tbl_rsa_master_t*
tbl_rsa_init_rsa()
{
    _g_p_tbl_rsa_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rsa_master_t));
    _g_p_tbl_rsa_master->rsa_list = ctclib_slist_create(tbl_rsa_cmp, NULL);
    return _g_p_tbl_rsa_master;
}


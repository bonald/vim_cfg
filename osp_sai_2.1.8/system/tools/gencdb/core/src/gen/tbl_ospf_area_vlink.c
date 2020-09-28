
#include "proto.h"
#include "gen/tbl_ospf_area_vlink_define.h"
#include "gen/tbl_ospf_area_vlink.h"
#include "cdb_data_cmp.h"

tbl_ospf_area_vlink_master_t *_g_p_tbl_ospf_area_vlink_master = NULL;

static uint32
_tbl_ospf_area_vlink_hash_make(tbl_ospf_area_vlink_t *p_ospf_area_vlink)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf_area_vlink->key;
    for (index = 0; index < sizeof(p_ospf_area_vlink->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf_area_vlink_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf_area_vlink_t *p_ospf_area_vlink1 = (tbl_ospf_area_vlink_t*)p_arg1;
    tbl_ospf_area_vlink_t *p_ospf_area_vlink2 = (tbl_ospf_area_vlink_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf_area_vlink1->key, &p_ospf_area_vlink2->key, sizeof(tbl_ospf_area_vlink_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf_area_vlink_add_ospf_area_vlink_sync(tbl_ospf_area_vlink_t *p_ospf_area_vlink, uint32 sync)
{
    tbl_ospf_area_vlink_master_t *p_master = _g_p_tbl_ospf_area_vlink_master;
    tbl_ospf_area_vlink_t *p_db_ospf_area_vlink = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf_area_vlink_get_ospf_area_vlink(&p_ospf_area_vlink->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf_area_vlink = XCALLOC(MEM_TBL_OSPF_AREA_VLINK, sizeof(tbl_ospf_area_vlink_t));
    if (NULL == p_db_ospf_area_vlink)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf_area_vlink, p_ospf_area_vlink, sizeof(tbl_ospf_area_vlink_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf_area_vlink_hash, (void*)p_db_ospf_area_vlink, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf_area_vlink_list, p_db_ospf_area_vlink);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF_AREA_VLINK, p_db_ospf_area_vlink);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF_AREA_VLINK, p_db_ospf_area_vlink);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_del_ospf_area_vlink_sync(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key, uint32 sync)
{
    tbl_ospf_area_vlink_master_t *p_master = _g_p_tbl_ospf_area_vlink_master;
    tbl_ospf_area_vlink_t *p_db_ospf_area_vlink = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_area_vlink = tbl_ospf_area_vlink_get_ospf_area_vlink(p_ospf_area_vlink_key);
    if (NULL == p_db_ospf_area_vlink)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF_AREA_VLINK, p_db_ospf_area_vlink);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF_AREA_VLINK, p_db_ospf_area_vlink);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf_area_vlink_hash, (void*)p_db_ospf_area_vlink);
    ctclib_slistnode_delete(p_master->ospf_area_vlink_list, p_db_ospf_area_vlink);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF_AREA_VLINK, p_db_ospf_area_vlink);

    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_set_ospf_area_vlink_field_sync(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_ospf_area_vlink_field_id_t field_id, uint32 sync)
{
    tbl_ospf_area_vlink_t *p_db_ospf_area_vlink = NULL;

    /* 1. lookup entry exist */
    p_db_ospf_area_vlink = tbl_ospf_area_vlink_get_ospf_area_vlink(&p_ospf_area_vlink->key);
    if (NULL == p_db_ospf_area_vlink)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG:
        p_db_ospf_area_vlink->auth_flag = p_ospf_area_vlink->auth_flag;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE:
        p_db_ospf_area_vlink->auth_type = p_ospf_area_vlink->auth_type;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY:
        sal_memcpy(p_db_ospf_area_vlink->auth_key, p_ospf_area_vlink->auth_key, sizeof(p_ospf_area_vlink->auth_key));
        break;

    case TBL_OSPF_AREA_VLINK_FLD_MD5_ID:
        p_db_ospf_area_vlink->md5_id = p_ospf_area_vlink->md5_id;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_MD5_KEY:
        sal_memcpy(p_db_ospf_area_vlink->md5_key, p_ospf_area_vlink->md5_key, sizeof(p_ospf_area_vlink->md5_key));
        break;

    case TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL:
        p_db_ospf_area_vlink->hello_interval = p_ospf_area_vlink->hello_interval;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL:
        p_db_ospf_area_vlink->dead_interval = p_ospf_area_vlink->dead_interval;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL:
        p_db_ospf_area_vlink->retransmit_interval = p_ospf_area_vlink->retransmit_interval;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY:
        p_db_ospf_area_vlink->transmit_delay = p_ospf_area_vlink->transmit_delay;
        break;

    case TBL_OSPF_AREA_VLINK_FLD_MAX:
        sal_memcpy(p_db_ospf_area_vlink, p_ospf_area_vlink, sizeof(tbl_ospf_area_vlink_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF_AREA_VLINK, field_id, p_db_ospf_area_vlink);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF_AREA_VLINK, field_id, p_db_ospf_area_vlink);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_add_ospf_area_vlink(tbl_ospf_area_vlink_t *p_ospf_area_vlink)
{
    return tbl_ospf_area_vlink_add_ospf_area_vlink_sync(p_ospf_area_vlink, TRUE);
}

int32
tbl_ospf_area_vlink_del_ospf_area_vlink(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key)
{
    return tbl_ospf_area_vlink_del_ospf_area_vlink_sync(p_ospf_area_vlink_key, TRUE);
}

int32
tbl_ospf_area_vlink_set_ospf_area_vlink_field(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_ospf_area_vlink_field_id_t field_id)
{
    return tbl_ospf_area_vlink_set_ospf_area_vlink_field_sync(p_ospf_area_vlink, field_id, TRUE);
}

tbl_ospf_area_vlink_t*
tbl_ospf_area_vlink_get_ospf_area_vlink(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key)
{
    tbl_ospf_area_vlink_master_t *p_master = _g_p_tbl_ospf_area_vlink_master;
    tbl_ospf_area_vlink_t lkp;

    sal_memcpy(&lkp.key, p_ospf_area_vlink_key, sizeof(tbl_ospf_area_vlink_key_t));
    return ctclib_hash_lookup(p_master->ospf_area_vlink_hash, &lkp);
}

char*
tbl_ospf_area_vlink_key_val2str(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ospf_area_vlink_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_area_vlink->key));
    return str;
}

int32
tbl_ospf_area_vlink_key_str2val(char *str, tbl_ospf_area_vlink_t *p_ospf_area_vlink)
{
    int32 ret = 0;

    ret = cdb_ospf_area_vlink_key_str2val(str, &p_ospf_area_vlink->key);
    return ret;
}

char*
tbl_ospf_area_vlink_key_name_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_VLINK);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ospf_area_vlink_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_area_vlink->key));
    return str;
}

char*
tbl_ospf_area_vlink_key_value_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char *str)
{

    cdb_ospf_area_vlink_key_val2str(str, MAX_CMD_STR_LEN, &p_ospf_area_vlink->key);
    return str;
}

char*
tbl_ospf_area_vlink_field_name_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_VLINK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF_AREA_VLINK_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf_area_vlink_field_value_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_KEY, field_id))
    {
        cdb_ospf_area_vlink_key_val2str(str, MAX_CMD_STR_LEN, &p_ospf_area_vlink->key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG, field_id))
    {
        sal_sprintf(str, "%d", p_ospf_area_vlink->auth_flag);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ospf_vlink_auth_type_strs, OSPF_VLINK_AUTH_TYPE_MAX, p_ospf_area_vlink->auth_type));
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ospf_area_vlink->auth_key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_ID, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_vlink->md5_id);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ospf_area_vlink->md5_key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_vlink->hello_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_vlink->dead_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_vlink->retransmit_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY, field_id))
    {
        sal_sprintf(str, "%u", p_ospf_area_vlink->transmit_delay);
    }
    return str;
}

char**
tbl_ospf_area_vlink_table_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf_area_vlink_key_name_dump(p_ospf_area_vlink, buf));
    for(i=1; i<TBL_OSPF_AREA_VLINK_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf_area_vlink_field_name_dump(p_ospf_area_vlink, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf_area_vlink_field_value_dump(p_ospf_area_vlink, i, buf));
    }
    return str;
}

int32
tbl_ospf_area_vlink_field_value_parser(char *str, int32 field_id, tbl_ospf_area_vlink_t *p_ospf_area_vlink)
{
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_KEY, field_id))
    {
        cdb_ospf_area_vlink_key_str2val(str, &p_ospf_area_vlink->key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG, field_id))
    {
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE, field_id))
    {
        cdb_enum_str2val(ospf_vlink_auth_type_strs, OSPF_VLINK_AUTH_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY, field_id))
    {
        sal_strcpy(p_ospf_area_vlink->auth_key, str);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_ID, field_id))
    {
        int32 ret;
        p_ospf_area_vlink->md5_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_KEY, field_id))
    {
        sal_strcpy(p_ospf_area_vlink->md5_key, str);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL, field_id))
    {
        int32 ret;
        p_ospf_area_vlink->hello_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL, field_id))
    {
        int32 ret;
        p_ospf_area_vlink->dead_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL, field_id))
    {
        int32 ret;
        p_ospf_area_vlink->retransmit_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY, field_id))
    {
        int32 ret;
        p_ospf_area_vlink->transmit_delay = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_table_parser(char** array, char* key_value,tbl_ospf_area_vlink_t *p_ospf_area_vlink)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_VLINK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf_area_vlink_key_str2val(key_value, p_ospf_area_vlink));

    for(i=1; i<TBL_OSPF_AREA_VLINK_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf_area_vlink_field_value_parser( array[j++], i, p_ospf_area_vlink));
    }

    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_dump_one(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF_AREA_VLINK);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_KEY].name, 
            cdb_ospf_area_vlink_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf_area_vlink->key));
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG].name,
            p_ospf_area_vlink->auth_flag);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE].name, 
            cdb_enum_val2str(ospf_vlink_auth_type_strs, OSPF_VLINK_AUTH_TYPE_MAX, p_ospf_area_vlink->auth_type));
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY].name,
            p_ospf_area_vlink->auth_key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_MD5_ID].name,
            p_ospf_area_vlink->md5_id);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_MD5_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_MD5_KEY].name,
            p_ospf_area_vlink->md5_key);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL].name,
            p_ospf_area_vlink->hello_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL].name,
            p_ospf_area_vlink->dead_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL].name,
            p_ospf_area_vlink->retransmit_interval);
    }
    if (FLD_MATCH(TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY].name,
            p_ospf_area_vlink->transmit_delay);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf_area_vlink_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf_area_vlink_master_t *p_master = _g_p_tbl_ospf_area_vlink_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf_area_vlink_t *p_db_ospf_area_vlink = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf_area_vlink_list, p_db_ospf_area_vlink, listnode, next)
    {
        rc |= fn(p_db_ospf_area_vlink, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf_area_vlink_master_t*
tbl_ospf_area_vlink_get_master()
{
    return _g_p_tbl_ospf_area_vlink_master;
}

tbl_ospf_area_vlink_master_t*
tbl_ospf_area_vlink_init_ospf_area_vlink()
{
    _g_p_tbl_ospf_area_vlink_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf_area_vlink_master_t));
    _g_p_tbl_ospf_area_vlink_master->ospf_area_vlink_hash = ctclib_hash_create(_tbl_ospf_area_vlink_hash_make, _tbl_ospf_area_vlink_hash_cmp);
    _g_p_tbl_ospf_area_vlink_master->ospf_area_vlink_list = ctclib_slist_create(tbl_ospf_area_vlink_cmp, NULL);
    return _g_p_tbl_ospf_area_vlink_master;
}


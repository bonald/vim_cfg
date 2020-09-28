
#include "proto.h"
#include "gen/tbl_fan_define.h"
#include "gen/tbl_fan.h"
#include "cdb_data_cmp.h"

tbl_fan_master_t *_g_p_tbl_fan_master = NULL;

static uint32
_tbl_fan_hash_make(tbl_fan_t *p_fan)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fan->key;
    for (index = 0; index < sizeof(p_fan->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fan_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fan_t *p_fan1 = (tbl_fan_t*)p_arg1;
    tbl_fan_t *p_fan2 = (tbl_fan_t*)p_arg2;

    if (0 == sal_memcmp(&p_fan1->key, &p_fan2->key, sizeof(tbl_fan_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fan_add_fan_sync(tbl_fan_t *p_fan, uint32 sync)
{
    tbl_fan_master_t *p_master = _g_p_tbl_fan_master;
    tbl_fan_t *p_db_fan = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fan_get_fan(&p_fan->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fan = XCALLOC(MEM_TBL_FAN, sizeof(tbl_fan_t));
    if (NULL == p_db_fan)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fan, p_fan, sizeof(tbl_fan_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fan_hash, (void*)p_db_fan, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->fan_list, p_db_fan);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FAN, p_db_fan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FAN, p_db_fan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fan_del_fan_sync(tbl_fan_key_t *p_fan_key, uint32 sync)
{
    tbl_fan_master_t *p_master = _g_p_tbl_fan_master;
    tbl_fan_t *p_db_fan = NULL;

    /* 1. lookup entry exist */
    p_db_fan = tbl_fan_get_fan(p_fan_key);
    if (NULL == p_db_fan)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FAN, p_db_fan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FAN, p_db_fan);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fan_hash, (void*)p_db_fan);
    ctclib_slistnode_delete(p_master->fan_list, p_db_fan);

    /* 4. free db entry */
    XFREE(MEM_TBL_FAN, p_db_fan);

    return PM_E_NONE;
}

int32
tbl_fan_set_fan_field_sync(tbl_fan_t *p_fan, tbl_fan_field_id_t field_id, uint32 sync)
{
    tbl_fan_t *p_db_fan = NULL;

    /* 1. lookup entry exist */
    p_db_fan = tbl_fan_get_fan(&p_fan->key);
    if (NULL == p_db_fan)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FAN_FLD_SLOT:
        p_db_fan->slot = p_fan->slot;
        break;

    case TBL_FAN_FLD_TRAY:
        p_db_fan->tray = p_fan->tray;
        break;

    case TBL_FAN_FLD_PERCENT:
        p_db_fan->percent = p_fan->percent;
        break;

    case TBL_FAN_FLD_ABSENT:
        p_db_fan->absent = p_fan->absent;
        break;

    case TBL_FAN_FLD_NAME:
        sal_memcpy(p_db_fan->name, p_fan->name, sizeof(p_fan->name));
        break;

    case TBL_FAN_FLD_DIRECTION:
        sal_memcpy(p_db_fan->direction, p_fan->direction, sizeof(p_fan->direction));
        break;

    case TBL_FAN_FLD_STATUS:
        sal_memcpy(p_db_fan->status, p_fan->status, sizeof(p_fan->status));
        break;

    case TBL_FAN_FLD_MODE:
        sal_memcpy(p_db_fan->mode, p_fan->mode, sizeof(p_fan->mode));
        break;

    case TBL_FAN_FLD_MAX:
        sal_memcpy(p_db_fan, p_fan, sizeof(tbl_fan_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FAN, field_id, p_db_fan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FAN, field_id, p_db_fan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fan_add_fan(tbl_fan_t *p_fan)
{
    return tbl_fan_add_fan_sync(p_fan, TRUE);
}

int32
tbl_fan_del_fan(tbl_fan_key_t *p_fan_key)
{
    return tbl_fan_del_fan_sync(p_fan_key, TRUE);
}

int32
tbl_fan_set_fan_field(tbl_fan_t *p_fan, tbl_fan_field_id_t field_id)
{
    return tbl_fan_set_fan_field_sync(p_fan, field_id, TRUE);
}

tbl_fan_t*
tbl_fan_get_fan(tbl_fan_key_t *p_fan_key)
{
    tbl_fan_master_t *p_master = _g_p_tbl_fan_master;
    tbl_fan_t lkp;

    sal_memcpy(&lkp.key, p_fan_key, sizeof(tbl_fan_key_t));
    return ctclib_hash_lookup(p_master->fan_hash, &lkp);
}

char*
tbl_fan_key_val2str(tbl_fan_t *p_fan, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_fan->key.id);
    return str;
}

int32
tbl_fan_key_str2val(char *str, tbl_fan_t *p_fan)
{
    int32 ret = 0;

    p_fan->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_fan_key_name_dump(tbl_fan_t *p_fan, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FAN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_fan->key.id);
    return str;
}

char*
tbl_fan_key_value_dump(tbl_fan_t *p_fan, char *str)
{
    sal_sprintf(str, "%d", p_fan->key.id);
    return str;
}

char*
tbl_fan_field_name_dump(tbl_fan_t *p_fan, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FAN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fan_field_value_dump(tbl_fan_t *p_fan, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FAN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_fan->key.id);
    }
    if (FLD_MATCH(TBL_FAN_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_fan->slot);
    }
    if (FLD_MATCH(TBL_FAN_FLD_TRAY, field_id))
    {
        sal_sprintf(str, "%d", p_fan->tray);
    }
    if (FLD_MATCH(TBL_FAN_FLD_PERCENT, field_id))
    {
        sal_sprintf(str, "%d", p_fan->percent);
    }
    if (FLD_MATCH(TBL_FAN_FLD_ABSENT, field_id))
    {
        sal_sprintf(str, "%d", p_fan->absent);
    }
    if (FLD_MATCH(TBL_FAN_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_fan->name);
    }
    if (FLD_MATCH(TBL_FAN_FLD_DIRECTION, field_id))
    {
        sal_sprintf(str, "%s", p_fan->direction);
    }
    if (FLD_MATCH(TBL_FAN_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_fan->status);
    }
    if (FLD_MATCH(TBL_FAN_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", p_fan->mode);
    }
    return str;
}

char**
tbl_fan_table_dump(tbl_fan_t *p_fan, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fan_key_name_dump(p_fan, buf));
    for(i=1; i<TBL_FAN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fan_field_name_dump(p_fan, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fan_field_value_dump(p_fan, i, buf));
    }
    return str;
}

int32
tbl_fan_field_value_parser(char *str, int32 field_id, tbl_fan_t *p_fan)
{
    if (FLD_MATCH(TBL_FAN_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_FAN_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_FAN_FLD_TRAY, field_id))
    {
    }
    if (FLD_MATCH(TBL_FAN_FLD_PERCENT, field_id))
    {
    }
    if (FLD_MATCH(TBL_FAN_FLD_ABSENT, field_id))
    {
    }
    if (FLD_MATCH(TBL_FAN_FLD_NAME, field_id))
    {
        sal_strcpy(p_fan->name, str);
    }
    if (FLD_MATCH(TBL_FAN_FLD_DIRECTION, field_id))
    {
        sal_strcpy(p_fan->direction, str);
    }
    if (FLD_MATCH(TBL_FAN_FLD_STATUS, field_id))
    {
        sal_strcpy(p_fan->status, str);
    }
    if (FLD_MATCH(TBL_FAN_FLD_MODE, field_id))
    {
        sal_strcpy(p_fan->mode, str);
    }
    return PM_E_NONE;
}

int32
tbl_fan_table_parser(char** array, char* key_value,tbl_fan_t *p_fan)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fan_key_str2val(key_value, p_fan));

    for(i=1; i<TBL_FAN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fan_field_value_parser( array[j++], i, p_fan));
    }

    return PM_E_NONE;
}

int32
tbl_fan_dump_one(tbl_fan_t *p_fan, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FAN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FAN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FAN_FLD_KEY].name,
            p_fan->key.id);
    }
    if (FLD_MATCH(TBL_FAN_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FAN_FLD_SLOT].name,
            p_fan->slot);
    }
    if (FLD_MATCH(TBL_FAN_FLD_TRAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FAN_FLD_TRAY].name,
            p_fan->tray);
    }
    if (FLD_MATCH(TBL_FAN_FLD_PERCENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FAN_FLD_PERCENT].name,
            p_fan->percent);
    }
    if (FLD_MATCH(TBL_FAN_FLD_ABSENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FAN_FLD_ABSENT].name,
            p_fan->absent);
    }
    if (FLD_MATCH(TBL_FAN_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FAN_FLD_NAME].name,
            p_fan->name);
    }
    if (FLD_MATCH(TBL_FAN_FLD_DIRECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FAN_FLD_DIRECTION].name,
            p_fan->direction);
    }
    if (FLD_MATCH(TBL_FAN_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FAN_FLD_STATUS].name,
            p_fan->status);
    }
    if (FLD_MATCH(TBL_FAN_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FAN_FLD_MODE].name,
            p_fan->mode);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fan_master_t *p_master = _g_p_tbl_fan_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fan_t *p_db_fan = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->fan_list, p_db_fan, listnode, next)
    {
        rc |= fn(p_db_fan, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fan_master_t*
tbl_fan_get_master()
{
    return _g_p_tbl_fan_master;
}

tbl_fan_master_t*
tbl_fan_init_fan()
{
    _g_p_tbl_fan_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fan_master_t));
    _g_p_tbl_fan_master->fan_hash = ctclib_hash_create(_tbl_fan_hash_make, _tbl_fan_hash_cmp);
    _g_p_tbl_fan_master->fan_list = ctclib_slist_create(tbl_fan_cmp, NULL);
    return _g_p_tbl_fan_master;
}


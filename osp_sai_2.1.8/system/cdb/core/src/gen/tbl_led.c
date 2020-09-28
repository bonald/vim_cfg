
#include "proto.h"
#include "gen/tbl_led_define.h"
#include "gen/tbl_led.h"
#include "cdb_data_cmp.h"

tbl_led_master_t *_g_p_tbl_led_master = NULL;

static uint32
_tbl_led_hash_make(tbl_led_t *p_led)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_led->key;
    for (index = 0; index < sizeof(p_led->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_led_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_led_t *p_led1 = (tbl_led_t*)p_arg1;
    tbl_led_t *p_led2 = (tbl_led_t*)p_arg2;

    if (0 == sal_memcmp(&p_led1->key, &p_led2->key, sizeof(tbl_led_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_led_add_led_sync(tbl_led_t *p_led, uint32 sync)
{
    tbl_led_master_t *p_master = _g_p_tbl_led_master;
    tbl_led_t *p_db_led = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_led_get_led(&p_led->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_led = XCALLOC(MEM_TBL_LED, sizeof(tbl_led_t));
    if (NULL == p_db_led)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_led, p_led, sizeof(tbl_led_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->led_hash, (void*)p_db_led, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->led_list, p_db_led);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LED, p_db_led);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LED, p_db_led);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_led_del_led_sync(tbl_led_key_t *p_led_key, uint32 sync)
{
    tbl_led_master_t *p_master = _g_p_tbl_led_master;
    tbl_led_t *p_db_led = NULL;

    /* 1. lookup entry exist */
    p_db_led = tbl_led_get_led(p_led_key);
    if (NULL == p_db_led)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LED, p_db_led);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LED, p_db_led);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->led_hash, (void*)p_db_led);
    ctclib_slistnode_delete(p_master->led_list, p_db_led);

    /* 4. free db entry */
    XFREE(MEM_TBL_LED, p_db_led);

    return PM_E_NONE;
}

int32
tbl_led_set_led_field_sync(tbl_led_t *p_led, tbl_led_field_id_t field_id, uint32 sync)
{
    tbl_led_t *p_db_led = NULL;

    /* 1. lookup entry exist */
    p_db_led = tbl_led_get_led(&p_led->key);
    if (NULL == p_db_led)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LED_FLD_SLOT:
        p_db_led->slot = p_led->slot;
        break;

    case TBL_LED_FLD_NAME:
        sal_memcpy(p_db_led->name, p_led->name, sizeof(p_led->name));
        break;

    case TBL_LED_FLD_STATUS:
        sal_memcpy(p_db_led->status, p_led->status, sizeof(p_led->status));
        break;

    case TBL_LED_FLD_MODE:
        sal_memcpy(p_db_led->mode, p_led->mode, sizeof(p_led->mode));
        break;

    case TBL_LED_FLD_MAX:
        sal_memcpy(p_db_led, p_led, sizeof(tbl_led_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LED, field_id, p_db_led);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LED, field_id, p_db_led);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_led_add_led(tbl_led_t *p_led)
{
    return tbl_led_add_led_sync(p_led, TRUE);
}

int32
tbl_led_del_led(tbl_led_key_t *p_led_key)
{
    return tbl_led_del_led_sync(p_led_key, TRUE);
}

int32
tbl_led_set_led_field(tbl_led_t *p_led, tbl_led_field_id_t field_id)
{
    return tbl_led_set_led_field_sync(p_led, field_id, TRUE);
}

tbl_led_t*
tbl_led_get_led(tbl_led_key_t *p_led_key)
{
    tbl_led_master_t *p_master = _g_p_tbl_led_master;
    tbl_led_t lkp;

    sal_memcpy(&lkp.key, p_led_key, sizeof(tbl_led_key_t));
    return ctclib_hash_lookup(p_master->led_hash, &lkp);
}

char*
tbl_led_key_val2str(tbl_led_t *p_led, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_led->key.id);
    return str;
}

int32
tbl_led_key_str2val(char *str, tbl_led_t *p_led)
{
    int32 ret = 0;

    p_led->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_led_key_name_dump(tbl_led_t *p_led, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LED);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_led->key.id);
    return str;
}

char*
tbl_led_key_value_dump(tbl_led_t *p_led, char *str)
{
    sal_sprintf(str, "%d", p_led->key.id);
    return str;
}

char*
tbl_led_field_name_dump(tbl_led_t *p_led, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LED);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LED_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_led_field_value_dump(tbl_led_t *p_led, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LED_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_led->key.id);
    }
    if (FLD_MATCH(TBL_LED_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_led->slot);
    }
    if (FLD_MATCH(TBL_LED_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_led->name);
    }
    if (FLD_MATCH(TBL_LED_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_led->status);
    }
    if (FLD_MATCH(TBL_LED_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", p_led->mode);
    }
    return str;
}

char**
tbl_led_table_dump(tbl_led_t *p_led, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_led_key_name_dump(p_led, buf));
    for(i=1; i<TBL_LED_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_led_field_name_dump(p_led, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_led_field_value_dump(p_led, i, buf));
    }
    return str;
}

int32
tbl_led_field_value_parser(char *str, int32 field_id, tbl_led_t *p_led)
{
    if (FLD_MATCH(TBL_LED_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_LED_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_LED_FLD_NAME, field_id))
    {
        sal_strcpy(p_led->name, str);
    }
    if (FLD_MATCH(TBL_LED_FLD_STATUS, field_id))
    {
        sal_strcpy(p_led->status, str);
    }
    if (FLD_MATCH(TBL_LED_FLD_MODE, field_id))
    {
        sal_strcpy(p_led->mode, str);
    }
    return PM_E_NONE;
}

int32
tbl_led_table_parser(char** array, char* key_value,tbl_led_t *p_led)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LED);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_led_key_str2val(key_value, p_led));

    for(i=1; i<TBL_LED_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_led_field_value_parser( array[j++], i, p_led));
    }

    return PM_E_NONE;
}

int32
tbl_led_dump_one(tbl_led_t *p_led, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LED);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_LED_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_LED_FLD_KEY].name,
            p_led->key.id);
    }
    if (FLD_MATCH(TBL_LED_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_LED_FLD_SLOT].name,
            p_led->slot);
    }
    if (FLD_MATCH(TBL_LED_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LED_FLD_NAME].name,
            p_led->name);
    }
    if (FLD_MATCH(TBL_LED_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LED_FLD_STATUS].name,
            p_led->status);
    }
    if (FLD_MATCH(TBL_LED_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LED_FLD_MODE].name,
            p_led->mode);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_led_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_led_master_t *p_master = _g_p_tbl_led_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_led_t *p_db_led = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->led_list, p_db_led, listnode, next)
    {
        rc |= fn(p_db_led, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_led_master_t*
tbl_led_get_master()
{
    return _g_p_tbl_led_master;
}

tbl_led_master_t*
tbl_led_init_led()
{
    _g_p_tbl_led_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_led_master_t));
    _g_p_tbl_led_master->led_hash = ctclib_hash_create(_tbl_led_hash_make, _tbl_led_hash_cmp);
    _g_p_tbl_led_master->led_list = ctclib_slist_create(tbl_led_cmp, NULL);
    return _g_p_tbl_led_master;
}


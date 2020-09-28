
#include "proto.h"
#include "gen/tbl_sensor_define.h"
#include "gen/tbl_sensor.h"
#include "cdb_data_cmp.h"

tbl_sensor_master_t *_g_p_tbl_sensor_master = NULL;

static uint32
_tbl_sensor_hash_make(tbl_sensor_t *p_sensor)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_sensor->key;
    for (index = 0; index < sizeof(p_sensor->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_sensor_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_sensor_t *p_sensor1 = (tbl_sensor_t*)p_arg1;
    tbl_sensor_t *p_sensor2 = (tbl_sensor_t*)p_arg2;

    if (0 == sal_memcmp(&p_sensor1->key, &p_sensor2->key, sizeof(tbl_sensor_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_sensor_add_sensor_sync(tbl_sensor_t *p_sensor, uint32 sync)
{
    tbl_sensor_master_t *p_master = _g_p_tbl_sensor_master;
    tbl_sensor_t *p_db_sensor = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_sensor_get_sensor(&p_sensor->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_sensor = XCALLOC(MEM_TBL_SENSOR, sizeof(tbl_sensor_t));
    if (NULL == p_db_sensor)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_sensor, p_sensor, sizeof(tbl_sensor_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->sensor_hash, (void*)p_db_sensor, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->sensor_list, p_db_sensor);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SENSOR, p_db_sensor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SENSOR, p_db_sensor);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sensor_del_sensor_sync(tbl_sensor_key_t *p_sensor_key, uint32 sync)
{
    tbl_sensor_master_t *p_master = _g_p_tbl_sensor_master;
    tbl_sensor_t *p_db_sensor = NULL;

    /* 1. lookup entry exist */
    p_db_sensor = tbl_sensor_get_sensor(p_sensor_key);
    if (NULL == p_db_sensor)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SENSOR, p_db_sensor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SENSOR, p_db_sensor);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->sensor_hash, (void*)p_db_sensor);
    ctclib_slistnode_delete(p_master->sensor_list, p_db_sensor);

    /* 4. free db entry */
    XFREE(MEM_TBL_SENSOR, p_db_sensor);

    return PM_E_NONE;
}

int32
tbl_sensor_set_sensor_field_sync(tbl_sensor_t *p_sensor, tbl_sensor_field_id_t field_id, uint32 sync)
{
    tbl_sensor_t *p_db_sensor = NULL;

    /* 1. lookup entry exist */
    p_db_sensor = tbl_sensor_get_sensor(&p_sensor->key);
    if (NULL == p_db_sensor)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SENSOR_FLD_SLOT:
        p_db_sensor->slot = p_sensor->slot;
        break;

    case TBL_SENSOR_FLD_TEMPERATURE:
        p_db_sensor->temperature = p_sensor->temperature;
        break;

    case TBL_SENSOR_FLD_NAME:
        sal_memcpy(p_db_sensor->name, p_sensor->name, sizeof(p_sensor->name));
        break;

    case TBL_SENSOR_FLD_POSITION:
        sal_memcpy(p_db_sensor->position, p_sensor->position, sizeof(p_sensor->position));
        break;

    case TBL_SENSOR_FLD_STATUS:
        sal_memcpy(p_db_sensor->status, p_sensor->status, sizeof(p_sensor->status));
        break;

    case TBL_SENSOR_FLD_TMPR_LOW:
        p_db_sensor->tmpr_low = p_sensor->tmpr_low;
        break;

    case TBL_SENSOR_FLD_TMPR_HIGH:
        p_db_sensor->tmpr_high = p_sensor->tmpr_high;
        break;

    case TBL_SENSOR_FLD_TMPR_CRITICAL:
        p_db_sensor->tmpr_critical = p_sensor->tmpr_critical;
        break;

    case TBL_SENSOR_FLD_IS_CHIP:
        p_db_sensor->is_chip = p_sensor->is_chip;
        break;

    case TBL_SENSOR_FLD_MAX:
        sal_memcpy(p_db_sensor, p_sensor, sizeof(tbl_sensor_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SENSOR, field_id, p_db_sensor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SENSOR, field_id, p_db_sensor);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sensor_add_sensor(tbl_sensor_t *p_sensor)
{
    return tbl_sensor_add_sensor_sync(p_sensor, TRUE);
}

int32
tbl_sensor_del_sensor(tbl_sensor_key_t *p_sensor_key)
{
    return tbl_sensor_del_sensor_sync(p_sensor_key, TRUE);
}

int32
tbl_sensor_set_sensor_field(tbl_sensor_t *p_sensor, tbl_sensor_field_id_t field_id)
{
    return tbl_sensor_set_sensor_field_sync(p_sensor, field_id, TRUE);
}

tbl_sensor_t*
tbl_sensor_get_sensor(tbl_sensor_key_t *p_sensor_key)
{
    tbl_sensor_master_t *p_master = _g_p_tbl_sensor_master;
    tbl_sensor_t lkp;

    sal_memcpy(&lkp.key, p_sensor_key, sizeof(tbl_sensor_key_t));
    return ctclib_hash_lookup(p_master->sensor_hash, &lkp);
}

char*
tbl_sensor_key_val2str(tbl_sensor_t *p_sensor, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_sensor->key.id);
    return str;
}

int32
tbl_sensor_key_str2val(char *str, tbl_sensor_t *p_sensor)
{
    int32 ret = 0;

    p_sensor->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_sensor_key_name_dump(tbl_sensor_t *p_sensor, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SENSOR);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_sensor->key.id);
    return str;
}

char*
tbl_sensor_key_value_dump(tbl_sensor_t *p_sensor, char *str)
{
    sal_sprintf(str, "%d", p_sensor->key.id);
    return str;
}

char*
tbl_sensor_field_name_dump(tbl_sensor_t *p_sensor, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SENSOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SENSOR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sensor_field_value_dump(tbl_sensor_t *p_sensor, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SENSOR_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->key.id);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->slot);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TEMPERATURE, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->temperature);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_sensor->name);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_POSITION, field_id))
    {
        sal_sprintf(str, "%s", p_sensor->position);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", p_sensor->status);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_LOW, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->tmpr_low);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_HIGH, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->tmpr_high);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_CRITICAL, field_id))
    {
        sal_sprintf(str, "%d", p_sensor->tmpr_critical);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_IS_CHIP, field_id))
    {
        sal_sprintf(str, "%u", p_sensor->is_chip);
    }
    return str;
}

char**
tbl_sensor_table_dump(tbl_sensor_t *p_sensor, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sensor_key_name_dump(p_sensor, buf));
    for(i=1; i<TBL_SENSOR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sensor_field_name_dump(p_sensor, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sensor_field_value_dump(p_sensor, i, buf));
    }
    return str;
}

int32
tbl_sensor_field_value_parser(char *str, int32 field_id, tbl_sensor_t *p_sensor)
{
    if (FLD_MATCH(TBL_SENSOR_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TEMPERATURE, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_NAME, field_id))
    {
        sal_strcpy(p_sensor->name, str);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_POSITION, field_id))
    {
        sal_strcpy(p_sensor->position, str);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_STATUS, field_id))
    {
        sal_strcpy(p_sensor->status, str);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_LOW, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_HIGH, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_CRITICAL, field_id))
    {
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_IS_CHIP, field_id))
    {
        int32 ret;
        p_sensor->is_chip = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_sensor_table_parser(char** array, char* key_value,tbl_sensor_t *p_sensor)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SENSOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sensor_key_str2val(key_value, p_sensor));

    for(i=1; i<TBL_SENSOR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sensor_field_value_parser( array[j++], i, p_sensor));
    }

    return PM_E_NONE;
}

int32
tbl_sensor_dump_one(tbl_sensor_t *p_sensor, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SENSOR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SENSOR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_KEY].name,
            p_sensor->key.id);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_SLOT].name,
            p_sensor->slot);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TEMPERATURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_TEMPERATURE].name,
            p_sensor->temperature);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SENSOR_FLD_NAME].name,
            p_sensor->name);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_POSITION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SENSOR_FLD_POSITION].name,
            p_sensor->position);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SENSOR_FLD_STATUS].name,
            p_sensor->status);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_TMPR_LOW].name,
            p_sensor->tmpr_low);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_TMPR_HIGH].name,
            p_sensor->tmpr_high);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_TMPR_CRITICAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_SENSOR_FLD_TMPR_CRITICAL].name,
            p_sensor->tmpr_critical);
    }
    if (FLD_MATCH(TBL_SENSOR_FLD_IS_CHIP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SENSOR_FLD_IS_CHIP].name,
            p_sensor->is_chip);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sensor_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sensor_master_t *p_master = _g_p_tbl_sensor_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_sensor_t *p_db_sensor = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->sensor_list, p_db_sensor, listnode, next)
    {
        rc |= fn(p_db_sensor, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sensor_master_t*
tbl_sensor_get_master()
{
    return _g_p_tbl_sensor_master;
}

tbl_sensor_master_t*
tbl_sensor_init_sensor()
{
    _g_p_tbl_sensor_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_sensor_master_t));
    _g_p_tbl_sensor_master->sensor_hash = ctclib_hash_create(_tbl_sensor_hash_make, _tbl_sensor_hash_cmp);
    _g_p_tbl_sensor_master->sensor_list = ctclib_slist_create(tbl_sensor_cmp, NULL);
    return _g_p_tbl_sensor_master;
}


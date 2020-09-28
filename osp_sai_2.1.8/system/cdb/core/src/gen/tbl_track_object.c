
#include "proto.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "cdb_data_cmp.h"

tbl_track_object_master_t *_g_p_tbl_track_object_master = NULL;

int32
tbl_track_object_add_track_object_sync(tbl_track_object_t *p_track_object, uint32 sync)
{
    tbl_track_object_master_t *p_master = _g_p_tbl_track_object_master;
    tbl_track_object_t *p_db_track_object = NULL;

    /* 1. lookup entry exist */
    if (tbl_track_object_get_track_object(&p_track_object->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_track_object = XCALLOC(MEM_TBL_TRACK_OBJECT, sizeof(tbl_track_object_t));
    if (NULL == p_db_track_object)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_track_object, p_track_object, sizeof(tbl_track_object_t));

    /* 4. add to db */
    p_master->trackobj_array[p_track_object->key.trackobj_id] = p_db_track_object;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TRACK_OBJECT, p_db_track_object);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TRACK_OBJECT, p_db_track_object);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_track_object_del_track_object_sync(tbl_track_object_key_t *p_track_object_key, uint32 sync)
{
    tbl_track_object_master_t *p_master = _g_p_tbl_track_object_master;
    tbl_track_object_t *p_db_track_object = NULL;

    /* 1. lookup entry exist */
    p_db_track_object = tbl_track_object_get_track_object(p_track_object_key);
    if (NULL == p_db_track_object)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TRACK_OBJECT, p_db_track_object);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TRACK_OBJECT, p_db_track_object);
        #endif 
    }

    /* 3. delete from db */
    p_master->trackobj_array[p_track_object_key->trackobj_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_TRACK_OBJECT, p_db_track_object);

    return PM_E_NONE;
}

int32
tbl_track_object_set_track_object_field_sync(tbl_track_object_t *p_track_object, tbl_track_object_field_id_t field_id, uint32 sync)
{
    tbl_track_object_t *p_db_track_object = NULL;

    /* 1. lookup entry exist */
    p_db_track_object = tbl_track_object_get_track_object(&p_track_object->key);
    if (NULL == p_db_track_object)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TRACK_OBJECT_FLD_TYPE:
        p_db_track_object->type = p_track_object->type;
        break;

    case TBL_TRACK_OBJECT_FLD_STATE:
        p_db_track_object->state = p_track_object->state;
        break;

    case TBL_TRACK_OBJECT_FLD_TRACK_IF:
        sal_memcpy(p_db_track_object->track_if, p_track_object->track_if, sizeof(p_track_object->track_if));
        break;

    case TBL_TRACK_OBJECT_FLD_RTR_ENTRYID:
        p_db_track_object->rtr_entryid = p_track_object->rtr_entryid;
        break;

    case TBL_TRACK_OBJECT_FLD_DELAY_UP:
        p_db_track_object->delay_up = p_track_object->delay_up;
        break;

    case TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP:
        p_db_track_object->running_delay_up = p_track_object->running_delay_up;
        break;

    case TBL_TRACK_OBJECT_FLD_DELAY_DOWN:
        p_db_track_object->delay_down = p_track_object->delay_down;
        break;

    case TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN:
        p_db_track_object->running_delay_down = p_track_object->running_delay_down;
        break;

    case TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP:
        sal_memcpy(p_db_track_object->tracked_vrid_bmp, p_track_object->tracked_vrid_bmp, sizeof(p_track_object->tracked_vrid_bmp));
        break;

    case TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT:
        p_db_track_object->static_rt_refcnt = p_track_object->static_rt_refcnt;
        break;

    case TBL_TRACK_OBJECT_FLD_MAX:
        sal_memcpy(p_db_track_object, p_track_object, sizeof(tbl_track_object_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TRACK_OBJECT, field_id, p_db_track_object);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TRACK_OBJECT, field_id, p_db_track_object);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_track_object_add_track_object(tbl_track_object_t *p_track_object)
{
    return tbl_track_object_add_track_object_sync(p_track_object, TRUE);
}

int32
tbl_track_object_del_track_object(tbl_track_object_key_t *p_track_object_key)
{
    return tbl_track_object_del_track_object_sync(p_track_object_key, TRUE);
}

int32
tbl_track_object_set_track_object_field(tbl_track_object_t *p_track_object, tbl_track_object_field_id_t field_id)
{
    return tbl_track_object_set_track_object_field_sync(p_track_object, field_id, TRUE);
}

tbl_track_object_t*
tbl_track_object_get_track_object(tbl_track_object_key_t *p_track_object_key)
{
    tbl_track_object_master_t *p_master = _g_p_tbl_track_object_master;

    if (GLB_TRACK_OBJECT_MAX+1 <= p_track_object_key->trackobj_id)
    {
        return NULL;
    }
    return (p_master->trackobj_array[p_track_object_key->trackobj_id]);
}

char*
tbl_track_object_key_val2str(tbl_track_object_t *p_track_object, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_track_object->key.trackobj_id);
    return str;
}

int32
tbl_track_object_key_str2val(char *str, tbl_track_object_t *p_track_object)
{
    int32 ret = 0;

    p_track_object->key.trackobj_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_track_object_key_name_dump(tbl_track_object_t *p_track_object, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_OBJECT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_track_object->key.trackobj_id);
    return str;
}

char*
tbl_track_object_key_value_dump(tbl_track_object_t *p_track_object, char *str)
{
    sal_sprintf(str, "%u", p_track_object->key.trackobj_id);
    return str;
}

char*
tbl_track_object_field_name_dump(tbl_track_object_t *p_track_object, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_OBJECT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TRACK_OBJECT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_track_object_field_value_dump(tbl_track_object_t *p_track_object, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->key.trackobj_id);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->type);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->state);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACK_IF, field_id))
    {
        sal_sprintf(str, "%s", p_track_object->track_if);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RTR_ENTRYID, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->rtr_entryid);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_UP, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->delay_up);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->running_delay_up);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_DOWN, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->delay_down);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->running_delay_down);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_track_object->tracked_vrid_bmp, sizeof(p_track_object->tracked_vrid_bmp)/4);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT, field_id))
    {
        sal_sprintf(str, "%u", p_track_object->static_rt_refcnt);
    }
    return str;
}

char**
tbl_track_object_table_dump(tbl_track_object_t *p_track_object, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_track_object_key_name_dump(p_track_object, buf));
    for(i=1; i<TBL_TRACK_OBJECT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_track_object_field_name_dump(p_track_object, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_track_object_field_value_dump(p_track_object, i, buf));
    }
    return str;
}

int32
tbl_track_object_field_value_parser(char *str, int32 field_id, tbl_track_object_t *p_track_object)
{
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_KEY, field_id))
    {
        int32 ret;
        p_track_object->key.trackobj_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TYPE, field_id))
    {
        int32 ret;
        p_track_object->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATE, field_id))
    {
        int32 ret;
        p_track_object->state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACK_IF, field_id))
    {
        sal_strcpy(p_track_object->track_if, str);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RTR_ENTRYID, field_id))
    {
        int32 ret;
        p_track_object->rtr_entryid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_UP, field_id))
    {
        int32 ret;
        p_track_object->delay_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP, field_id))
    {
        int32 ret;
        p_track_object->running_delay_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_DOWN, field_id))
    {
        int32 ret;
        p_track_object->delay_down = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN, field_id))
    {
        int32 ret;
        p_track_object->running_delay_down = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_track_object->tracked_vrid_bmp, sizeof(p_track_object->tracked_vrid_bmp)/4, err);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT, field_id))
    {
        int32 ret;
        p_track_object->static_rt_refcnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_track_object_table_parser(char** array, char* key_value,tbl_track_object_t *p_track_object)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_OBJECT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_track_object_key_str2val(key_value, p_track_object));

    for(i=1; i<TBL_TRACK_OBJECT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_track_object_field_value_parser( array[j++], i, p_track_object));
    }

    return PM_E_NONE;
}

int32
tbl_track_object_dump_one(tbl_track_object_t *p_track_object, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_OBJECT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_KEY].name,
            p_track_object->key.trackobj_id);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_TYPE].name,
            p_track_object->type);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_STATE].name,
            p_track_object->state);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACK_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_TRACK_IF].name,
            p_track_object->track_if);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RTR_ENTRYID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_RTR_ENTRYID].name,
            p_track_object->rtr_entryid);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_DELAY_UP].name,
            p_track_object->delay_up);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_UP].name,
            p_track_object->running_delay_up);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_DELAY_DOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_DELAY_DOWN].name,
            p_track_object->delay_down);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_RUNNING_DELAY_DOWN].name,
            p_track_object->running_delay_down);
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_TRACKED_VRID_BMP].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_track_object->tracked_vrid_bmp, sizeof(p_track_object->tracked_vrid_bmp)/4));
    }
    if (FLD_MATCH(TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT].name,
            p_track_object->static_rt_refcnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_track_object_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_track_object_master_t *p_master = _g_p_tbl_track_object_master;
    tbl_track_object_t *p_db_track_object = NULL;
    uint32 trackobj_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (trackobj_id = 0; trackobj_id < GLB_TRACK_OBJECT_MAX+1; trackobj_id++)
    {
        p_db_track_object = p_master->trackobj_array[trackobj_id];
        if (NULL == p_db_track_object)
        {
            continue;
        }
        rc |= fn(p_db_track_object, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_track_object_master_t*
tbl_track_object_get_master()
{
    return _g_p_tbl_track_object_master;
}

tbl_track_object_master_t*
tbl_track_object_init_track_object()
{
    _g_p_tbl_track_object_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_track_object_master_t));
    return _g_p_tbl_track_object_master;
}


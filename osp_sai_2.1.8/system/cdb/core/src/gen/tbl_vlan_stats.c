
#include "proto.h"
#include "gen/tbl_vlan_stats_define.h"
#include "gen/tbl_vlan_stats.h"
#include "cdb_data_cmp.h"

tbl_vlan_stats_master_t *_g_p_tbl_vlan_stats_master = NULL;

int32
tbl_vlan_stats_add_vlan_stats_sync(tbl_vlan_stats_t *p_vlan_stats, uint32 sync)
{
    tbl_vlan_stats_master_t *p_master = _g_p_tbl_vlan_stats_master;
    tbl_vlan_stats_t *p_db_vlan_stats = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlan_stats_get_vlan_stats(&p_vlan_stats->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan_stats = XCALLOC(MEM_TBL_VLAN_STATS, sizeof(tbl_vlan_stats_t));
    if (NULL == p_db_vlan_stats)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan_stats, p_vlan_stats, sizeof(tbl_vlan_stats_t));

    /* 4. add to db */
    p_master->vlan_stats_array[p_vlan_stats->key.vid] = p_db_vlan_stats;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLAN_STATS, p_db_vlan_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLAN_STATS, p_db_vlan_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_stats_del_vlan_stats_sync(tbl_vlan_stats_key_t *p_vlan_stats_key, uint32 sync)
{
    tbl_vlan_stats_master_t *p_master = _g_p_tbl_vlan_stats_master;
    tbl_vlan_stats_t *p_db_vlan_stats = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_stats = tbl_vlan_stats_get_vlan_stats(p_vlan_stats_key);
    if (NULL == p_db_vlan_stats)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLAN_STATS, p_db_vlan_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLAN_STATS, p_db_vlan_stats);
        #endif 
    }

    /* 3. delete from db */
    p_master->vlan_stats_array[p_vlan_stats_key->vid] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VLAN_STATS, p_db_vlan_stats);

    return PM_E_NONE;
}

int32
tbl_vlan_stats_set_vlan_stats_field_sync(tbl_vlan_stats_t *p_vlan_stats, tbl_vlan_stats_field_id_t field_id, uint32 sync)
{
    tbl_vlan_stats_t *p_db_vlan_stats = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_stats = tbl_vlan_stats_get_vlan_stats(&p_vlan_stats->key);
    if (NULL == p_db_vlan_stats)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLAN_STATS_FLD_PKTCNTL2IN:
        p_db_vlan_stats->pktcntl2in = p_vlan_stats->pktcntl2in;
        break;

    case TBL_VLAN_STATS_FLD_PKTCNTL2OUT:
        p_db_vlan_stats->pktcntl2out = p_vlan_stats->pktcntl2out;
        break;

    case TBL_VLAN_STATS_FLD_PKTCNTL3IN:
        p_db_vlan_stats->pktcntl3in = p_vlan_stats->pktcntl3in;
        break;

    case TBL_VLAN_STATS_FLD_PKTCNTL3OUT:
        p_db_vlan_stats->pktcntl3out = p_vlan_stats->pktcntl3out;
        break;

    case TBL_VLAN_STATS_FLD_PKTCNTACL:
        p_db_vlan_stats->pktcntacl = p_vlan_stats->pktcntacl;
        break;

    case TBL_VLAN_STATS_FLD_BYTECNTL2IN:
        p_db_vlan_stats->bytecntl2in = p_vlan_stats->bytecntl2in;
        break;

    case TBL_VLAN_STATS_FLD_BYTECNTL2OUT:
        p_db_vlan_stats->bytecntl2out = p_vlan_stats->bytecntl2out;
        break;

    case TBL_VLAN_STATS_FLD_BYTECNTL3IN:
        p_db_vlan_stats->bytecntl3in = p_vlan_stats->bytecntl3in;
        break;

    case TBL_VLAN_STATS_FLD_BYTECNTL3OUT:
        p_db_vlan_stats->bytecntl3out = p_vlan_stats->bytecntl3out;
        break;

    case TBL_VLAN_STATS_FLD_BYTECNTACL:
        p_db_vlan_stats->bytecntacl = p_vlan_stats->bytecntacl;
        break;

    case TBL_VLAN_STATS_FLD_MAX:
        sal_memcpy(p_db_vlan_stats, p_vlan_stats, sizeof(tbl_vlan_stats_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLAN_STATS, field_id, p_db_vlan_stats);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLAN_STATS, field_id, p_db_vlan_stats);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_stats_add_vlan_stats(tbl_vlan_stats_t *p_vlan_stats)
{
    return tbl_vlan_stats_add_vlan_stats_sync(p_vlan_stats, TRUE);
}

int32
tbl_vlan_stats_del_vlan_stats(tbl_vlan_stats_key_t *p_vlan_stats_key)
{
    return tbl_vlan_stats_del_vlan_stats_sync(p_vlan_stats_key, TRUE);
}

int32
tbl_vlan_stats_set_vlan_stats_field(tbl_vlan_stats_t *p_vlan_stats, tbl_vlan_stats_field_id_t field_id)
{
    return tbl_vlan_stats_set_vlan_stats_field_sync(p_vlan_stats, field_id, TRUE);
}

tbl_vlan_stats_t*
tbl_vlan_stats_get_vlan_stats(tbl_vlan_stats_key_t *p_vlan_stats_key)
{
    tbl_vlan_stats_master_t *p_master = _g_p_tbl_vlan_stats_master;

    if (GLB_MAX_VLAN_ID <= p_vlan_stats_key->vid)
    {
        return NULL;
    }
    return (p_master->vlan_stats_array[p_vlan_stats_key->vid]);
}

char*
tbl_vlan_stats_key_val2str(tbl_vlan_stats_t *p_vlan_stats, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlan_stats->key.vid);
    return str;
}

int32
tbl_vlan_stats_key_str2val(char *str, tbl_vlan_stats_t *p_vlan_stats)
{
    int32 ret = 0;

    p_vlan_stats->key.vid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vlan_stats_key_name_dump(tbl_vlan_stats_t *p_vlan_stats, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_STATS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vlan_stats->key.vid);
    return str;
}

char*
tbl_vlan_stats_key_value_dump(tbl_vlan_stats_t *p_vlan_stats, char *str)
{
    sal_sprintf(str, "%u", p_vlan_stats->key.vid);
    return str;
}

char*
tbl_vlan_stats_field_name_dump(tbl_vlan_stats_t *p_vlan_stats, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLAN_STATS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlan_stats_field_value_dump(tbl_vlan_stats_t *p_vlan_stats, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_stats->key.vid);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2IN, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->pktcntl2in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->pktcntl2out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3IN, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->pktcntl3in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->pktcntl3out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTACL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->pktcntacl);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2IN, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->bytecntl2in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->bytecntl2out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3IN, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->bytecntl3in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3OUT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->bytecntl3out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTACL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan_stats->bytecntacl);
    }
    return str;
}

char**
tbl_vlan_stats_table_dump(tbl_vlan_stats_t *p_vlan_stats, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlan_stats_key_name_dump(p_vlan_stats, buf));
    for(i=1; i<TBL_VLAN_STATS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlan_stats_field_name_dump(p_vlan_stats, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlan_stats_field_value_dump(p_vlan_stats, i, buf));
    }
    return str;
}

int32
tbl_vlan_stats_field_value_parser(char *str, int32 field_id, tbl_vlan_stats_t *p_vlan_stats)
{
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_KEY, field_id))
    {
        int32 ret;
        p_vlan_stats->key.vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2IN, field_id))
    {
        int32 ret;
        p_vlan_stats->pktcntl2in = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2OUT, field_id))
    {
        int32 ret;
        p_vlan_stats->pktcntl2out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3IN, field_id))
    {
        int32 ret;
        p_vlan_stats->pktcntl3in = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3OUT, field_id))
    {
        int32 ret;
        p_vlan_stats->pktcntl3out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTACL, field_id))
    {
        int32 ret;
        p_vlan_stats->pktcntacl = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2IN, field_id))
    {
        int32 ret;
        p_vlan_stats->bytecntl2in = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2OUT, field_id))
    {
        int32 ret;
        p_vlan_stats->bytecntl2out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3IN, field_id))
    {
        int32 ret;
        p_vlan_stats->bytecntl3in = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3OUT, field_id))
    {
        int32 ret;
        p_vlan_stats->bytecntl3out = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTACL, field_id))
    {
        int32 ret;
        p_vlan_stats->bytecntacl = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vlan_stats_table_parser(char** array, char* key_value,tbl_vlan_stats_t *p_vlan_stats)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_STATS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlan_stats_key_str2val(key_value, p_vlan_stats));

    for(i=1; i<TBL_VLAN_STATS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlan_stats_field_value_parser( array[j++], i, p_vlan_stats));
    }

    return PM_E_NONE;
}

int32
tbl_vlan_stats_dump_one(tbl_vlan_stats_t *p_vlan_stats, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_STATS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_VLAN_STATS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_STATS_FLD_KEY].name,
            p_vlan_stats->key.vid);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_PKTCNTL2IN].name,
            p_vlan_stats->pktcntl2in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL2OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_PKTCNTL2OUT].name,
            p_vlan_stats->pktcntl2out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_PKTCNTL3IN].name,
            p_vlan_stats->pktcntl3in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTL3OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_PKTCNTL3OUT].name,
            p_vlan_stats->pktcntl3out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_PKTCNTACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_PKTCNTACL].name,
            p_vlan_stats->pktcntacl);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_BYTECNTL2IN].name,
            p_vlan_stats->bytecntl2in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL2OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_BYTECNTL2OUT].name,
            p_vlan_stats->bytecntl2out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_BYTECNTL3IN].name,
            p_vlan_stats->bytecntl3in);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTL3OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_BYTECNTL3OUT].name,
            p_vlan_stats->bytecntl3out);
    }
    if (FLD_MATCH(TBL_VLAN_STATS_FLD_BYTECNTACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_STATS_FLD_BYTECNTACL].name,
            p_vlan_stats->bytecntacl);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlan_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlan_stats_master_t *p_master = _g_p_tbl_vlan_stats_master;
    tbl_vlan_stats_t *p_db_vlan_stats = NULL;
    uint32 vid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan_stats = p_master->vlan_stats_array[vid];
        if (NULL == p_db_vlan_stats)
        {
            continue;
        }
        rc |= fn(p_db_vlan_stats, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlan_stats_master_t*
tbl_vlan_stats_get_master()
{
    return _g_p_tbl_vlan_stats_master;
}

tbl_vlan_stats_master_t*
tbl_vlan_stats_init_vlan_stats()
{
    _g_p_tbl_vlan_stats_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlan_stats_master_t));
    return _g_p_tbl_vlan_stats_master;
}


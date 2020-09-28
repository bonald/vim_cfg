
#include "proto.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "cdb_data_cmp.h"

tbl_dhcsrvgrp_master_t *_g_p_tbl_dhcsrvgrp_master = NULL;

int32
tbl_dhcsrvgrp_add_dhcsrvgrp_sync(tbl_dhcsrvgrp_t *p_dhcsrvgrp, uint32 sync)
{
    tbl_dhcsrvgrp_master_t *p_master = _g_p_tbl_dhcsrvgrp_master;
    tbl_dhcsrvgrp_t *p_db_dhcsrvgrp = NULL;

    /* 1. lookup entry exist */
    if (tbl_dhcsrvgrp_get_dhcsrvgrp(&p_dhcsrvgrp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dhcsrvgrp = XCALLOC(MEM_TBL_DHCSRVGRP, sizeof(tbl_dhcsrvgrp_t));
    if (NULL == p_db_dhcsrvgrp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dhcsrvgrp, p_dhcsrvgrp, sizeof(tbl_dhcsrvgrp_t));

    /* 4. add to db */
    p_master->dhcsrvgrp_array[p_dhcsrvgrp->key.id] = p_db_dhcsrvgrp;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DHCSRVGRP, p_db_dhcsrvgrp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DHCSRVGRP, p_db_dhcsrvgrp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_del_dhcsrvgrp_sync(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key, uint32 sync)
{
    tbl_dhcsrvgrp_master_t *p_master = _g_p_tbl_dhcsrvgrp_master;
    tbl_dhcsrvgrp_t *p_db_dhcsrvgrp = NULL;

    /* 1. lookup entry exist */
    p_db_dhcsrvgrp = tbl_dhcsrvgrp_get_dhcsrvgrp(p_dhcsrvgrp_key);
    if (NULL == p_db_dhcsrvgrp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DHCSRVGRP, p_db_dhcsrvgrp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DHCSRVGRP, p_db_dhcsrvgrp);
        #endif 
    }

    /* 3. delete from db */
    p_master->dhcsrvgrp_array[p_dhcsrvgrp_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_DHCSRVGRP, p_db_dhcsrvgrp);

    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_set_dhcsrvgrp_field_sync(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_dhcsrvgrp_field_id_t field_id, uint32 sync)
{
    tbl_dhcsrvgrp_t *p_db_dhcsrvgrp = NULL;

    /* 1. lookup entry exist */
    p_db_dhcsrvgrp = tbl_dhcsrvgrp_get_dhcsrvgrp(&p_dhcsrvgrp->key);
    if (NULL == p_db_dhcsrvgrp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCSRVGRP_FLD_CNT:
        p_db_dhcsrvgrp->cnt = p_dhcsrvgrp->cnt;
        break;

    case TBL_DHCSRVGRP_FLD_ADDRS:
        sal_memcpy(p_db_dhcsrvgrp->addrs, p_dhcsrvgrp->addrs, sizeof(p_dhcsrvgrp->addrs));
        break;

    case TBL_DHCSRVGRP_FLD_MAX:
        sal_memcpy(p_db_dhcsrvgrp, p_dhcsrvgrp, sizeof(tbl_dhcsrvgrp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCSRVGRP, field_id, p_db_dhcsrvgrp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCSRVGRP, field_id, p_db_dhcsrvgrp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_add_dhcsrvgrp(tbl_dhcsrvgrp_t *p_dhcsrvgrp)
{
    return tbl_dhcsrvgrp_add_dhcsrvgrp_sync(p_dhcsrvgrp, TRUE);
}

int32
tbl_dhcsrvgrp_del_dhcsrvgrp(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key)
{
    return tbl_dhcsrvgrp_del_dhcsrvgrp_sync(p_dhcsrvgrp_key, TRUE);
}

int32
tbl_dhcsrvgrp_set_dhcsrvgrp_field(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_dhcsrvgrp_field_id_t field_id)
{
    return tbl_dhcsrvgrp_set_dhcsrvgrp_field_sync(p_dhcsrvgrp, field_id, TRUE);
}

tbl_dhcsrvgrp_t*
tbl_dhcsrvgrp_get_dhcsrvgrp(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key)
{
    tbl_dhcsrvgrp_master_t *p_master = _g_p_tbl_dhcsrvgrp_master;

    if (GLB_MAX_DHCSRVGRP_NUM <= p_dhcsrvgrp_key->id)
    {
        return NULL;
    }
    return (p_master->dhcsrvgrp_array[p_dhcsrvgrp_key->id]);
}

char*
tbl_dhcsrvgrp_key_val2str(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_dhcsrvgrp->key.id);
    return str;
}

int32
tbl_dhcsrvgrp_key_str2val(char *str, tbl_dhcsrvgrp_t *p_dhcsrvgrp)
{
    int32 ret = 0;

    p_dhcsrvgrp->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_dhcsrvgrp_key_name_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSRVGRP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_dhcsrvgrp->key.id);
    return str;
}

char*
tbl_dhcsrvgrp_key_value_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char *str)
{
    sal_sprintf(str, "%u", p_dhcsrvgrp->key.id);
    return str;
}

char*
tbl_dhcsrvgrp_field_name_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSRVGRP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCSRVGRP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhcsrvgrp_field_value_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsrvgrp->key.id);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_dhcsrvgrp->cnt);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_ADDRS, field_id))
    {
        sal_sprintf(str, "%s", p_dhcsrvgrp->addrs);
    }
    return str;
}

char**
tbl_dhcsrvgrp_table_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhcsrvgrp_key_name_dump(p_dhcsrvgrp, buf));
    for(i=1; i<TBL_DHCSRVGRP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhcsrvgrp_field_name_dump(p_dhcsrvgrp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhcsrvgrp_field_value_dump(p_dhcsrvgrp, i, buf));
    }
    return str;
}

int32
tbl_dhcsrvgrp_field_value_parser(char *str, int32 field_id, tbl_dhcsrvgrp_t *p_dhcsrvgrp)
{
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_KEY, field_id))
    {
        int32 ret;
        p_dhcsrvgrp->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_CNT, field_id))
    {
        int32 ret;
        p_dhcsrvgrp->cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_ADDRS, field_id))
    {
        sal_strcpy(p_dhcsrvgrp->addrs, str);
    }
    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_table_parser(char** array, char* key_value,tbl_dhcsrvgrp_t *p_dhcsrvgrp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSRVGRP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhcsrvgrp_key_str2val(key_value, p_dhcsrvgrp));

    for(i=1; i<TBL_DHCSRVGRP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhcsrvgrp_field_value_parser( array[j++], i, p_dhcsrvgrp));
    }

    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_dump_one(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCSRVGRP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSRVGRP_FLD_KEY].name,
            p_dhcsrvgrp->key.id);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCSRVGRP_FLD_CNT].name,
            p_dhcsrvgrp->cnt);
    }
    if (FLD_MATCH(TBL_DHCSRVGRP_FLD_ADDRS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCSRVGRP_FLD_ADDRS].name,
            p_dhcsrvgrp->addrs);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhcsrvgrp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhcsrvgrp_master_t *p_master = _g_p_tbl_dhcsrvgrp_master;
    tbl_dhcsrvgrp_t *p_db_dhcsrvgrp = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_MAX_DHCSRVGRP_NUM; id++)
    {
        p_db_dhcsrvgrp = p_master->dhcsrvgrp_array[id];
        if (NULL == p_db_dhcsrvgrp)
        {
            continue;
        }
        rc |= fn(p_db_dhcsrvgrp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhcsrvgrp_master_t*
tbl_dhcsrvgrp_get_master()
{
    return _g_p_tbl_dhcsrvgrp_master;
}

tbl_dhcsrvgrp_master_t*
tbl_dhcsrvgrp_init_dhcsrvgrp()
{
    _g_p_tbl_dhcsrvgrp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dhcsrvgrp_master_t));
    return _g_p_tbl_dhcsrvgrp_master;
}


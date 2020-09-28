
#include "proto.h"
#include "gen/tbl_fea_port_if_define.h"
#include "gen/tbl_fea_port_if.h"
#include "cdb_data_cmp.h"

tbl_fea_port_if_master_t *_g_p_tbl_fea_port_if_master = NULL;

static uint32
_tbl_fea_port_if_hash_make(tbl_fea_port_if_t *p_portif)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_portif->key;
    for (index = 0; index < sizeof(p_portif->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_port_if_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_port_if_t *p_portif1 = (tbl_fea_port_if_t*)p_arg1;
    tbl_fea_port_if_t *p_portif2 = (tbl_fea_port_if_t*)p_arg2;

    if (0 == sal_memcmp(&p_portif1->key, &p_portif2->key, sizeof(tbl_fea_port_if_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_port_if_add_fea_port_if_sync(tbl_fea_port_if_t *p_portif, uint32 sync)
{
    tbl_fea_port_if_master_t *p_master = _g_p_tbl_fea_port_if_master;
    tbl_fea_port_if_t *p_db_portif = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_port_if_get_fea_port_if(&p_portif->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_portif = XCALLOC(MEM_TBL_FEA_PORT_IF, sizeof(tbl_fea_port_if_t));
    if (NULL == p_db_portif)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_portif, p_portif, sizeof(tbl_fea_port_if_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->portif_hash, (void*)p_db_portif, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->portif_list, p_db_portif);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_PORT_IF, p_db_portif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_PORT_IF, p_db_portif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_if_del_fea_port_if_sync(tbl_fea_port_if_key_t *p_portif_key, uint32 sync)
{
    tbl_fea_port_if_master_t *p_master = _g_p_tbl_fea_port_if_master;
    tbl_fea_port_if_t *p_db_portif = NULL;

    /* 1. lookup entry exist */
    p_db_portif = tbl_fea_port_if_get_fea_port_if(p_portif_key);
    if (NULL == p_db_portif)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_PORT_IF, p_db_portif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_PORT_IF, p_db_portif);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->portif_hash, (void*)p_db_portif);
    ctclib_slistnode_delete(p_master->portif_list, p_db_portif);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_PORT_IF, p_db_portif);

    return PM_E_NONE;
}

int32
tbl_fea_port_if_set_fea_port_if_field_sync(tbl_fea_port_if_t *p_portif, tbl_fea_port_if_field_id_t field_id, uint32 sync)
{
    tbl_fea_port_if_t *p_db_portif = NULL;

    /* 1. lookup entry exist */
    p_db_portif = tbl_fea_port_if_get_fea_port_if(&p_portif->key);
    if (NULL == p_db_portif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_PORT_IF_FLD_IFINDEX:
        p_db_portif->ifindex = p_portif->ifindex;
        break;

    case TBL_FEA_PORT_IF_FLD_NAME:
        sal_memcpy(p_db_portif->name, p_portif->name, sizeof(p_portif->name));
        break;

    case TBL_FEA_PORT_IF_FLD_LAG_MEMBER:
        p_db_portif->lag_member = p_portif->lag_member;
        break;

    case TBL_FEA_PORT_IF_FLD_MAX:
        sal_memcpy(p_db_portif, p_portif, sizeof(tbl_fea_port_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_PORT_IF, field_id, p_db_portif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_PORT_IF, field_id, p_db_portif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_if_add_fea_port_if(tbl_fea_port_if_t *p_portif)
{
    return tbl_fea_port_if_add_fea_port_if_sync(p_portif, TRUE);
}

int32
tbl_fea_port_if_del_fea_port_if(tbl_fea_port_if_key_t *p_portif_key)
{
    return tbl_fea_port_if_del_fea_port_if_sync(p_portif_key, TRUE);
}

int32
tbl_fea_port_if_set_fea_port_if_field(tbl_fea_port_if_t *p_portif, tbl_fea_port_if_field_id_t field_id)
{
    return tbl_fea_port_if_set_fea_port_if_field_sync(p_portif, field_id, TRUE);
}

tbl_fea_port_if_t*
tbl_fea_port_if_get_fea_port_if(tbl_fea_port_if_key_t *p_portif_key)
{
    tbl_fea_port_if_master_t *p_master = _g_p_tbl_fea_port_if_master;
    tbl_fea_port_if_t lkp;

    sal_memcpy(&lkp.key, p_portif_key, sizeof(tbl_fea_port_if_key_t));
    return ctclib_hash_lookup(p_master->portif_hash, &lkp);
}

char*
tbl_fea_port_if_key_val2str(tbl_fea_port_if_t *p_portif, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%"PRIu64, p_portif->key.portid);
    return str;
}

int32
tbl_fea_port_if_key_str2val(char *str, tbl_fea_port_if_t *p_portif)
{
    int32 ret = 0;

    p_portif->key.portid = cdb_uint64_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_port_if_key_name_dump(tbl_fea_port_if_t *p_portif, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%"PRIu64, p_node->name, p_portif->key.portid);
    return str;
}

char*
tbl_fea_port_if_key_value_dump(tbl_fea_port_if_t *p_portif, char *str)
{
    sal_sprintf(str, "%"PRIu64, p_portif->key.portid);
    return str;
}

char*
tbl_fea_port_if_field_name_dump(tbl_fea_port_if_t *p_portif, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_PORT_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_port_if_field_value_dump(tbl_fea_port_if_t *p_portif, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_portif->key.portid);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_portif->ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_portif->name);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_LAG_MEMBER, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_portif->lag_member);
    }
    return str;
}

char**
tbl_fea_port_if_table_dump(tbl_fea_port_if_t *p_portif, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_port_if_key_name_dump(p_portif, buf));
    for(i=1; i<TBL_FEA_PORT_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_port_if_field_name_dump(p_portif, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_port_if_field_value_dump(p_portif, i, buf));
    }
    return str;
}

int32
tbl_fea_port_if_field_value_parser(char *str, int32 field_id, tbl_fea_port_if_t *p_portif)
{
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_KEY, field_id))
    {
        int32 ret;
        p_portif->key.portid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_portif->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_NAME, field_id))
    {
        sal_strcpy(p_portif->name, str);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_LAG_MEMBER, field_id))
    {
        int32 ret;
        p_portif->lag_member = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_port_if_table_parser(char** array, char* key_value,tbl_fea_port_if_t *p_portif)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_port_if_key_str2val(key_value, p_portif));

    for(i=1; i<TBL_FEA_PORT_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_port_if_field_value_parser( array[j++], i, p_portif));
    }

    return PM_E_NONE;
}

int32
tbl_fea_port_if_dump_one(tbl_fea_port_if_t *p_portif, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PORT_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_PORT_IF_FLD_KEY].name,
            p_portif->key.portid);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PORT_IF_FLD_IFINDEX].name,
            p_portif->ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_PORT_IF_FLD_NAME].name,
            p_portif->name);
    }
    if (FLD_MATCH(TBL_FEA_PORT_IF_FLD_LAG_MEMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_PORT_IF_FLD_LAG_MEMBER].name,
            p_portif->lag_member);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_port_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_port_if_master_t *p_master = _g_p_tbl_fea_port_if_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fea_port_if_t *p_db_portif = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->portif_list, p_db_portif, listnode, next)
    {
        rc |= fn(p_db_portif, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_port_if_master_t*
tbl_fea_port_if_get_master()
{
    return _g_p_tbl_fea_port_if_master;
}

tbl_fea_port_if_master_t*
tbl_fea_port_if_init_fea_port_if()
{
    _g_p_tbl_fea_port_if_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_port_if_master_t));
    _g_p_tbl_fea_port_if_master->portif_hash = ctclib_hash_create(_tbl_fea_port_if_hash_make, _tbl_fea_port_if_hash_cmp);
    _g_p_tbl_fea_port_if_master->portif_list = ctclib_slist_create(tbl_fea_port_if_cmp, NULL);
    return _g_p_tbl_fea_port_if_master;
}


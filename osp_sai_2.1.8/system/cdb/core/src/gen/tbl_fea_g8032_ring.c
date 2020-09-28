
#include "proto.h"
#include "gen/tbl_fea_g8032_ring_define.h"
#include "gen/tbl_fea_g8032_ring.h"
#include "cdb_data_cmp.h"

tbl_fea_g8032_ring_master_t *_g_p_tbl_fea_g8032_ring_master = NULL;

int32
tbl_fea_g8032_ring_add_fea_g8032_ring_sync(tbl_fea_g8032_ring_t *p_fea_g8032_ring, uint32 sync)
{
    tbl_fea_g8032_ring_master_t *p_master = _g_p_tbl_fea_g8032_ring_master;
    tbl_fea_g8032_ring_t *p_db_fea_g8032_ring = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_g8032_ring_get_fea_g8032_ring(&p_fea_g8032_ring->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_g8032_ring = XCALLOC(MEM_TBL_FEA_G8032_RING, sizeof(tbl_fea_g8032_ring_t));
    if (NULL == p_db_fea_g8032_ring)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_g8032_ring, p_fea_g8032_ring, sizeof(tbl_fea_g8032_ring_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->fea_g8032_list, p_db_fea_g8032_ring);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_G8032_RING, p_db_fea_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_G8032_RING, p_db_fea_g8032_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_del_fea_g8032_ring_sync(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key, uint32 sync)
{
    tbl_fea_g8032_ring_master_t *p_master = _g_p_tbl_fea_g8032_ring_master;
    tbl_fea_g8032_ring_t *p_db_fea_g8032_ring = NULL;

    /* 1. lookup entry exist */
    p_db_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(p_fea_g8032_ring_key);
    if (NULL == p_db_fea_g8032_ring)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_G8032_RING, p_db_fea_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_G8032_RING, p_db_fea_g8032_ring);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->fea_g8032_list, p_db_fea_g8032_ring);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_G8032_RING, p_db_fea_g8032_ring);

    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_set_fea_g8032_ring_field_sync(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_fea_g8032_ring_field_id_t field_id, uint32 sync)
{
    tbl_fea_g8032_ring_t *p_db_fea_g8032_ring = NULL;

    /* 1. lookup entry exist */
    p_db_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(&p_fea_g8032_ring->key);
    if (NULL == p_db_fea_g8032_ring)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_G8032_RING_FLD_MC_GROUP_ID:
        p_db_fea_g8032_ring->mc_group_id = p_fea_g8032_ring->mc_group_id;
        break;

    case TBL_FEA_G8032_RING_FLD_APS_GROUP_ID:
        p_db_fea_g8032_ring->aps_group_id = p_fea_g8032_ring->aps_group_id;
        break;

    case TBL_FEA_G8032_RING_FLD_NH_ID:
        p_db_fea_g8032_ring->nh_id = p_fea_g8032_ring->nh_id;
        break;

    case TBL_FEA_G8032_RING_FLD_NHOFFSET:
        p_db_fea_g8032_ring->nhoffset = p_fea_g8032_ring->nhoffset;
        break;

    case TBL_FEA_G8032_RING_FLD_MAX:
        sal_memcpy(p_db_fea_g8032_ring, p_fea_g8032_ring, sizeof(tbl_fea_g8032_ring_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_G8032_RING, field_id, p_db_fea_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_G8032_RING, field_id, p_db_fea_g8032_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_add_fea_g8032_ring(tbl_fea_g8032_ring_t *p_fea_g8032_ring)
{
    return tbl_fea_g8032_ring_add_fea_g8032_ring_sync(p_fea_g8032_ring, TRUE);
}

int32
tbl_fea_g8032_ring_del_fea_g8032_ring(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key)
{
    return tbl_fea_g8032_ring_del_fea_g8032_ring_sync(p_fea_g8032_ring_key, TRUE);
}

int32
tbl_fea_g8032_ring_set_fea_g8032_ring_field(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_fea_g8032_ring_field_id_t field_id)
{
    return tbl_fea_g8032_ring_set_fea_g8032_ring_field_sync(p_fea_g8032_ring, field_id, TRUE);
}

tbl_fea_g8032_ring_t*
tbl_fea_g8032_ring_get_fea_g8032_ring(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key)
{
    tbl_fea_g8032_ring_master_t *p_master = _g_p_tbl_fea_g8032_ring_master;
    tbl_fea_g8032_ring_t lkp;

    sal_memcpy(&lkp.key, p_fea_g8032_ring_key, sizeof(tbl_fea_g8032_ring_key_t));
    return ctclib_slistdata_lookup(p_master->fea_g8032_list, &lkp);
}

char*
tbl_fea_g8032_ring_key_val2str(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_g8032_fea_ring_val2str(buf, MAX_CMD_STR_LEN, &p_fea_g8032_ring->key));
    return str;
}

int32
tbl_fea_g8032_ring_key_str2val(char *str, tbl_fea_g8032_ring_t *p_fea_g8032_ring)
{
    int32 ret = 0;

    ret = cdb_g8032_fea_ring_str2val(str, &p_fea_g8032_ring->key);
    return ret;
}

char*
tbl_fea_g8032_ring_key_name_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_G8032_RING);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_g8032_fea_ring_val2str(buf, MAX_CMD_STR_LEN, &p_fea_g8032_ring->key));
    return str;
}

char*
tbl_fea_g8032_ring_key_value_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char *str)
{

    cdb_g8032_fea_ring_val2str(str, MAX_CMD_STR_LEN, &p_fea_g8032_ring->key);
    return str;
}

char*
tbl_fea_g8032_ring_field_name_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_G8032_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_G8032_RING_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_g8032_ring_field_value_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_KEY, field_id))
    {
        cdb_g8032_fea_ring_val2str(str, MAX_CMD_STR_LEN, &p_fea_g8032_ring->key);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_MC_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_fea_g8032_ring->mc_group_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_APS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_fea_g8032_ring->aps_group_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NH_ID, field_id))
    {
        sal_sprintf(str, "%u", p_fea_g8032_ring->nh_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NHOFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_fea_g8032_ring->nhoffset);
    }
    return str;
}

char**
tbl_fea_g8032_ring_table_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_g8032_ring_key_name_dump(p_fea_g8032_ring, buf));
    for(i=1; i<TBL_FEA_G8032_RING_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_g8032_ring_field_name_dump(p_fea_g8032_ring, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_g8032_ring_field_value_dump(p_fea_g8032_ring, i, buf));
    }
    return str;
}

int32
tbl_fea_g8032_ring_field_value_parser(char *str, int32 field_id, tbl_fea_g8032_ring_t *p_fea_g8032_ring)
{
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_KEY, field_id))
    {
        cdb_g8032_fea_ring_str2val(str, &p_fea_g8032_ring->key);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_MC_GROUP_ID, field_id))
    {
        int32 ret;
        p_fea_g8032_ring->mc_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_APS_GROUP_ID, field_id))
    {
        int32 ret;
        p_fea_g8032_ring->aps_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NH_ID, field_id))
    {
        int32 ret;
        p_fea_g8032_ring->nh_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NHOFFSET, field_id))
    {
        int32 ret;
        p_fea_g8032_ring->nhoffset = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_table_parser(char** array, char* key_value,tbl_fea_g8032_ring_t *p_fea_g8032_ring)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_G8032_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_g8032_ring_key_str2val(key_value, p_fea_g8032_ring));

    for(i=1; i<TBL_FEA_G8032_RING_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_g8032_ring_field_value_parser( array[j++], i, p_fea_g8032_ring));
    }

    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_dump_one(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_G8032_RING);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_G8032_RING_FLD_KEY].name, 
            cdb_g8032_fea_ring_val2str(buf, MAX_CMD_STR_LEN, &p_fea_g8032_ring->key));
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_MC_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_G8032_RING_FLD_MC_GROUP_ID].name,
            p_fea_g8032_ring->mc_group_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_APS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_G8032_RING_FLD_APS_GROUP_ID].name,
            p_fea_g8032_ring->aps_group_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NH_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_G8032_RING_FLD_NH_ID].name,
            p_fea_g8032_ring->nh_id);
    }
    if (FLD_MATCH(TBL_FEA_G8032_RING_FLD_NHOFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_G8032_RING_FLD_NHOFFSET].name,
            p_fea_g8032_ring->nhoffset);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_g8032_ring_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_g8032_ring_master_t *p_master = _g_p_tbl_fea_g8032_ring_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_fea_g8032_ring_t *p_db_fea_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->fea_g8032_list, p_db_fea_g8032_ring, listnode, next)
    {
        rc |= fn(p_db_fea_g8032_ring, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_g8032_ring_master_t*
tbl_fea_g8032_ring_get_master()
{
    return _g_p_tbl_fea_g8032_ring_master;
}

tbl_fea_g8032_ring_master_t*
tbl_fea_g8032_ring_init_fea_g8032_ring()
{
    _g_p_tbl_fea_g8032_ring_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_g8032_ring_master_t));
    _g_p_tbl_fea_g8032_ring_master->fea_g8032_list = ctclib_slist_create(tbl_fea_g8032_ring_cmp, NULL);
    return _g_p_tbl_fea_g8032_ring_master;
}


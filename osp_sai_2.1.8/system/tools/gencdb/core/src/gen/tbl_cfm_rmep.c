
#include "proto.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#include "cdb_data_cmp.h"

tbl_cfm_rmep_master_t *_g_p_tbl_cfm_rmep_master = NULL;

static uint32
_tbl_cfm_rmep_hash_make(tbl_cfm_rmep_t *p_rmep)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rmep->key;
    for (index = 0; index < sizeof(p_rmep->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cfm_rmep_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cfm_rmep_t *p_rmep1 = (tbl_cfm_rmep_t*)p_arg1;
    tbl_cfm_rmep_t *p_rmep2 = (tbl_cfm_rmep_t*)p_arg2;

    if (0 == sal_memcmp(&p_rmep1->key, &p_rmep2->key, sizeof(tbl_cfm_rmep_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cfm_rmep_add_cfm_rmep_sync(tbl_cfm_rmep_t *p_rmep, uint32 sync)
{
    tbl_cfm_rmep_master_t *p_master = _g_p_tbl_cfm_rmep_master;
    tbl_cfm_rmep_t *p_db_rmep = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cfm_rmep_get_cfm_rmep(&p_rmep->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rmep = XCALLOC(MEM_TBL_CFM_RMEP, sizeof(tbl_cfm_rmep_t));
    if (NULL == p_db_rmep)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rmep, p_rmep, sizeof(tbl_cfm_rmep_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_rmep_hash, (void*)p_db_rmep, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_rmep_list, p_db_rmep);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CFM_RMEP, p_db_rmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CFM_RMEP, p_db_rmep);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_rmep_del_cfm_rmep_sync(tbl_cfm_rmep_key_t *p_rmep_key, uint32 sync)
{
    tbl_cfm_rmep_master_t *p_master = _g_p_tbl_cfm_rmep_master;
    tbl_cfm_rmep_t *p_db_rmep = NULL;

    /* 1. lookup entry exist */
    p_db_rmep = tbl_cfm_rmep_get_cfm_rmep(p_rmep_key);
    if (NULL == p_db_rmep)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CFM_RMEP, p_db_rmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CFM_RMEP, p_db_rmep);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_rmep_hash, (void*)p_db_rmep);
    ctclib_slistnode_delete(p_master->cfm_rmep_list, p_db_rmep);

    /* 4. free db entry */
    XFREE(MEM_TBL_CFM_RMEP, p_db_rmep);

    return PM_E_NONE;
}

int32
tbl_cfm_rmep_set_cfm_rmep_field_sync(tbl_cfm_rmep_t *p_rmep, tbl_cfm_rmep_field_id_t field_id, uint32 sync)
{
    tbl_cfm_rmep_t *p_db_rmep = NULL;

    /* 1. lookup entry exist */
    p_db_rmep = tbl_cfm_rmep_get_cfm_rmep(&p_rmep->key);
    if (NULL == p_db_rmep)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_RMEP_FLD_IFINDEX:
        p_db_rmep->ifindex = p_rmep->ifindex;
        break;

    case TBL_CFM_RMEP_FLD_MD_NAME:
        sal_memcpy(p_db_rmep->md_name, p_rmep->md_name, sizeof(p_rmep->md_name));
        break;

    case TBL_CFM_RMEP_FLD_MAID:
        sal_memcpy(p_db_rmep->maid, p_rmep->maid, sizeof(p_rmep->maid));
        break;

    case TBL_CFM_RMEP_FLD_MAC_ADDR:
        sal_memcpy(p_db_rmep->mac_addr, p_rmep->mac_addr, sizeof(p_rmep->mac_addr));
        break;

    case TBL_CFM_RMEP_FLD_FIRST_PKT_RX:
        p_db_rmep->first_pkt_rx = p_rmep->first_pkt_rx;
        break;

    case TBL_CFM_RMEP_FLD_D_LOC:
        p_db_rmep->d_loc = p_rmep->d_loc;
        break;

    case TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD:
        p_db_rmep->d_unexp_period = p_rmep->d_unexp_period;
        break;

    case TBL_CFM_RMEP_FLD_MA_SA_MISMATCH:
        p_db_rmep->ma_sa_mismatch = p_rmep->ma_sa_mismatch;
        break;

    case TBL_CFM_RMEP_FLD_RMEP_LAST_RDI:
        p_db_rmep->rmep_last_rdi = p_rmep->rmep_last_rdi;
        break;

    case TBL_CFM_RMEP_FLD_IFNAME:
        sal_memcpy(p_db_rmep->ifname, p_rmep->ifname, sizeof(p_rmep->ifname));
        break;

    case TBL_CFM_RMEP_FLD_LOCAL_MEP_ID:
        p_db_rmep->local_mep_id = p_rmep->local_mep_id;
        break;

    case TBL_CFM_RMEP_FLD_MAX:
        sal_memcpy(p_db_rmep, p_rmep, sizeof(tbl_cfm_rmep_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_RMEP, field_id, p_db_rmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_RMEP, field_id, p_db_rmep);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_rmep_add_cfm_rmep(tbl_cfm_rmep_t *p_rmep)
{
    return tbl_cfm_rmep_add_cfm_rmep_sync(p_rmep, TRUE);
}

int32
tbl_cfm_rmep_del_cfm_rmep(tbl_cfm_rmep_key_t *p_rmep_key)
{
    return tbl_cfm_rmep_del_cfm_rmep_sync(p_rmep_key, TRUE);
}

int32
tbl_cfm_rmep_set_cfm_rmep_field(tbl_cfm_rmep_t *p_rmep, tbl_cfm_rmep_field_id_t field_id)
{
    return tbl_cfm_rmep_set_cfm_rmep_field_sync(p_rmep, field_id, TRUE);
}

tbl_cfm_rmep_t*
tbl_cfm_rmep_get_cfm_rmep(tbl_cfm_rmep_key_t *p_rmep_key)
{
    tbl_cfm_rmep_master_t *p_master = _g_p_tbl_cfm_rmep_master;
    tbl_cfm_rmep_t lkp;

    sal_memcpy(&lkp.key, p_rmep_key, sizeof(tbl_cfm_rmep_key_t));
    return ctclib_hash_lookup(p_master->cfm_rmep_hash, &lkp);
}

char*
tbl_cfm_rmep_key_val2str(tbl_cfm_rmep_t *p_rmep, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_cfm_rmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmep->key));
    return str;
}

int32
tbl_cfm_rmep_key_str2val(char *str, tbl_cfm_rmep_t *p_rmep)
{
    int32 ret = 0;

    ret = cdb_cfm_rmep_key_str2val(str, &p_rmep->key);
    return ret;
}

char*
tbl_cfm_rmep_key_name_dump(tbl_cfm_rmep_t *p_rmep, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_RMEP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_cfm_rmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmep->key));
    return str;
}

char*
tbl_cfm_rmep_key_value_dump(tbl_cfm_rmep_t *p_rmep, char *str)
{

    cdb_cfm_rmep_key_val2str(str, MAX_CMD_STR_LEN, &p_rmep->key);
    return str;
}

char*
tbl_cfm_rmep_field_name_dump(tbl_cfm_rmep_t *p_rmep, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_RMEP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_RMEP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_rmep_field_value_dump(tbl_cfm_rmep_t *p_rmep, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_KEY, field_id))
    {
        cdb_cfm_rmep_key_val2str(str, MAX_CMD_STR_LEN, &p_rmep->key);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->ifindex);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_rmep->md_name);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAID, field_id))
    {
        sal_sprintf(str, "%s", p_rmep->maid);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_rmep->mac_addr);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_FIRST_PKT_RX, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->first_pkt_rx);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_LOC, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->d_loc);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->d_unexp_period);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MA_SA_MISMATCH, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->ma_sa_mismatch);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_RMEP_LAST_RDI, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->rmep_last_rdi);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_rmep->ifname);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_LOCAL_MEP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_rmep->local_mep_id);
    }
    return str;
}

char**
tbl_cfm_rmep_table_dump(tbl_cfm_rmep_t *p_rmep, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_rmep_key_name_dump(p_rmep, buf));
    for(i=1; i<TBL_CFM_RMEP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_rmep_field_name_dump(p_rmep, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_rmep_field_value_dump(p_rmep, i, buf));
    }
    return str;
}

int32
tbl_cfm_rmep_field_value_parser(char *str, int32 field_id, tbl_cfm_rmep_t *p_rmep)
{
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_KEY, field_id))
    {
        cdb_cfm_rmep_key_str2val(str, &p_rmep->key);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_rmep->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MD_NAME, field_id))
    {
        sal_strcpy(p_rmep->md_name, str);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAID, field_id))
    {
        sal_strcpy(p_rmep->maid, str);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_rmep->mac_addr, str);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_FIRST_PKT_RX, field_id))
    {
        int32 ret;
        p_rmep->first_pkt_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_LOC, field_id))
    {
        int32 ret;
        p_rmep->d_loc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD, field_id))
    {
        int32 ret;
        p_rmep->d_unexp_period = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MA_SA_MISMATCH, field_id))
    {
        int32 ret;
        p_rmep->ma_sa_mismatch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_RMEP_LAST_RDI, field_id))
    {
        int32 ret;
        p_rmep->rmep_last_rdi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_rmep->ifname, str);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_LOCAL_MEP_ID, field_id))
    {
        int32 ret;
        p_rmep->local_mep_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cfm_rmep_table_parser(char** array, char* key_value,tbl_cfm_rmep_t *p_rmep)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_RMEP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_rmep_key_str2val(key_value, p_rmep));

    for(i=1; i<TBL_CFM_RMEP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_rmep_field_value_parser( array[j++], i, p_rmep));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_rmep_dump_one(tbl_cfm_rmep_t *p_rmep, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_RMEP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CFM_RMEP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_RMEP_FLD_KEY].name, 
            cdb_cfm_rmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_rmep->key));
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_IFINDEX].name,
            p_rmep->ifindex);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_RMEP_FLD_MD_NAME].name,
            p_rmep->md_name);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_RMEP_FLD_MAID].name,
            p_rmep->maid);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_RMEP_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_rmep->mac_addr));
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_FIRST_PKT_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_FIRST_PKT_RX].name,
            p_rmep->first_pkt_rx);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_LOC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_D_LOC].name,
            p_rmep->d_loc);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD].name,
            p_rmep->d_unexp_period);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_MA_SA_MISMATCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_MA_SA_MISMATCH].name,
            p_rmep->ma_sa_mismatch);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_RMEP_LAST_RDI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_RMEP_LAST_RDI].name,
            p_rmep->rmep_last_rdi);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_RMEP_FLD_IFNAME].name,
            p_rmep->ifname);
    }
    if (FLD_MATCH(TBL_CFM_RMEP_FLD_LOCAL_MEP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_RMEP_FLD_LOCAL_MEP_ID].name,
            p_rmep->local_mep_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_rmep_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_rmep_master_t *p_master = _g_p_tbl_cfm_rmep_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cfm_rmep_t *p_db_rmep = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_rmep_list, p_db_rmep, listnode, next)
    {
        rc |= fn(p_db_rmep, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_rmep_master_t*
tbl_cfm_rmep_get_master()
{
    return _g_p_tbl_cfm_rmep_master;
}

tbl_cfm_rmep_master_t*
tbl_cfm_rmep_init_cfm_rmep()
{
    _g_p_tbl_cfm_rmep_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cfm_rmep_master_t));
    _g_p_tbl_cfm_rmep_master->cfm_rmep_hash = ctclib_hash_create(_tbl_cfm_rmep_hash_make, _tbl_cfm_rmep_hash_cmp);
    _g_p_tbl_cfm_rmep_master->cfm_rmep_list = ctclib_slist_create(tbl_cfm_rmep_cmp, NULL);
    return _g_p_tbl_cfm_rmep_master;
}



#include "proto.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "cdb_data_cmp.h"

tbl_cfm_lmep_master_t *_g_p_tbl_cfm_lmep_master = NULL;

static uint32
_tbl_cfm_lmep_hash_make(tbl_cfm_lmep_t *p_lmep)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_lmep->key;
    for (index = 0; index < sizeof(p_lmep->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cfm_lmep_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cfm_lmep_t *p_lmep1 = (tbl_cfm_lmep_t*)p_arg1;
    tbl_cfm_lmep_t *p_lmep2 = (tbl_cfm_lmep_t*)p_arg2;

    if (0 == sal_memcmp(&p_lmep1->key, &p_lmep2->key, sizeof(tbl_cfm_lmep_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cfm_lmep_add_cfm_lmep_sync(tbl_cfm_lmep_t *p_lmep, uint32 sync)
{
    tbl_cfm_lmep_master_t *p_master = _g_p_tbl_cfm_lmep_master;
    tbl_cfm_lmep_t *p_db_lmep = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cfm_lmep_get_cfm_lmep(&p_lmep->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lmep = XCALLOC(MEM_TBL_CFM_LMEP, sizeof(tbl_cfm_lmep_t));
    if (NULL == p_db_lmep)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lmep, p_lmep, sizeof(tbl_cfm_lmep_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_lmep_hash, (void*)p_db_lmep, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_lmep_list, p_db_lmep);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CFM_LMEP, p_db_lmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CFM_LMEP, p_db_lmep);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_lmep_del_cfm_lmep_sync(tbl_cfm_lmep_key_t *p_lmep_key, uint32 sync)
{
    tbl_cfm_lmep_master_t *p_master = _g_p_tbl_cfm_lmep_master;
    tbl_cfm_lmep_t *p_db_lmep = NULL;

    /* 1. lookup entry exist */
    p_db_lmep = tbl_cfm_lmep_get_cfm_lmep(p_lmep_key);
    if (NULL == p_db_lmep)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CFM_LMEP, p_db_lmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CFM_LMEP, p_db_lmep);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_lmep_hash, (void*)p_db_lmep);
    ctclib_slistnode_delete(p_master->cfm_lmep_list, p_db_lmep);

    /* 4. free db entry */
    XFREE(MEM_TBL_CFM_LMEP, p_db_lmep);

    return PM_E_NONE;
}

int32
tbl_cfm_lmep_set_cfm_lmep_field_sync(tbl_cfm_lmep_t *p_lmep, tbl_cfm_lmep_field_id_t field_id, uint32 sync)
{
    tbl_cfm_lmep_t *p_db_lmep = NULL;

    /* 1. lookup entry exist */
    p_db_lmep = tbl_cfm_lmep_get_cfm_lmep(&p_lmep->key);
    if (NULL == p_db_lmep)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_LMEP_FLD_MEP_ID:
        p_db_lmep->mep_id = p_lmep->mep_id;
        break;

    case TBL_CFM_LMEP_FLD_MAID:
        sal_memcpy(p_db_lmep->maid, p_lmep->maid, sizeof(p_lmep->maid));
        break;

    case TBL_CFM_LMEP_FLD_MD_NAME:
        sal_memcpy(p_db_lmep->md_name, p_lmep->md_name, sizeof(p_lmep->md_name));
        break;

    case TBL_CFM_LMEP_FLD_DIRECTION:
        p_db_lmep->direction = p_lmep->direction;
        break;

    case TBL_CFM_LMEP_FLD_D_UNEXP_MEP:
        p_db_lmep->d_unexp_mep = p_lmep->d_unexp_mep;
        break;

    case TBL_CFM_LMEP_FLD_D_MISMERGE:
        p_db_lmep->d_mismerge = p_lmep->d_mismerge;
        break;

    case TBL_CFM_LMEP_FLD_D_MEG_LVL:
        p_db_lmep->d_meg_lvl = p_lmep->d_meg_lvl;
        break;

    case TBL_CFM_LMEP_FLD_CCI_ENABLED:
        p_db_lmep->cci_enabled = p_lmep->cci_enabled;
        break;

    case TBL_CFM_LMEP_FLD_CCI_INTERVAL:
        p_db_lmep->cci_interval = p_lmep->cci_interval;
        break;

    case TBL_CFM_LMEP_FLD_RDI:
        p_db_lmep->rdi = p_lmep->rdi;
        break;

    case TBL_CFM_LMEP_FLD_IFNAME:
        sal_memcpy(p_db_lmep->ifname, p_lmep->ifname, sizeof(p_lmep->ifname));
        break;

    case TBL_CFM_LMEP_FLD_IF_MAC_ADDR:
        sal_memcpy(p_db_lmep->if_mac_addr, p_lmep->if_mac_addr, sizeof(p_lmep->if_mac_addr));
        break;

    case TBL_CFM_LMEP_FLD_CCM_VLAN_COS:
        p_db_lmep->ccm_vlan_cos = p_lmep->ccm_vlan_cos;
        break;

    case TBL_CFM_LMEP_FLD_RING_ID:
        p_db_lmep->ring_id = p_lmep->ring_id;
        break;

    case TBL_CFM_LMEP_FLD_MEP_INDEX:
        p_db_lmep->mep_index = p_lmep->mep_index;
        break;

    case TBL_CFM_LMEP_FLD_RMEP_NUM:
        p_db_lmep->rmep_num = p_lmep->rmep_num;
        break;

    case TBL_CFM_LMEP_FLD_MAX:
        sal_memcpy(p_db_lmep, p_lmep, sizeof(tbl_cfm_lmep_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_LMEP, field_id, p_db_lmep);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_LMEP, field_id, p_db_lmep);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_lmep_add_cfm_lmep(tbl_cfm_lmep_t *p_lmep)
{
    return tbl_cfm_lmep_add_cfm_lmep_sync(p_lmep, TRUE);
}

int32
tbl_cfm_lmep_del_cfm_lmep(tbl_cfm_lmep_key_t *p_lmep_key)
{
    return tbl_cfm_lmep_del_cfm_lmep_sync(p_lmep_key, TRUE);
}

int32
tbl_cfm_lmep_set_cfm_lmep_field(tbl_cfm_lmep_t *p_lmep, tbl_cfm_lmep_field_id_t field_id)
{
    return tbl_cfm_lmep_set_cfm_lmep_field_sync(p_lmep, field_id, TRUE);
}

tbl_cfm_lmep_t*
tbl_cfm_lmep_get_cfm_lmep(tbl_cfm_lmep_key_t *p_lmep_key)
{
    tbl_cfm_lmep_master_t *p_master = _g_p_tbl_cfm_lmep_master;
    tbl_cfm_lmep_t lkp;

    sal_memcpy(&lkp.key, p_lmep_key, sizeof(tbl_cfm_lmep_key_t));
    return ctclib_hash_lookup(p_master->cfm_lmep_hash, &lkp);
}

char*
tbl_cfm_lmep_key_val2str(tbl_cfm_lmep_t *p_lmep, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_cfm_lmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_lmep->key));
    return str;
}

int32
tbl_cfm_lmep_key_str2val(char *str, tbl_cfm_lmep_t *p_lmep)
{
    int32 ret = 0;

    ret = cdb_cfm_lmep_key_str2val(str, &p_lmep->key);
    return ret;
}

char*
tbl_cfm_lmep_key_name_dump(tbl_cfm_lmep_t *p_lmep, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_LMEP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_cfm_lmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_lmep->key));
    return str;
}

char*
tbl_cfm_lmep_key_value_dump(tbl_cfm_lmep_t *p_lmep, char *str)
{

    cdb_cfm_lmep_key_val2str(str, MAX_CMD_STR_LEN, &p_lmep->key);
    return str;
}

char*
tbl_cfm_lmep_field_name_dump(tbl_cfm_lmep_t *p_lmep, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_LMEP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_LMEP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_lmep_field_value_dump(tbl_cfm_lmep_t *p_lmep, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_KEY, field_id))
    {
        cdb_cfm_lmep_key_val2str(str, MAX_CMD_STR_LEN, &p_lmep->key);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->mep_id);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MAID, field_id))
    {
        sal_sprintf(str, "%s", p_lmep->maid);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_lmep->md_name);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_DIRECTION, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->direction);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_UNEXP_MEP, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->d_unexp_mep);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MISMERGE, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->d_mismerge);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MEG_LVL, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->d_meg_lvl);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_ENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->cci_enabled);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->cci_interval);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RDI, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->rdi);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_lmep->ifname);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IF_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lmep->if_mac_addr);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCM_VLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->ccm_vlan_cos);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RING_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->ring_id);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->mep_index);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RMEP_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_lmep->rmep_num);
    }
    return str;
}

char**
tbl_cfm_lmep_table_dump(tbl_cfm_lmep_t *p_lmep, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_lmep_key_name_dump(p_lmep, buf));
    for(i=1; i<TBL_CFM_LMEP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_lmep_field_name_dump(p_lmep, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_lmep_field_value_dump(p_lmep, i, buf));
    }
    return str;
}

int32
tbl_cfm_lmep_field_value_parser(char *str, int32 field_id, tbl_cfm_lmep_t *p_lmep)
{
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_KEY, field_id))
    {
        cdb_cfm_lmep_key_str2val(str, &p_lmep->key);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_ID, field_id))
    {
        int32 ret;
        p_lmep->mep_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MAID, field_id))
    {
        sal_strcpy(p_lmep->maid, str);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MD_NAME, field_id))
    {
        sal_strcpy(p_lmep->md_name, str);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_DIRECTION, field_id))
    {
        int32 ret;
        p_lmep->direction = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_UNEXP_MEP, field_id))
    {
        int32 ret;
        p_lmep->d_unexp_mep = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MISMERGE, field_id))
    {
        int32 ret;
        p_lmep->d_mismerge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MEG_LVL, field_id))
    {
        int32 ret;
        p_lmep->d_meg_lvl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_ENABLED, field_id))
    {
        int32 ret;
        p_lmep->cci_enabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_INTERVAL, field_id))
    {
        int32 ret;
        p_lmep->cci_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RDI, field_id))
    {
        int32 ret;
        p_lmep->rdi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_lmep->ifname, str);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IF_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_lmep->if_mac_addr, str);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCM_VLAN_COS, field_id))
    {
        int32 ret;
        p_lmep->ccm_vlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RING_ID, field_id))
    {
        int32 ret;
        p_lmep->ring_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_INDEX, field_id))
    {
        int32 ret;
        p_lmep->mep_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RMEP_NUM, field_id))
    {
        int32 ret;
        p_lmep->rmep_num = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cfm_lmep_table_parser(char** array, char* key_value,tbl_cfm_lmep_t *p_lmep)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_LMEP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_lmep_key_str2val(key_value, p_lmep));

    for(i=1; i<TBL_CFM_LMEP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_lmep_field_value_parser( array[j++], i, p_lmep));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_lmep_dump_one(tbl_cfm_lmep_t *p_lmep, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_LMEP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CFM_LMEP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_LMEP_FLD_KEY].name, 
            cdb_cfm_lmep_key_val2str(buf, MAX_CMD_STR_LEN, &p_lmep->key));
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_MEP_ID].name,
            p_lmep->mep_id);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MAID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_LMEP_FLD_MAID].name,
            p_lmep->maid);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_LMEP_FLD_MD_NAME].name,
            p_lmep->md_name);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_DIRECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_DIRECTION].name,
            p_lmep->direction);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_UNEXP_MEP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_D_UNEXP_MEP].name,
            p_lmep->d_unexp_mep);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MISMERGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_D_MISMERGE].name,
            p_lmep->d_mismerge);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_D_MEG_LVL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_D_MEG_LVL].name,
            p_lmep->d_meg_lvl);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_ENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_CCI_ENABLED].name,
            p_lmep->cci_enabled);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCI_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_CCI_INTERVAL].name,
            p_lmep->cci_interval);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RDI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_RDI].name,
            p_lmep->rdi);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_LMEP_FLD_IFNAME].name,
            p_lmep->ifname);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_IF_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_LMEP_FLD_IF_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lmep->if_mac_addr));
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_CCM_VLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_CCM_VLAN_COS].name,
            p_lmep->ccm_vlan_cos);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RING_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_RING_ID].name,
            p_lmep->ring_id);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_MEP_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_MEP_INDEX].name,
            p_lmep->mep_index);
    }
    if (FLD_MATCH(TBL_CFM_LMEP_FLD_RMEP_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_LMEP_FLD_RMEP_NUM].name,
            p_lmep->rmep_num);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_lmep_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_lmep_master_t *p_master = _g_p_tbl_cfm_lmep_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cfm_lmep_t *p_db_lmep = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_lmep_list, p_db_lmep, listnode, next)
    {
        rc |= fn(p_db_lmep, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_lmep_master_t*
tbl_cfm_lmep_get_master()
{
    return _g_p_tbl_cfm_lmep_master;
}

tbl_cfm_lmep_master_t*
tbl_cfm_lmep_init_cfm_lmep()
{
    _g_p_tbl_cfm_lmep_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cfm_lmep_master_t));
    _g_p_tbl_cfm_lmep_master->cfm_lmep_hash = ctclib_hash_create(_tbl_cfm_lmep_hash_make, _tbl_cfm_lmep_hash_cmp);
    _g_p_tbl_cfm_lmep_master->cfm_lmep_list = ctclib_slist_create(tbl_cfm_lmep_cmp, NULL);
    return _g_p_tbl_cfm_lmep_master;
}


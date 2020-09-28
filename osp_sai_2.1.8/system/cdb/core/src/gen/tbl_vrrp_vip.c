
#include "proto.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "cdb_data_cmp.h"

tbl_vrrp_vip_master_t *_g_p_tbl_vrrp_vip_master = NULL;

static uint32
_tbl_vrrp_vip_hash_make(tbl_vrrp_vip_t *p_vrrp_vip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vrrp_vip->key;
    for (index = 0; index < sizeof(p_vrrp_vip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_vrrp_vip_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_vrrp_vip_t *p_vrrp_vip1 = (tbl_vrrp_vip_t*)p_arg1;
    tbl_vrrp_vip_t *p_vrrp_vip2 = (tbl_vrrp_vip_t*)p_arg2;

    if (0 == sal_memcmp(&p_vrrp_vip1->key, &p_vrrp_vip2->key, sizeof(tbl_vrrp_vip_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_vrrp_vip_add_vrrp_vip_sync(tbl_vrrp_vip_t *p_vrrp_vip, uint32 sync)
{
    tbl_vrrp_vip_master_t *p_master = _g_p_tbl_vrrp_vip_master;
    tbl_vrrp_vip_t *p_db_vrrp_vip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_vrrp_vip_get_vrrp_vip(&p_vrrp_vip->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vrrp_vip = XCALLOC(MEM_TBL_VRRP_VIP, sizeof(tbl_vrrp_vip_t));
    if (NULL == p_db_vrrp_vip)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vrrp_vip, p_vrrp_vip, sizeof(tbl_vrrp_vip_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->vrrp_vip_hash, (void*)p_db_vrrp_vip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->vrrp_vip_list, p_db_vrrp_vip);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VRRP_VIP, p_db_vrrp_vip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VRRP_VIP, p_db_vrrp_vip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_vip_del_vrrp_vip_sync(tbl_vrrp_vip_key_t *p_vrrp_vip_key, uint32 sync)
{
    tbl_vrrp_vip_master_t *p_master = _g_p_tbl_vrrp_vip_master;
    tbl_vrrp_vip_t *p_db_vrrp_vip = NULL;

    /* 1. lookup entry exist */
    p_db_vrrp_vip = tbl_vrrp_vip_get_vrrp_vip(p_vrrp_vip_key);
    if (NULL == p_db_vrrp_vip)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VRRP_VIP, p_db_vrrp_vip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VRRP_VIP, p_db_vrrp_vip);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->vrrp_vip_hash, (void*)p_db_vrrp_vip);
    ctclib_slistnode_delete(p_master->vrrp_vip_list, p_db_vrrp_vip);

    /* 4. free db entry */
    XFREE(MEM_TBL_VRRP_VIP, p_db_vrrp_vip);

    return PM_E_NONE;
}

int32
tbl_vrrp_vip_set_vrrp_vip_field_sync(tbl_vrrp_vip_t *p_vrrp_vip, tbl_vrrp_vip_field_id_t field_id, uint32 sync)
{
    tbl_vrrp_vip_t *p_db_vrrp_vip = NULL;

    /* 1. lookup entry exist */
    p_db_vrrp_vip = tbl_vrrp_vip_get_vrrp_vip(&p_vrrp_vip->key);
    if (NULL == p_db_vrrp_vip)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VRRP_VIP_FLD_VMAC:
        sal_memcpy(p_db_vrrp_vip->vmac, p_vrrp_vip->vmac, sizeof(p_vrrp_vip->vmac));
        break;

    case TBL_VRRP_VIP_FLD_INTERFACE_MAC:
        sal_memcpy(p_db_vrrp_vip->interface_mac, p_vrrp_vip->interface_mac, sizeof(p_vrrp_vip->interface_mac));
        break;

    case TBL_VRRP_VIP_FLD_MAX:
        sal_memcpy(p_db_vrrp_vip, p_vrrp_vip, sizeof(tbl_vrrp_vip_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VRRP_VIP, field_id, p_db_vrrp_vip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VRRP_VIP, field_id, p_db_vrrp_vip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_vip_add_vrrp_vip(tbl_vrrp_vip_t *p_vrrp_vip)
{
    return tbl_vrrp_vip_add_vrrp_vip_sync(p_vrrp_vip, TRUE);
}

int32
tbl_vrrp_vip_del_vrrp_vip(tbl_vrrp_vip_key_t *p_vrrp_vip_key)
{
    return tbl_vrrp_vip_del_vrrp_vip_sync(p_vrrp_vip_key, TRUE);
}

int32
tbl_vrrp_vip_set_vrrp_vip_field(tbl_vrrp_vip_t *p_vrrp_vip, tbl_vrrp_vip_field_id_t field_id)
{
    return tbl_vrrp_vip_set_vrrp_vip_field_sync(p_vrrp_vip, field_id, TRUE);
}

tbl_vrrp_vip_t*
tbl_vrrp_vip_get_vrrp_vip(tbl_vrrp_vip_key_t *p_vrrp_vip_key)
{
    tbl_vrrp_vip_master_t *p_master = _g_p_tbl_vrrp_vip_master;
    tbl_vrrp_vip_t lkp;

    sal_memcpy(&lkp.key, p_vrrp_vip_key, sizeof(tbl_vrrp_vip_key_t));
    return ctclib_hash_lookup(p_master->vrrp_vip_hash, &lkp);
}

char*
tbl_vrrp_vip_key_val2str(tbl_vrrp_vip_t *p_vrrp_vip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_vip->key));
    return str;
}

int32
tbl_vrrp_vip_key_str2val(char *str, tbl_vrrp_vip_t *p_vrrp_vip)
{
    int32 ret = 0;

    ret = cdb_addr_str2val(&p_vrrp_vip->key, str, 0);
    return ret;
}

char*
tbl_vrrp_vip_key_name_dump(tbl_vrrp_vip_t *p_vrrp_vip, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_VIP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_vip->key));
    return str;
}

char*
tbl_vrrp_vip_key_value_dump(tbl_vrrp_vip_t *p_vrrp_vip, char *str)
{

    cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_vip->key);
    return str;
}

char*
tbl_vrrp_vip_field_name_dump(tbl_vrrp_vip_t *p_vrrp_vip, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_VIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VRRP_VIP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vrrp_vip_field_value_dump(tbl_vrrp_vip_t *p_vrrp_vip, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_KEY, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vrrp_vip->key);
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_VMAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_vrrp_vip->vmac);
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_INTERFACE_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_vrrp_vip->interface_mac);
    }
    return str;
}

char**
tbl_vrrp_vip_table_dump(tbl_vrrp_vip_t *p_vrrp_vip, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vrrp_vip_key_name_dump(p_vrrp_vip, buf));
    for(i=1; i<TBL_VRRP_VIP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_vip_field_name_dump(p_vrrp_vip, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vrrp_vip_field_value_dump(p_vrrp_vip, i, buf));
    }
    return str;
}

int32
tbl_vrrp_vip_field_value_parser(char *str, int32 field_id, tbl_vrrp_vip_t *p_vrrp_vip)
{
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_KEY, field_id))
    {
        cdb_addr_str2val(&p_vrrp_vip->key, str, 0);
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_VMAC, field_id))
    {
        cdb_mac_addr_str2val(p_vrrp_vip->vmac, str);
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_INTERFACE_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_vrrp_vip->interface_mac, str);
    }
    return PM_E_NONE;
}

int32
tbl_vrrp_vip_table_parser(char** array, char* key_value,tbl_vrrp_vip_t *p_vrrp_vip)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_VIP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vrrp_vip_key_str2val(key_value, p_vrrp_vip));

    for(i=1; i<TBL_VRRP_VIP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vrrp_vip_field_value_parser( array[j++], i, p_vrrp_vip));
    }

    return PM_E_NONE;
}

int32
tbl_vrrp_vip_dump_one(tbl_vrrp_vip_t *p_vrrp_vip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VRRP_VIP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VRRP_VIP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_VIP_FLD_KEY].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vrrp_vip->key));
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_VMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_VIP_FLD_VMAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_vip->vmac));
    }
    if (FLD_MATCH(TBL_VRRP_VIP_FLD_INTERFACE_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VRRP_VIP_FLD_INTERFACE_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_vrrp_vip->interface_mac));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vrrp_vip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vrrp_vip_master_t *p_master = _g_p_tbl_vrrp_vip_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vrrp_vip_t *p_db_vrrp_vip = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vrrp_vip_list, p_db_vrrp_vip, listnode, next)
    {
        rc |= fn(p_db_vrrp_vip, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vrrp_vip_master_t*
tbl_vrrp_vip_get_master()
{
    return _g_p_tbl_vrrp_vip_master;
}

tbl_vrrp_vip_master_t*
tbl_vrrp_vip_init_vrrp_vip()
{
    _g_p_tbl_vrrp_vip_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vrrp_vip_master_t));
    _g_p_tbl_vrrp_vip_master->vrrp_vip_hash = ctclib_hash_create(_tbl_vrrp_vip_hash_make, _tbl_vrrp_vip_hash_cmp);
    _g_p_tbl_vrrp_vip_master->vrrp_vip_list = ctclib_slist_create(tbl_vrrp_vip_cmp, NULL);
    return _g_p_tbl_vrrp_vip_master;
}


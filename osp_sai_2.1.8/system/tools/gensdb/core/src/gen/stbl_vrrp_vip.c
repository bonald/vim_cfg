
#include "sdb.h"
#include "gen/stbl_vrrp_vip_define.h"
#include "gen/stbl_vrrp_vip.h"
#include "sdb_data_cmp.h"

stbl_vrrp_vip_master_t *_g_p_stbl_vrrp_vip_master = NULL;

static uint32
_stbl_vrrp_vip_hash_make(stbl_vrrp_vip_t *p_vip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vip->key;
    for (index = 0; index < sizeof(p_vip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_vrrp_vip_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_vrrp_vip_t *p_vip1 = (stbl_vrrp_vip_t*)p_arg1;
    stbl_vrrp_vip_t *p_vip2 = (stbl_vrrp_vip_t*)p_arg2;

    if (0 == sal_memcmp(&p_vip1->key, &p_vip2->key, sizeof(stbl_vrrp_vip_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_vrrp_vip_add_vrrp_vip_sync(stbl_vrrp_vip_t *p_vip, uint32 sync)
{
    stbl_vrrp_vip_master_t *p_master = _g_p_stbl_vrrp_vip_master;
    stbl_vrrp_vip_t *p_db_vip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_vrrp_vip_get_vrrp_vip(&p_vip->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vip = XCALLOC(MEM_STBL_VRRP_VIP, sizeof(stbl_vrrp_vip_t));
    if (NULL == p_db_vip)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vip, p_vip, sizeof(stbl_vrrp_vip_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->vip_hash, (void*)p_db_vip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_vrrp_vip_del_vrrp_vip_sync(stbl_vrrp_vip_key_t *p_vip_key, uint32 sync)
{
    stbl_vrrp_vip_master_t *p_master = _g_p_stbl_vrrp_vip_master;
    stbl_vrrp_vip_t *p_db_vip = NULL;

    /* 1. lookup entry exist */
    p_db_vip = stbl_vrrp_vip_get_vrrp_vip(p_vip_key);
    if (NULL == p_db_vip)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->vip_hash, (void*)p_db_vip);

    /* 4. free db entry */
    XFREE(MEM_STBL_VRRP_VIP, p_db_vip);

    return SDB_E_NONE;
}

int32
stbl_vrrp_vip_set_vrrp_vip_field_sync(stbl_vrrp_vip_t *p_vip, stbl_vrrp_vip_field_id_t field_id, uint32 sync)
{
    stbl_vrrp_vip_t *p_db_vip = NULL;

    /* 1. lookup entry exist */
    p_db_vip = stbl_vrrp_vip_get_vrrp_vip(&p_vip->key);
    if (NULL == p_db_vip)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_VRRP_VIP_FLD_VMAC:
        sal_memcpy(p_db_vip->vmac, p_vip->vmac, sizeof(p_vip->vmac));
        break;

    case STBL_VRRP_VIP_FLD_IF_MAC:
        sal_memcpy(p_db_vip->if_mac, p_vip->if_mac, sizeof(p_vip->if_mac));
        break;

    case STBL_VRRP_VIP_FLD_MAX:
        sal_memcpy(p_db_vip, p_vip, sizeof(stbl_vrrp_vip_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_vrrp_vip_add_vrrp_vip(stbl_vrrp_vip_t *p_vip)
{
    return stbl_vrrp_vip_add_vrrp_vip_sync(p_vip, TRUE);
}

int32
stbl_vrrp_vip_del_vrrp_vip(stbl_vrrp_vip_key_t *p_vip_key)
{
    return stbl_vrrp_vip_del_vrrp_vip_sync(p_vip_key, TRUE);
}

int32
stbl_vrrp_vip_set_vrrp_vip_field(stbl_vrrp_vip_t *p_vip, stbl_vrrp_vip_field_id_t field_id)
{
    return stbl_vrrp_vip_set_vrrp_vip_field_sync(p_vip, field_id, TRUE);
}

stbl_vrrp_vip_t*
stbl_vrrp_vip_get_vrrp_vip(stbl_vrrp_vip_key_t *p_vip_key)
{
    stbl_vrrp_vip_master_t *p_master = _g_p_stbl_vrrp_vip_master;
    stbl_vrrp_vip_t lkp;

    sal_memcpy(&lkp.key, p_vip_key, sizeof(stbl_vrrp_vip_key_t));
    return ctclib_hash_lookup(p_master->vip_hash, &lkp);
}

char*
stbl_vrrp_vip_key_val2str(stbl_vrrp_vip_t *p_vip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_ipaddr_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vip->key));
    return str;
}

int32
stbl_vrrp_vip_dump_one(stbl_vrrp_vip_t *p_vip, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_VRRP_VIP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_VRRP_VIP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VRRP_VIP_FLD_KEY].name, 
            sdb_ipaddr_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_vip->key));
    }
    if (SDB_FLD_MATCH(STBL_VRRP_VIP_FLD_VMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VRRP_VIP_FLD_VMAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_vip->vmac));
    }
    if (SDB_FLD_MATCH(STBL_VRRP_VIP_FLD_IF_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_VRRP_VIP_FLD_IF_MAC].name, 
            sdb_mac_addr_val2str(buf, SDB_MAX_CMD_STR_LEN, p_vip->if_mac));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_vrrp_vip_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_vrrp_vip_master_t *p_master = _g_p_stbl_vrrp_vip_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->vip_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_vrrp_vip_master_t*
stbl_vrrp_vip_get_master()
{
    return _g_p_stbl_vrrp_vip_master;
}

stbl_vrrp_vip_master_t*
stbl_vrrp_vip_init_vrrp_vip()
{
    _g_p_stbl_vrrp_vip_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_vrrp_vip_master_t));
    _g_p_stbl_vrrp_vip_master->vip_hash = ctclib_hash_create(_stbl_vrrp_vip_hash_make, _stbl_vrrp_vip_hash_cmp);
    return _g_p_stbl_vrrp_vip_master;
}


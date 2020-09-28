
#include "proto.h"
#include "gen/tbl_ipsg_s_ip_define.h"
#include "gen/tbl_ipsg_s_ip.h"
#include "cdb_data_cmp.h"

tbl_ipsg_s_ip_master_t *_g_p_tbl_ipsg_s_ip_master = NULL;

static uint32
_tbl_ipsg_s_ip_hash_make(tbl_ipsg_s_ip_t *p_ipsg_s_ip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ipsg_s_ip->key;
    for (index = 0; index < sizeof(p_ipsg_s_ip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ipsg_s_ip_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsg_s_ip_t *p_ipsg_s_ip1 = (tbl_ipsg_s_ip_t*)p_arg1;
    tbl_ipsg_s_ip_t *p_ipsg_s_ip2 = (tbl_ipsg_s_ip_t*)p_arg2;

    if (0 == sal_memcmp(&p_ipsg_s_ip1->key, &p_ipsg_s_ip2->key, sizeof(tbl_ipsg_s_ip_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsg_s_ip_add_ipsg_s_ip_sync(tbl_ipsg_s_ip_t *p_ipsg_s_ip, uint32 sync)
{
    tbl_ipsg_s_ip_master_t *p_master = _g_p_tbl_ipsg_s_ip_master;
    tbl_ipsg_s_ip_t *p_db_ipsg_s_ip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipsg_s_ip_get_ipsg_s_ip(&p_ipsg_s_ip->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipsg_s_ip = XCALLOC(MEM_TBL_IPSG_S_IP, sizeof(tbl_ipsg_s_ip_t));
    if (NULL == p_db_ipsg_s_ip)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipsg_s_ip, p_ipsg_s_ip, sizeof(tbl_ipsg_s_ip_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ipsg_s_ip_hash, (void*)p_db_ipsg_s_ip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPSG_S_IP, p_db_ipsg_s_ip);
    }

    return PM_E_NONE;
}

int32
tbl_ipsg_s_ip_del_ipsg_s_ip_sync(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key, uint32 sync)
{
    tbl_ipsg_s_ip_master_t *p_master = _g_p_tbl_ipsg_s_ip_master;
    tbl_ipsg_s_ip_t *p_db_ipsg_s_ip = NULL;

    /* 1. lookup entry exist */
    p_db_ipsg_s_ip = tbl_ipsg_s_ip_get_ipsg_s_ip(p_ipsg_s_ip_key);
    if (NULL == p_db_ipsg_s_ip)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPSG_S_IP, p_db_ipsg_s_ip);
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ipsg_s_ip_hash, (void*)p_db_ipsg_s_ip);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPSG_S_IP, p_db_ipsg_s_ip);

    return PM_E_NONE;
}

int32
tbl_ipsg_s_ip_set_ipsg_s_ip_field_sync(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_ipsg_s_ip_field_id_t field_id, uint32 sync)
{
    tbl_ipsg_s_ip_t *p_db_ipsg_s_ip = NULL;

    /* 1. lookup entry exist */
    p_db_ipsg_s_ip = tbl_ipsg_s_ip_get_ipsg_s_ip(&p_ipsg_s_ip->key);
    if (NULL == p_db_ipsg_s_ip)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSG_S_IP_FLD_VID:
        p_db_ipsg_s_ip->vid = p_ipsg_s_ip->vid;
        break;

    case TBL_IPSG_S_IP_FLD_IFNAME:
        sal_memcpy(p_db_ipsg_s_ip->ifname, p_ipsg_s_ip->ifname, sizeof(p_ipsg_s_ip->ifname));
        break;

    case TBL_IPSG_S_IP_FLD_MAC_ADDR:
        sal_memcpy(p_db_ipsg_s_ip->mac_addr, p_ipsg_s_ip->mac_addr, sizeof(p_ipsg_s_ip->mac_addr));
        break;

    case TBL_IPSG_S_IP_FLD_IP_ADDR:
        sal_memcpy(&p_db_ipsg_s_ip->ip_addr, &p_ipsg_s_ip->ip_addr, sizeof(p_ipsg_s_ip->ip_addr));
        break;

    case TBL_IPSG_S_IP_FLD_MAX:
        sal_memcpy(p_db_ipsg_s_ip, p_ipsg_s_ip, sizeof(tbl_ipsg_s_ip_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSG_S_IP, field_id, p_db_ipsg_s_ip);
    }

    return PM_E_NONE;
}

int32
tbl_ipsg_s_ip_add_ipsg_s_ip(tbl_ipsg_s_ip_t *p_ipsg_s_ip)
{
    return tbl_ipsg_s_ip_add_ipsg_s_ip_sync(p_ipsg_s_ip, TRUE);
}

int32
tbl_ipsg_s_ip_del_ipsg_s_ip(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key)
{
    return tbl_ipsg_s_ip_del_ipsg_s_ip_sync(p_ipsg_s_ip_key, TRUE);
}

int32
tbl_ipsg_s_ip_set_ipsg_s_ip_field(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_ipsg_s_ip_field_id_t field_id)
{
    return tbl_ipsg_s_ip_set_ipsg_s_ip_field_sync(p_ipsg_s_ip, field_id, TRUE);
}

tbl_ipsg_s_ip_t*
tbl_ipsg_s_ip_get_ipsg_s_ip(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key)
{
    tbl_ipsg_s_ip_master_t *p_master = _g_p_tbl_ipsg_s_ip_master;
    tbl_ipsg_s_ip_t lkp;

    sal_memcpy(&lkp.key, p_ipsg_s_ip_key, sizeof(tbl_ipsg_s_ip_key_t));
    return ctclib_hash_lookup(p_master->ipsg_s_ip_hash, &lkp);
}

char*
tbl_ipsg_s_ip_key_val2str(tbl_ipsg_s_ip_t *p_ipsg_s_ip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ipsg_s_ip->key.ip));
    return str;
}

int32
tbl_ipsg_s_ip_dump_one(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSG_S_IP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSG_S_IP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_IP_FLD_KEY].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ipsg_s_ip->key.ip));
    }
    if (FLD_MATCH(TBL_IPSG_S_IP_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSG_S_IP_FLD_VID].name,
            p_ipsg_s_ip->vid);
    }
    if (FLD_MATCH(TBL_IPSG_S_IP_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_IP_FLD_IFNAME].name,
            p_ipsg_s_ip->ifname);
    }
    if (FLD_MATCH(TBL_IPSG_S_IP_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_IP_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ipsg_s_ip->mac_addr));
    }
    if (FLD_MATCH(TBL_IPSG_S_IP_FLD_IP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_IP_FLD_IP_ADDR].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ipsg_s_ip->ip_addr));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsg_s_ip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsg_s_ip_master_t *p_master = _g_p_tbl_ipsg_s_ip_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->ipsg_s_ip_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_ipsg_s_ip_master_t*
tbl_ipsg_s_ip_get_master()
{
    return _g_p_tbl_ipsg_s_ip_master;
}

tbl_ipsg_s_ip_master_t*
tbl_ipsg_s_ip_init_ipsg_s_ip()
{
    _g_p_tbl_ipsg_s_ip_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipsg_s_ip_master_t));
    _g_p_tbl_ipsg_s_ip_master->ipsg_s_ip_hash = ctclib_hash_create(_tbl_ipsg_s_ip_hash_make, _tbl_ipsg_s_ip_hash_cmp);
    return _g_p_tbl_ipsg_s_ip_master;
}


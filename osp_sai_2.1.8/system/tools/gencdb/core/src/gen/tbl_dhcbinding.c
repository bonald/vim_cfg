
#include "proto.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "cdb_data_cmp.h"

tbl_dhcbinding_master_t *_g_p_tbl_dhcbinding_master = NULL;

static uint32
_tbl_dhcbinding_hash_make(tbl_dhcbinding_t *p_dhcbinding)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_dhcbinding->key;
    for (index = 0; index < sizeof(p_dhcbinding->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_dhcbinding_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_dhcbinding_t *p_dhcbinding1 = (tbl_dhcbinding_t*)p_arg1;
    tbl_dhcbinding_t *p_dhcbinding2 = (tbl_dhcbinding_t*)p_arg2;

    if (0 == sal_memcmp(&p_dhcbinding1->key, &p_dhcbinding2->key, sizeof(tbl_dhcbinding_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_dhcbinding_add_dhcbinding_sync(tbl_dhcbinding_t *p_dhcbinding, uint32 sync)
{
    tbl_dhcbinding_master_t *p_master = _g_p_tbl_dhcbinding_master;
    tbl_dhcbinding_t *p_db_dhcbinding = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_dhcbinding_get_dhcbinding(&p_dhcbinding->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dhcbinding = XCALLOC(MEM_TBL_DHCBINDING, sizeof(tbl_dhcbinding_t));
    if (NULL == p_db_dhcbinding)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dhcbinding, p_dhcbinding, sizeof(tbl_dhcbinding_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->dhcbinding_hash, (void*)p_db_dhcbinding, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->dhcbinding_list, p_db_dhcbinding);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DHCBINDING, p_db_dhcbinding);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DHCBINDING, p_db_dhcbinding);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcbinding_del_dhcbinding_sync(tbl_dhcbinding_key_t *p_dhcbinding_key, uint32 sync)
{
    tbl_dhcbinding_master_t *p_master = _g_p_tbl_dhcbinding_master;
    tbl_dhcbinding_t *p_db_dhcbinding = NULL;

    /* 1. lookup entry exist */
    p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(p_dhcbinding_key);
    if (NULL == p_db_dhcbinding)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DHCBINDING, p_db_dhcbinding);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DHCBINDING, p_db_dhcbinding);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->dhcbinding_hash, (void*)p_db_dhcbinding);
    ctclib_slistnode_delete(p_master->dhcbinding_list, p_db_dhcbinding);

    /* 4. free db entry */
    XFREE(MEM_TBL_DHCBINDING, p_db_dhcbinding);

    return PM_E_NONE;
}

int32
tbl_dhcbinding_set_dhcbinding_field_sync(tbl_dhcbinding_t *p_dhcbinding, tbl_dhcbinding_field_id_t field_id, uint32 sync)
{
    tbl_dhcbinding_t *p_db_dhcbinding = NULL;

    /* 1. lookup entry exist */
    p_db_dhcbinding = tbl_dhcbinding_get_dhcbinding(&p_dhcbinding->key);
    if (NULL == p_db_dhcbinding)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DHCBINDING_FLD_VID:
        p_db_dhcbinding->vid = p_dhcbinding->vid;
        break;

    case TBL_DHCBINDING_FLD_IFNAME:
        sal_memcpy(p_db_dhcbinding->ifname, p_dhcbinding->ifname, sizeof(p_dhcbinding->ifname));
        break;

    case TBL_DHCBINDING_FLD_IFNAME_NEW:
        sal_memcpy(p_db_dhcbinding->ifname_new, p_dhcbinding->ifname_new, sizeof(p_dhcbinding->ifname_new));
        break;

    case TBL_DHCBINDING_FLD_CLIENT_MAC:
        sal_memcpy(p_db_dhcbinding->client_mac, p_dhcbinding->client_mac, sizeof(p_dhcbinding->client_mac));
        break;

    case TBL_DHCBINDING_FLD_CLIENT_IP:
        sal_memcpy(&p_db_dhcbinding->client_ip, &p_dhcbinding->client_ip, sizeof(p_dhcbinding->client_ip));
        break;

    case TBL_DHCBINDING_FLD_CIPSOUR:
        p_db_dhcbinding->cIpsour = p_dhcbinding->cIpsour;
        break;

    case TBL_DHCBINDING_FLD_LEASE:
        p_db_dhcbinding->lease = p_dhcbinding->lease;
        break;

    case TBL_DHCBINDING_FLD_LEASE_NEW:
        p_db_dhcbinding->lease_new = p_dhcbinding->lease_new;
        break;

    case TBL_DHCBINDING_FLD_TRANSID:
        p_db_dhcbinding->transid = p_dhcbinding->transid;
        break;

    case TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT:
        p_db_dhcbinding->ipsg_tbl_exsit = p_dhcbinding->ipsg_tbl_exsit;
        break;

    case TBL_DHCBINDING_FLD_STATE:
        p_db_dhcbinding->state = p_dhcbinding->state;
        break;

    case TBL_DHCBINDING_FLD_TYPE:
        p_db_dhcbinding->type = p_dhcbinding->type;
        break;

    case TBL_DHCBINDING_FLD_MAX:
        sal_memcpy(p_db_dhcbinding, p_dhcbinding, sizeof(tbl_dhcbinding_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DHCBINDING, field_id, p_db_dhcbinding);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DHCBINDING, field_id, p_db_dhcbinding);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dhcbinding_add_dhcbinding(tbl_dhcbinding_t *p_dhcbinding)
{
    return tbl_dhcbinding_add_dhcbinding_sync(p_dhcbinding, TRUE);
}

int32
tbl_dhcbinding_del_dhcbinding(tbl_dhcbinding_key_t *p_dhcbinding_key)
{
    return tbl_dhcbinding_del_dhcbinding_sync(p_dhcbinding_key, TRUE);
}

int32
tbl_dhcbinding_set_dhcbinding_field(tbl_dhcbinding_t *p_dhcbinding, tbl_dhcbinding_field_id_t field_id)
{
    return tbl_dhcbinding_set_dhcbinding_field_sync(p_dhcbinding, field_id, TRUE);
}

tbl_dhcbinding_t*
tbl_dhcbinding_get_dhcbinding(tbl_dhcbinding_key_t *p_dhcbinding_key)
{
    tbl_dhcbinding_master_t *p_master = _g_p_tbl_dhcbinding_master;
    tbl_dhcbinding_t lkp;

    sal_memcpy(&lkp.key, p_dhcbinding_key, sizeof(tbl_dhcbinding_key_t));
    return ctclib_hash_lookup(p_master->dhcbinding_hash, &lkp);
}

char*
tbl_dhcbinding_key_val2str(tbl_dhcbinding_t *p_dhcbinding, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_binding_key_val2str(buf, MAX_CMD_STR_LEN, &p_dhcbinding->key));
    return str;
}

int32
tbl_dhcbinding_key_str2val(char *str, tbl_dhcbinding_t *p_dhcbinding)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_binding_key_str2val(&p_dhcbinding->key, str, err);
    return ret;
}

char*
tbl_dhcbinding_key_name_dump(tbl_dhcbinding_t *p_dhcbinding, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCBINDING);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_binding_key_val2str(buf, MAX_CMD_STR_LEN, &p_dhcbinding->key));
    return str;
}

char*
tbl_dhcbinding_key_value_dump(tbl_dhcbinding_t *p_dhcbinding, char *str)
{

    cdb_binding_key_val2str(str, MAX_CMD_STR_LEN, &p_dhcbinding->key);
    return str;
}

char*
tbl_dhcbinding_field_name_dump(tbl_dhcbinding_t *p_dhcbinding, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCBINDING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DHCBINDING_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dhcbinding_field_value_dump(tbl_dhcbinding_t *p_dhcbinding, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DHCBINDING_FLD_KEY, field_id))
    {
        cdb_binding_key_val2str(str, MAX_CMD_STR_LEN, &p_dhcbinding->key);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->vid);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_dhcbinding->ifname);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME_NEW, field_id))
    {
        sal_sprintf(str, "%s", p_dhcbinding->ifname_new);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_dhcbinding->client_mac);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_dhcbinding->client_ip);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CIPSOUR, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->cIpsour);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->lease);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE_NEW, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->lease_new);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TRANSID, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->transid);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT, field_id))
    {
        sal_sprintf(str, "%u", p_dhcbinding->ipsg_tbl_exsit);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(dhcbinding_state_strs, DHCBINDING_STATE_MAX, p_dhcbinding->state));
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(dhcbinding_type_strs, DHCBINDING_TYPE_MAX, p_dhcbinding->type));
    }
    return str;
}

char**
tbl_dhcbinding_table_dump(tbl_dhcbinding_t *p_dhcbinding, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dhcbinding_key_name_dump(p_dhcbinding, buf));
    for(i=1; i<TBL_DHCBINDING_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dhcbinding_field_name_dump(p_dhcbinding, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dhcbinding_field_value_dump(p_dhcbinding, i, buf));
    }
    return str;
}

int32
tbl_dhcbinding_field_value_parser(char *str, int32 field_id, tbl_dhcbinding_t *p_dhcbinding)
{
    if (FLD_MATCH(TBL_DHCBINDING_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_binding_key_str2val(&p_dhcbinding->key, str, err);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_VID, field_id))
    {
        int32 ret;
        p_dhcbinding->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_dhcbinding->ifname, str);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME_NEW, field_id))
    {
        sal_strcpy(p_dhcbinding->ifname_new, str);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_dhcbinding->client_mac, str);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_IP, field_id))
    {
        cdb_addr_str2val(&p_dhcbinding->client_ip, str, 0);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CIPSOUR, field_id))
    {
        int32 ret;
        p_dhcbinding->cIpsour = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE, field_id))
    {
        int32 ret;
        p_dhcbinding->lease = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE_NEW, field_id))
    {
        int32 ret;
        p_dhcbinding->lease_new = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TRANSID, field_id))
    {
        int32 ret;
        p_dhcbinding->transid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT, field_id))
    {
        int32 ret;
        p_dhcbinding->ipsg_tbl_exsit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_STATE, field_id))
    {
        cdb_enum_str2val(dhcbinding_state_strs, DHCBINDING_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(dhcbinding_type_strs, DHCBINDING_TYPE_MAX, str);
    }
    return PM_E_NONE;
}

int32
tbl_dhcbinding_table_parser(char** array, char* key_value,tbl_dhcbinding_t *p_dhcbinding)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCBINDING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dhcbinding_key_str2val(key_value, p_dhcbinding));

    for(i=1; i<TBL_DHCBINDING_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dhcbinding_field_value_parser( array[j++], i, p_dhcbinding));
    }

    return PM_E_NONE;
}

int32
tbl_dhcbinding_dump_one(tbl_dhcbinding_t *p_dhcbinding, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DHCBINDING);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DHCBINDING_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_KEY].name, 
            cdb_binding_key_val2str(buf, MAX_CMD_STR_LEN, &p_dhcbinding->key));
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_VID].name,
            p_dhcbinding->vid);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_IFNAME].name,
            p_dhcbinding->ifname);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IFNAME_NEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_IFNAME_NEW].name,
            p_dhcbinding->ifname_new);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_CLIENT_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_dhcbinding->client_mac));
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CLIENT_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_CLIENT_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_dhcbinding->client_ip));
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_CIPSOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_CIPSOUR].name,
            p_dhcbinding->cIpsour);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_LEASE].name,
            p_dhcbinding->lease);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_LEASE_NEW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_LEASE_NEW].name,
            p_dhcbinding->lease_new);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TRANSID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_TRANSID].name,
            p_dhcbinding->transid);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT].name,
            p_dhcbinding->ipsg_tbl_exsit);
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_STATE].name, 
            cdb_enum_val2str(dhcbinding_state_strs, DHCBINDING_STATE_MAX, p_dhcbinding->state));
    }
    if (FLD_MATCH(TBL_DHCBINDING_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DHCBINDING_FLD_TYPE].name, 
            cdb_enum_val2str(dhcbinding_type_strs, DHCBINDING_TYPE_MAX, p_dhcbinding->type));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dhcbinding_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dhcbinding_master_t *p_master = _g_p_tbl_dhcbinding_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dhcbinding_t *p_db_dhcbinding = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->dhcbinding_list, p_db_dhcbinding, listnode, next)
    {
        rc |= fn(p_db_dhcbinding, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dhcbinding_master_t*
tbl_dhcbinding_get_master()
{
    return _g_p_tbl_dhcbinding_master;
}

tbl_dhcbinding_master_t*
tbl_dhcbinding_init_dhcbinding()
{
    _g_p_tbl_dhcbinding_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dhcbinding_master_t));
    _g_p_tbl_dhcbinding_master->dhcbinding_hash = ctclib_hash_create(_tbl_dhcbinding_hash_make, _tbl_dhcbinding_hash_cmp);
    _g_p_tbl_dhcbinding_master->dhcbinding_list = ctclib_slist_create(tbl_dhcbinding_cmp, NULL);
    _g_p_tbl_dhcbinding_master->dhcbinding_ip_hash = ctclib_hash_create(_tbl_dhcbinding_hash_make, _tbl_dhcbinding_hash_cmp);
    return _g_p_tbl_dhcbinding_master;
}


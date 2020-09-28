
#include "proto.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "cdb_data_cmp.h"

tbl_openflow_tunnel_interface_master_t *_g_p_tbl_openflow_tunnel_interface_master = NULL;

static uint32
_tbl_openflow_tunnel_interface_hash_make(tbl_openflow_tunnel_interface_t *p_tunnel_if)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tunnel_if->key;
    for (index = 0; index < sizeof(p_tunnel_if->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_openflow_tunnel_interface_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_openflow_tunnel_interface_t *p_tunnel_if1 = (tbl_openflow_tunnel_interface_t*)p_arg1;
    tbl_openflow_tunnel_interface_t *p_tunnel_if2 = (tbl_openflow_tunnel_interface_t*)p_arg2;

    if (0 == sal_memcmp(&p_tunnel_if1->key, &p_tunnel_if2->key, sizeof(tbl_openflow_tunnel_interface_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_openflow_tunnel_interface_add_openflow_tunnel_interface_sync(tbl_openflow_tunnel_interface_t *p_tunnel_if, uint32 sync)
{
    tbl_openflow_tunnel_interface_master_t *p_master = _g_p_tbl_openflow_tunnel_interface_master;
    tbl_openflow_tunnel_interface_t *p_db_tunnel_if = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&p_tunnel_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tunnel_if = XCALLOC(MEM_TBL_OPENFLOW_TUNNEL_INTERFACE, sizeof(tbl_openflow_tunnel_interface_t));
    if (NULL == p_db_tunnel_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tunnel_if, p_tunnel_if, sizeof(tbl_openflow_tunnel_interface_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tunnel_if_hash, (void*)p_db_tunnel_if, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->tunnel_if_list, p_db_tunnel_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OPENFLOW_TUNNEL_INTERFACE, p_db_tunnel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OPENFLOW_TUNNEL_INTERFACE, p_db_tunnel_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_del_openflow_tunnel_interface_sync(tbl_openflow_tunnel_interface_key_t *p_tunnel_if_key, uint32 sync)
{
    tbl_openflow_tunnel_interface_master_t *p_master = _g_p_tbl_openflow_tunnel_interface_master;
    tbl_openflow_tunnel_interface_t *p_db_tunnel_if = NULL;

    /* 1. lookup entry exist */
    p_db_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(p_tunnel_if_key);
    if (NULL == p_db_tunnel_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OPENFLOW_TUNNEL_INTERFACE, p_db_tunnel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OPENFLOW_TUNNEL_INTERFACE, p_db_tunnel_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tunnel_if_hash, (void*)p_db_tunnel_if);
    ctclib_slistnode_delete(p_master->tunnel_if_list, p_db_tunnel_if);

    /* 4. free db entry */
    XFREE(MEM_TBL_OPENFLOW_TUNNEL_INTERFACE, p_db_tunnel_if);

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field_sync(tbl_openflow_tunnel_interface_t *p_tunnel_if, tbl_openflow_tunnel_interface_field_id_t field_id, uint32 sync)
{
    tbl_openflow_tunnel_interface_t *p_db_tunnel_if = NULL;

    /* 1. lookup entry exist */
    p_db_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&p_tunnel_if->key);
    if (NULL == p_db_tunnel_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE:
        p_db_tunnel_if->openflow_enable = p_tunnel_if->openflow_enable;
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP:
        sal_memcpy(p_db_tunnel_if->source_ip, p_tunnel_if->source_ip, sizeof(p_tunnel_if->source_ip));
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP:
        sal_memcpy(p_db_tunnel_if->remote_ip, p_tunnel_if->remote_ip, sizeof(p_tunnel_if->remote_ip));
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME:
        sal_memcpy(p_db_tunnel_if->bind_port_name, p_tunnel_if->bind_port_name, sizeof(p_tunnel_if->bind_port_name));
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC:
        sal_memcpy(p_db_tunnel_if->bind_mac, p_tunnel_if->bind_mac, sizeof(p_tunnel_if->bind_mac));
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN:
        p_db_tunnel_if->bind_vlan = p_tunnel_if->bind_vlan;
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG:
        p_db_tunnel_if->flag = p_tunnel_if->flag;
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS:
        p_db_tunnel_if->tunnel_status = p_tunnel_if->tunnel_status;
        break;

    case TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_MAX:
        sal_memcpy(p_db_tunnel_if, p_tunnel_if, sizeof(tbl_openflow_tunnel_interface_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW_TUNNEL_INTERFACE, field_id, p_db_tunnel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW_TUNNEL_INTERFACE, field_id, p_db_tunnel_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_add_openflow_tunnel_interface(tbl_openflow_tunnel_interface_t *p_tunnel_if)
{
    return tbl_openflow_tunnel_interface_add_openflow_tunnel_interface_sync(p_tunnel_if, TRUE);
}

int32
tbl_openflow_tunnel_interface_del_openflow_tunnel_interface(tbl_openflow_tunnel_interface_key_t *p_tunnel_if_key)
{
    return tbl_openflow_tunnel_interface_del_openflow_tunnel_interface_sync(p_tunnel_if_key, TRUE);
}

int32
tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(tbl_openflow_tunnel_interface_t *p_tunnel_if, tbl_openflow_tunnel_interface_field_id_t field_id)
{
    return tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field_sync(p_tunnel_if, field_id, TRUE);
}

tbl_openflow_tunnel_interface_t*
tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(tbl_openflow_tunnel_interface_key_t *p_tunnel_if_key)
{
    tbl_openflow_tunnel_interface_master_t *p_master = _g_p_tbl_openflow_tunnel_interface_master;
    tbl_openflow_tunnel_interface_t lkp;

    sal_memcpy(&lkp.key, p_tunnel_if_key, sizeof(tbl_openflow_tunnel_interface_key_t));
    return ctclib_hash_lookup(p_master->tunnel_if_hash, &lkp);
}

char*
tbl_openflow_tunnel_interface_key_val2str(tbl_openflow_tunnel_interface_t *p_tunnel_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_tunnel_if->key.name);
    return str;
}

int32
tbl_openflow_tunnel_interface_key_str2val(char *str, tbl_openflow_tunnel_interface_t *p_tunnel_if)
{
    int32 ret = 0;

    sal_strcpy(p_tunnel_if->key.name, str);
    return ret;
}

char*
tbl_openflow_tunnel_interface_key_name_dump(tbl_openflow_tunnel_interface_t *p_tunnel_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_INTERFACE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_tunnel_if->key.name);
    return str;
}

char*
tbl_openflow_tunnel_interface_key_value_dump(tbl_openflow_tunnel_interface_t *p_tunnel_if, char *str)
{
    sal_sprintf(str, "%s", p_tunnel_if->key.name);
    return str;
}

char*
tbl_openflow_tunnel_interface_field_name_dump(tbl_openflow_tunnel_interface_t *p_tunnel_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_tunnel_interface_field_value_dump(tbl_openflow_tunnel_interface_t *p_tunnel_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_tunnel_if->key.name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_tunnel_if->openflow_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP, field_id))
    {
        sal_sprintf(str, "%s", p_tunnel_if->source_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP, field_id))
    {
        sal_sprintf(str, "%s", p_tunnel_if->remote_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_tunnel_if->bind_port_name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC, field_id))
    {
        sal_sprintf(str, "%s", p_tunnel_if->bind_mac);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_tunnel_if->bind_vlan);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_tunnel_if->flag);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_tunnel_if->tunnel_status);
    }
    return str;
}

char**
tbl_openflow_tunnel_interface_table_dump(tbl_openflow_tunnel_interface_t *p_tunnel_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_tunnel_interface_key_name_dump(p_tunnel_if, buf));
    for(i=1; i<TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_tunnel_interface_field_name_dump(p_tunnel_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_tunnel_interface_field_value_dump(p_tunnel_if, i, buf));
    }
    return str;
}

int32
tbl_openflow_tunnel_interface_field_value_parser(char *str, int32 field_id, tbl_openflow_tunnel_interface_t *p_tunnel_if)
{
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_KEY, field_id))
    {
        sal_strcpy(p_tunnel_if->key.name, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE, field_id))
    {
        int32 ret;
        p_tunnel_if->openflow_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP, field_id))
    {
        sal_strcpy(p_tunnel_if->source_ip, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP, field_id))
    {
        sal_strcpy(p_tunnel_if->remote_ip, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME, field_id))
    {
        sal_strcpy(p_tunnel_if->bind_port_name, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC, field_id))
    {
        sal_strcpy(p_tunnel_if->bind_mac, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN, field_id))
    {
        int32 ret;
        p_tunnel_if->bind_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG, field_id))
    {
        int32 ret;
        p_tunnel_if->flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS, field_id))
    {
        int32 ret;
        p_tunnel_if->tunnel_status = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_table_parser(char** array, char* key_value,tbl_openflow_tunnel_interface_t *p_tunnel_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_tunnel_interface_key_str2val(key_value, p_tunnel_if));

    for(i=1; i<TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_tunnel_interface_field_value_parser( array[j++], i, p_tunnel_if));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_dump_one(tbl_openflow_tunnel_interface_t *p_tunnel_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_TUNNEL_INTERFACE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_KEY].name,
            p_tunnel_if->key.name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE].name,
            p_tunnel_if->openflow_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP].name,
            p_tunnel_if->source_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP].name,
            p_tunnel_if->remote_ip);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME].name,
            p_tunnel_if->bind_port_name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC].name,
            p_tunnel_if->bind_mac);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN].name,
            p_tunnel_if->bind_vlan);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG].name,
            p_tunnel_if->flag);
    }
    if (FLD_MATCH(TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS].name,
            p_tunnel_if->tunnel_status);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_tunnel_interface_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_tunnel_interface_master_t *p_master = _g_p_tbl_openflow_tunnel_interface_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_openflow_tunnel_interface_t *p_db_tunnel_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tunnel_if_list, p_db_tunnel_if, listnode, next)
    {
        rc |= fn(p_db_tunnel_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_openflow_tunnel_interface_master_t*
tbl_openflow_tunnel_interface_get_master()
{
    return _g_p_tbl_openflow_tunnel_interface_master;
}

tbl_openflow_tunnel_interface_master_t*
tbl_openflow_tunnel_interface_init_openflow_tunnel_interface()
{
    _g_p_tbl_openflow_tunnel_interface_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_openflow_tunnel_interface_master_t));
    _g_p_tbl_openflow_tunnel_interface_master->tunnel_if_hash = ctclib_hash_create(_tbl_openflow_tunnel_interface_hash_make, _tbl_openflow_tunnel_interface_hash_cmp);
    _g_p_tbl_openflow_tunnel_interface_master->tunnel_if_list = ctclib_slist_create(tbl_openflow_tunnel_interface_cmp, NULL);
    return _g_p_tbl_openflow_tunnel_interface_master;
}


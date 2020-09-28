
#include "proto.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "cdb_data_cmp.h"

tbl_openflow_interface_master_t *_g_p_tbl_openflow_interface_master = NULL;

static uint32
_tbl_openflow_interface_hash_make(tbl_openflow_interface_t *p_if)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_if->key;
    for (index = 0; index < sizeof(p_if->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_openflow_interface_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_openflow_interface_t *p_if1 = (tbl_openflow_interface_t*)p_arg1;
    tbl_openflow_interface_t *p_if2 = (tbl_openflow_interface_t*)p_arg2;

    if (0 == sal_memcmp(&p_if1->key, &p_if2->key, sizeof(tbl_openflow_interface_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_openflow_interface_add_openflow_interface_sync(tbl_openflow_interface_t *p_if, uint32 sync)
{
    tbl_openflow_interface_master_t *p_master = _g_p_tbl_openflow_interface_master;
    tbl_openflow_interface_t *p_db_if = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_openflow_interface_get_openflow_interface(&p_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_if = XCALLOC(MEM_TBL_OPENFLOW_INTERFACE, sizeof(tbl_openflow_interface_t));
    if (NULL == p_db_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_if, p_if, sizeof(tbl_openflow_interface_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->if_hash, (void*)p_db_if, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OPENFLOW_INTERFACE, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OPENFLOW_INTERFACE, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_interface_del_openflow_interface_sync(tbl_openflow_interface_key_t *p_if_key, uint32 sync)
{
    tbl_openflow_interface_master_t *p_master = _g_p_tbl_openflow_interface_master;
    tbl_openflow_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_openflow_interface_get_openflow_interface(p_if_key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OPENFLOW_INTERFACE, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OPENFLOW_INTERFACE, p_db_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->if_hash, (void*)p_db_if);
    ctclib_slistnode_delete(p_master->if_list, p_db_if);

    /* 4. free db entry */
    XFREE(MEM_TBL_OPENFLOW_INTERFACE, p_db_if);

    return PM_E_NONE;
}

int32
tbl_openflow_interface_set_openflow_interface_field_sync(tbl_openflow_interface_t *p_if, tbl_openflow_interface_field_id_t field_id, uint32 sync)
{
    tbl_openflow_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_openflow_interface_get_openflow_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE:
        p_db_if->openflow_enable = p_if->openflow_enable;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER:
        p_db_if->obey_vlan_filter = p_if->obey_vlan_filter;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE:
        p_db_if->openflow_instance_enable = p_if->openflow_instance_enable;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0:
        p_db_if->is_add_to_br0 = p_if->is_add_to_br0;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN:
        sal_memcpy(p_db_if->protected_vlan, p_if->protected_vlan, sizeof(p_if->protected_vlan));
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE:
        p_db_if->protected_vlan_change = p_if->protected_vlan_change;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET:
        p_db_if->protected_vlan_change_is_set = p_if->protected_vlan_change_is_set;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_ACTION:
        sal_memcpy(p_db_if->action, p_if->action, sizeof(p_if->action));
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE:
        p_db_if->ingress_add_native_vlan_enable = p_if->ingress_add_native_vlan_enable;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE:
        p_db_if->egress_remove_native_vlan_enable = p_if->egress_remove_native_vlan_enable;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_IFINDEX:
        p_db_if->ifindex = p_if->ifindex;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE:
        p_db_if->bind_tunnel_type = p_if->bind_tunnel_type;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT:
        p_db_if->bind_tunnel_cnt = p_if->bind_tunnel_cnt;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE:
        p_db_if->tunnel_mpls_disable = p_if->tunnel_mpls_disable;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL:
        p_db_if->is_bind_tunnel = p_if->is_bind_tunnel;
        break;

    case TBL_OPENFLOW_INTERFACE_FLD_MAX:
        sal_memcpy(p_db_if, p_if, sizeof(tbl_openflow_interface_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW_INTERFACE, field_id, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW_INTERFACE, field_id, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_interface_add_openflow_interface(tbl_openflow_interface_t *p_if)
{
    return tbl_openflow_interface_add_openflow_interface_sync(p_if, TRUE);
}

int32
tbl_openflow_interface_del_openflow_interface(tbl_openflow_interface_key_t *p_if_key)
{
    return tbl_openflow_interface_del_openflow_interface_sync(p_if_key, TRUE);
}

int32
tbl_openflow_interface_set_openflow_interface_field(tbl_openflow_interface_t *p_if, tbl_openflow_interface_field_id_t field_id)
{
    return tbl_openflow_interface_set_openflow_interface_field_sync(p_if, field_id, TRUE);
}

tbl_openflow_interface_t*
tbl_openflow_interface_get_openflow_interface(tbl_openflow_interface_key_t *p_if_key)
{
    tbl_openflow_interface_master_t *p_master = _g_p_tbl_openflow_interface_master;
    tbl_openflow_interface_t lkp;

    sal_memcpy(&lkp.key, p_if_key, sizeof(tbl_openflow_interface_key_t));
    return ctclib_hash_lookup(p_master->if_hash, &lkp);
}

char*
tbl_openflow_interface_key_val2str(tbl_openflow_interface_t *p_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_if->key.name);
    return str;
}

int32
tbl_openflow_interface_key_str2val(char *str, tbl_openflow_interface_t *p_if)
{
    int32 ret = 0;

    sal_strcpy(p_if->key.name, str);
    return ret;
}

char*
tbl_openflow_interface_key_name_dump(tbl_openflow_interface_t *p_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_INTERFACE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_if->key.name);
    return str;
}

char*
tbl_openflow_interface_key_value_dump(tbl_openflow_interface_t *p_if, char *str)
{
    sal_sprintf(str, "%s", p_if->key.name);
    return str;
}

char*
tbl_openflow_interface_field_name_dump(tbl_openflow_interface_t *p_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_INTERFACE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_interface_field_value_dump(tbl_openflow_interface_t *p_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_if->key.name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->openflow_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_if->obey_vlan_filter);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->openflow_instance_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0, field_id))
    {
        sal_sprintf(str, "%u", p_if->is_add_to_br0);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_if->protected_vlan, sizeof(p_if->protected_vlan)/4);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE, field_id))
    {
        sal_sprintf(str, "%u", p_if->protected_vlan_change);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET, field_id))
    {
        sal_sprintf(str, "%u", p_if->protected_vlan_change_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_ACTION, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_if->action, sizeof(p_if->action)/4);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->ingress_add_native_vlan_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->egress_remove_native_vlan_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_if->ifindex);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_if->bind_tunnel_type);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->bind_tunnel_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->tunnel_mpls_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL, field_id))
    {
        sal_sprintf(str, "%u", p_if->is_bind_tunnel);
    }
    return str;
}

char**
tbl_openflow_interface_table_dump(tbl_openflow_interface_t *p_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_interface_key_name_dump(p_if, buf));
    for(i=1; i<TBL_OPENFLOW_INTERFACE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_interface_field_name_dump(p_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_interface_field_value_dump(p_if, i, buf));
    }
    return str;
}

int32
tbl_openflow_interface_field_value_parser(char *str, int32 field_id, tbl_openflow_interface_t *p_if)
{
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_KEY, field_id))
    {
        sal_strcpy(p_if->key.name, str);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE, field_id))
    {
        int32 ret;
        p_if->openflow_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER, field_id))
    {
        int32 ret;
        p_if->obey_vlan_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE, field_id))
    {
        int32 ret;
        p_if->openflow_instance_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0, field_id))
    {
        int32 ret;
        p_if->is_add_to_br0 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN, field_id))
    {
        cdb_uint32_array_str2val(str, p_if->protected_vlan);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE, field_id))
    {
        int32 ret;
        p_if->protected_vlan_change = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET, field_id))
    {
        int32 ret;
        p_if->protected_vlan_change_is_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_ACTION, field_id))
    {
        cdb_uint32_array_str2val(str, p_if->action);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE, field_id))
    {
        int32 ret;
        p_if->ingress_add_native_vlan_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE, field_id))
    {
        int32 ret;
        p_if->egress_remove_native_vlan_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_if->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE, field_id))
    {
        int32 ret;
        p_if->bind_tunnel_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT, field_id))
    {
        int32 ret;
        p_if->bind_tunnel_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE, field_id))
    {
        int32 ret;
        p_if->tunnel_mpls_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL, field_id))
    {
        int32 ret;
        p_if->is_bind_tunnel = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_interface_table_parser(char** array, char* key_value,tbl_openflow_interface_t *p_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_interface_key_str2val(key_value, p_if));

    for(i=1; i<TBL_OPENFLOW_INTERFACE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_interface_field_value_parser( array[j++], i, p_if));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_interface_dump_one(tbl_openflow_interface_t *p_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_INTERFACE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_KEY].name,
            p_if->key.name);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE].name,
            p_if->openflow_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER].name,
            p_if->obey_vlan_filter);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE].name,
            p_if->openflow_instance_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0].name,
            p_if->is_add_to_br0);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_if->protected_vlan, sizeof(p_if->protected_vlan)/4));
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE].name,
            p_if->protected_vlan_change);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET].name,
            p_if->protected_vlan_change_is_set);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_ACTION].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_if->action, sizeof(p_if->action)/4));
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE].name,
            p_if->ingress_add_native_vlan_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE].name,
            p_if->egress_remove_native_vlan_enable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_IFINDEX].name,
            p_if->ifindex);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE].name,
            p_if->bind_tunnel_type);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT].name,
            p_if->bind_tunnel_cnt);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE].name,
            p_if->tunnel_mpls_disable);
    }
    if (FLD_MATCH(TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL].name,
            p_if->is_bind_tunnel);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_interface_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_interface_master_t *p_master = _g_p_tbl_openflow_interface_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_openflow_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        rc |= fn(p_db_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_openflow_interface_master_t*
tbl_openflow_interface_get_master()
{
    return _g_p_tbl_openflow_interface_master;
}

tbl_openflow_interface_master_t*
tbl_openflow_interface_init_openflow_interface()
{
    _g_p_tbl_openflow_interface_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_openflow_interface_master_t));
    _g_p_tbl_openflow_interface_master->if_hash = ctclib_hash_create(_tbl_openflow_interface_hash_make, _tbl_openflow_interface_hash_cmp);
    _g_p_tbl_openflow_interface_master->if_list = ctclib_slist_create(tbl_openflow_interface_cmp, NULL);
    return _g_p_tbl_openflow_interface_master;
}


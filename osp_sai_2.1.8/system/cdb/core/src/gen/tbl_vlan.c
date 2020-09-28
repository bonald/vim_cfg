
#include "proto.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "cdb_data_cmp.h"

tbl_vlan_master_t *_g_p_tbl_vlan_master = NULL;

int32
tbl_vlan_add_vlan_sync(tbl_vlan_t *p_vlan, uint32 sync)
{
    tbl_vlan_master_t *p_master = _g_p_tbl_vlan_master;
    tbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlan_get_vlan(&p_vlan->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan = XCALLOC(MEM_TBL_VLAN, sizeof(tbl_vlan_t));
    if (NULL == p_db_vlan)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan, p_vlan, sizeof(tbl_vlan_t));

    /* 4. add to db */
    p_master->vlan_array[p_vlan->key.vid] = p_db_vlan;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLAN, p_db_vlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLAN, p_db_vlan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_del_vlan_sync(tbl_vlan_key_t *p_vlan_key, uint32 sync)
{
    tbl_vlan_master_t *p_master = _g_p_tbl_vlan_master;
    tbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = tbl_vlan_get_vlan(p_vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLAN, p_db_vlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLAN, p_db_vlan);
        #endif 
    }

    /* 3. delete from db */
    p_master->vlan_array[p_vlan_key->vid] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VLAN, p_db_vlan);

    return PM_E_NONE;
}

int32
tbl_vlan_set_vlan_field_sync(tbl_vlan_t *p_vlan, tbl_vlan_field_id_t field_id, uint32 sync)
{
    tbl_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = tbl_vlan_get_vlan(&p_vlan->key);
    if (NULL == p_db_vlan)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLAN_FLD_MAX_MAC_COUNT:
        p_db_vlan->max_mac_count = p_vlan->max_mac_count;
        break;

    case TBL_VLAN_FLD_CUR_MAC_COUNT:
        p_db_vlan->cur_mac_count = p_vlan->cur_mac_count;
        break;

    case TBL_VLAN_FLD_INSTANCE:
        p_db_vlan->instance = p_vlan->instance;
        break;

    case TBL_VLAN_FLD_DHCSNOOPING:
        p_db_vlan->dhcsnooping = p_vlan->dhcsnooping;
        break;

    case TBL_VLAN_FLD_ARPSNOOPING:
        p_db_vlan->arpsnooping = p_vlan->arpsnooping;
        break;

    case TBL_VLAN_FLD_SECURITY_ACTION:
        p_db_vlan->security_action = p_vlan->security_action;
        break;

    case TBL_VLAN_FLD_ENABLE:
        p_db_vlan->enable = p_vlan->enable;
        break;

    case TBL_VLAN_FLD_MAC_LEARNING_EN:
        p_db_vlan->mac_learning_en = p_vlan->mac_learning_en;
        break;

    case TBL_VLAN_FLD_OP_IFINDEX:
        p_db_vlan->op_ifindex = p_vlan->op_ifindex;
        break;

    case TBL_VLAN_FLD_MEMBER_PORT:
        sal_memcpy(&p_db_vlan->member_port, &p_vlan->member_port, sizeof(p_vlan->member_port));
        break;

    case TBL_VLAN_FLD_NAME:
        sal_memcpy(p_db_vlan->name, p_vlan->name, sizeof(p_vlan->name));
        break;

    case TBL_VLAN_FLD_ERPS_DOMAIN_ID:
        p_db_vlan->erps_domain_id = p_vlan->erps_domain_id;
        break;

    case TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM:
        p_db_vlan->dot1x_guest_vlan_num = p_vlan->dot1x_guest_vlan_num;
        break;

    case TBL_VLAN_FLD_STATS_EN:
        p_db_vlan->stats_en = p_vlan->stats_en;
        break;

    case TBL_VLAN_FLD_G8032_RING_ID:
        p_db_vlan->g8032_ring_id = p_vlan->g8032_ring_id;
        break;

    case TBL_VLAN_FLD_MAX:
        sal_memcpy(p_db_vlan, p_vlan, sizeof(tbl_vlan_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLAN, field_id, p_db_vlan);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLAN, field_id, p_db_vlan);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_add_vlan(tbl_vlan_t *p_vlan)
{
    return tbl_vlan_add_vlan_sync(p_vlan, TRUE);
}

int32
tbl_vlan_del_vlan(tbl_vlan_key_t *p_vlan_key)
{
    return tbl_vlan_del_vlan_sync(p_vlan_key, TRUE);
}

int32
tbl_vlan_set_vlan_field(tbl_vlan_t *p_vlan, tbl_vlan_field_id_t field_id)
{
    return tbl_vlan_set_vlan_field_sync(p_vlan, field_id, TRUE);
}

tbl_vlan_t*
tbl_vlan_get_vlan(tbl_vlan_key_t *p_vlan_key)
{
    tbl_vlan_master_t *p_master = _g_p_tbl_vlan_master;

    if (GLB_MAX_VLAN_ID <= p_vlan_key->vid)
    {
        return NULL;
    }
    return (p_master->vlan_array[p_vlan_key->vid]);
}

char*
tbl_vlan_key_val2str(tbl_vlan_t *p_vlan, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlan->key.vid);
    return str;
}

int32
tbl_vlan_key_str2val(char *str, tbl_vlan_t *p_vlan)
{
    int32 ret = 0;

    p_vlan->key.vid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vlan_key_name_dump(tbl_vlan_t *p_vlan, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vlan->key.vid);
    return str;
}

char*
tbl_vlan_key_value_dump(tbl_vlan_t *p_vlan, char *str)
{
    sal_sprintf(str, "%u", p_vlan->key.vid);
    return str;
}

char*
tbl_vlan_field_name_dump(tbl_vlan_t *p_vlan, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLAN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlan_field_value_dump(tbl_vlan_t *p_vlan, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLAN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->key.vid);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAX_MAC_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->max_mac_count);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_CUR_MAC_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->cur_mac_count);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_INSTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->instance);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DHCSNOOPING, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->dhcsnooping);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ARPSNOOPING, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->arpsnooping);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_SECURITY_ACTION, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_vlan_security_action_strs, GLB_VLAN_SECURITY_ACTION_MAX, p_vlan->security_action));
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->enable);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAC_LEARNING_EN, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->mac_learning_en);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_OP_IFINDEX, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlan->op_ifindex);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MEMBER_PORT, field_id))
    {
        cdb_vlan_member_port_val2str(str, MAX_CMD_STR_LEN, &p_vlan->member_port);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_vlan->name);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ERPS_DOMAIN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->erps_domain_id);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->dot1x_guest_vlan_num);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_STATS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->stats_en);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_G8032_RING_ID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan->g8032_ring_id);
    }
    return str;
}

char**
tbl_vlan_table_dump(tbl_vlan_t *p_vlan, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlan_key_name_dump(p_vlan, buf));
    for(i=1; i<TBL_VLAN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlan_field_name_dump(p_vlan, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlan_field_value_dump(p_vlan, i, buf));
    }
    return str;
}

int32
tbl_vlan_field_value_parser(char *str, int32 field_id, tbl_vlan_t *p_vlan)
{
    if (FLD_MATCH(TBL_VLAN_FLD_KEY, field_id))
    {
        int32 ret;
        p_vlan->key.vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAX_MAC_COUNT, field_id))
    {
        int32 ret;
        p_vlan->max_mac_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_CUR_MAC_COUNT, field_id))
    {
        int32 ret;
        p_vlan->cur_mac_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_INSTANCE, field_id))
    {
        int32 ret;
        p_vlan->instance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DHCSNOOPING, field_id))
    {
        int32 ret;
        p_vlan->dhcsnooping = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ARPSNOOPING, field_id))
    {
        int32 ret;
        p_vlan->arpsnooping = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_SECURITY_ACTION, field_id))
    {
        cdb_enum_str2val(glb_vlan_security_action_strs, GLB_VLAN_SECURITY_ACTION_MAX, str);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_vlan->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAC_LEARNING_EN, field_id))
    {
        int32 ret;
        p_vlan->mac_learning_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_OP_IFINDEX, field_id))
    {
        int32 ret;
        p_vlan->op_ifindex = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MEMBER_PORT, field_id))
    {
        cdb_vlan_member_port_str2val(str, &p_vlan->member_port);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_NAME, field_id))
    {
        sal_strcpy(p_vlan->name, str);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ERPS_DOMAIN_ID, field_id))
    {
        int32 ret;
        p_vlan->erps_domain_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM, field_id))
    {
        int32 ret;
        p_vlan->dot1x_guest_vlan_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_STATS_EN, field_id))
    {
        int32 ret;
        p_vlan->stats_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_G8032_RING_ID, field_id))
    {
        int32 ret;
        p_vlan->g8032_ring_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vlan_table_parser(char** array, char* key_value,tbl_vlan_t *p_vlan)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlan_key_str2val(key_value, p_vlan));

    for(i=1; i<TBL_VLAN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlan_field_value_parser( array[j++], i, p_vlan));
    }

    return PM_E_NONE;
}

int32
tbl_vlan_dump_one(tbl_vlan_t *p_vlan, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLAN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_KEY].name,
            p_vlan->key.vid);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAX_MAC_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_MAX_MAC_COUNT].name,
            p_vlan->max_mac_count);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_CUR_MAC_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_CUR_MAC_COUNT].name,
            p_vlan->cur_mac_count);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_INSTANCE].name,
            p_vlan->instance);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DHCSNOOPING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_DHCSNOOPING].name,
            p_vlan->dhcsnooping);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ARPSNOOPING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_ARPSNOOPING].name,
            p_vlan->arpsnooping);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_SECURITY_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_FLD_SECURITY_ACTION].name, 
            cdb_enum_val2str(glb_vlan_security_action_strs, GLB_VLAN_SECURITY_ACTION_MAX, p_vlan->security_action));
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_ENABLE].name,
            p_vlan->enable);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MAC_LEARNING_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_MAC_LEARNING_EN].name,
            p_vlan->mac_learning_en);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_OP_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLAN_FLD_OP_IFINDEX].name,
            p_vlan->op_ifindex);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_MEMBER_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_FLD_MEMBER_PORT].name, 
            cdb_vlan_member_port_val2str(buf, MAX_CMD_STR_LEN, &p_vlan->member_port));
    }
    if (FLD_MATCH(TBL_VLAN_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_FLD_NAME].name,
            p_vlan->name);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_ERPS_DOMAIN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_ERPS_DOMAIN_ID].name,
            p_vlan->erps_domain_id);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM].name,
            p_vlan->dot1x_guest_vlan_num);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_STATS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_STATS_EN].name,
            p_vlan->stats_en);
    }
    if (FLD_MATCH(TBL_VLAN_FLD_G8032_RING_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_FLD_G8032_RING_ID].name,
            p_vlan->g8032_ring_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlan_master_t *p_master = _g_p_tbl_vlan_master;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        rc |= fn(p_db_vlan, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlan_master_t*
tbl_vlan_get_master()
{
    return _g_p_tbl_vlan_master;
}

tbl_vlan_master_t*
tbl_vlan_init_vlan()
{
    _g_p_tbl_vlan_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlan_master_t));
    return _g_p_tbl_vlan_master;
}


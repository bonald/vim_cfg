
#include "proto.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "cdb_data_cmp.h"

tbl_brg_global_t *_g_p_tbl_brg_global = NULL;

int32
tbl_brg_global_set_brg_global_field_sync(tbl_brg_global_t *p_brg_glb, tbl_brg_global_field_id_t field_id, uint32 sync)
{
    tbl_brg_global_t *p_db_brg_glb = _g_p_tbl_brg_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB:
        p_db_brg_glb->max_static_fdb = p_brg_glb->max_static_fdb;
        break;

    case TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB:
        p_db_brg_glb->max_l2mcast_fdb = p_brg_glb->max_l2mcast_fdb;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME:
        p_db_brg_glb->fdb_aging_time = p_brg_glb->fdb_aging_time;
        break;

    case TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME:
        p_db_brg_glb->oper_fdb_aging_time = p_brg_glb->oper_fdb_aging_time;
        break;

    case TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE:
        p_db_brg_glb->hw_learning_enable = p_brg_glb->hw_learning_enable;
        break;

    case TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING:
        p_db_brg_glb->ipsg_max_port_binding = p_brg_glb->ipsg_max_port_binding;
        break;

    case TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD:
        p_db_brg_glb->cur_ipv4_source_guard = p_brg_glb->cur_ipv4_source_guard;
        break;

    case TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE:
        p_db_brg_glb->port_isolate_mode = p_brg_glb->port_isolate_mode;
        break;

    case TBL_BRG_GLOBAL_FLD_INSTANCE:
        sal_memcpy(p_db_brg_glb->instance, p_brg_glb->instance, sizeof(p_brg_glb->instance));
        break;

    case TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID:
        sal_memcpy(p_db_brg_glb->pvlan_isolate_id, p_brg_glb->pvlan_isolate_id, sizeof(p_brg_glb->pvlan_isolate_id));
        break;

    case TBL_BRG_GLOBAL_FLD_MAX_INSTANCE:
        p_db_brg_glb->max_instance = p_brg_glb->max_instance;
        break;

    case TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID:
        p_db_brg_glb->max_pvlan_isolate_id = p_brg_glb->max_pvlan_isolate_id;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN:
        p_db_brg_glb->fdb_loop_errdis_en = p_brg_glb->fdb_loop_errdis_en;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE:
        p_db_brg_glb->fdb_loop_max_size = p_brg_glb->fdb_loop_max_size;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE:
        p_db_brg_glb->fdb_loop_add_rate = p_brg_glb->fdb_loop_add_rate;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT:
        p_db_brg_glb->fdb_loop_disc_cnt = p_brg_glb->fdb_loop_disc_cnt;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE:
        p_db_brg_glb->fdb_loop_curr_size = p_brg_glb->fdb_loop_curr_size;
        break;

    case TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP:
        p_db_brg_glb->trap_delay_up = p_brg_glb->trap_delay_up;
        break;

    case TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN:
        p_db_brg_glb->trap_delay_down = p_brg_glb->trap_delay_down;
        break;

    case TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING:
        p_db_brg_glb->fdb_all_flushing = p_brg_glb->fdb_all_flushing;
        break;

    case TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS:
        p_db_brg_glb->mac_based_vlan_class = p_brg_glb->mac_based_vlan_class;
        break;

    case TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS:
        p_db_brg_glb->ipv4_based_vlan_class = p_brg_glb->ipv4_based_vlan_class;
        break;

    case TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS:
        p_db_brg_glb->ipv6_based_vlan_class = p_brg_glb->ipv6_based_vlan_class;
        break;

    case TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE:
        p_db_brg_glb->max_mirror_mac_escape = p_brg_glb->max_mirror_mac_escape;
        break;

    case TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS:
        p_db_brg_glb->l2protocol_cos = p_brg_glb->l2protocol_cos;
        break;

    case TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT:
        p_db_brg_glb->vlan_mapping_entry_cnt = p_brg_glb->vlan_mapping_entry_cnt;
        break;

    case TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1:
        p_db_brg_glb->vlan_tpid_type1 = p_brg_glb->vlan_tpid_type1;
        break;

    case TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT:
        p_db_brg_glb->tpid_type1_ref_cnt = p_brg_glb->tpid_type1_ref_cnt;
        break;

    case TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2:
        p_db_brg_glb->vlan_tpid_type2 = p_brg_glb->vlan_tpid_type2;
        break;

    case TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT:
        p_db_brg_glb->tpid_type2_ref_cnt = p_brg_glb->tpid_type2_ref_cnt;
        break;

    case TBL_BRG_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_brg_glb, p_brg_glb, sizeof(tbl_brg_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BRG_GLOBAL, field_id, p_db_brg_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BRG_GLOBAL, field_id, p_db_brg_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_brg_global_set_brg_global_field(tbl_brg_global_t *p_brg_glb, tbl_brg_global_field_id_t field_id)
{
    return tbl_brg_global_set_brg_global_field_sync(p_brg_glb, field_id, TRUE);
}

tbl_brg_global_t*
tbl_brg_global_get_brg_global()
{
    return _g_p_tbl_brg_global;
}

int32
tbl_brg_global_key_str2val(char *str, tbl_brg_global_t *p_brg_glb)
{
    return PM_E_NONE;
}

char*
tbl_brg_global_key_name_dump(tbl_brg_global_t *p_brg_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BRG_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_brg_global_key_value_dump(tbl_brg_global_t *p_brg_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_brg_global_field_name_dump(tbl_brg_global_t *p_brg_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BRG_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BRG_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_brg_global_field_value_dump(tbl_brg_global_t *p_brg_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->max_static_fdb);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->max_l2mcast_fdb);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_aging_time);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->oper_fdb_aging_time);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->hw_learning_enable);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->ipsg_max_port_binding);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->cur_ipv4_source_guard);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_isolate_strs, GLB_PORT_ISOLATE_MAX, p_brg_glb->port_isolate_mode));
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_INSTANCE, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_brg_glb->instance, sizeof(p_brg_glb->instance)/4);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_brg_glb->pvlan_isolate_id, sizeof(p_brg_glb->pvlan_isolate_id)/4);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_INSTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->max_instance);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->max_pvlan_isolate_id);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_loop_errdis_en);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_loop_max_size);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_loop_add_rate);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_loop_disc_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_loop_curr_size);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->trap_delay_up);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->trap_delay_down);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->fdb_all_flushing);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->mac_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->ipv4_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->ipv6_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->max_mirror_mac_escape);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->l2protocol_cos);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->vlan_mapping_entry_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->vlan_tpid_type1);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->tpid_type1_ref_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->vlan_tpid_type2);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_brg_glb->tpid_type2_ref_cnt);
    }
    return str;
}

char**
tbl_brg_global_table_dump(tbl_brg_global_t *p_brg_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_brg_global_key_name_dump(p_brg_glb, buf));
    for(i=1; i<TBL_BRG_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_brg_global_field_name_dump(p_brg_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_brg_global_field_value_dump(p_brg_glb, i, buf));
    }
    return str;
}

int32
tbl_brg_global_field_value_parser(char *str, int32 field_id, tbl_brg_global_t *p_brg_glb)
{
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB, field_id))
    {
        int32 ret;
        p_brg_glb->max_static_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB, field_id))
    {
        int32 ret;
        p_brg_glb->max_l2mcast_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_aging_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME, field_id))
    {
        int32 ret;
        p_brg_glb->oper_fdb_aging_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE, field_id))
    {
        int32 ret;
        p_brg_glb->hw_learning_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING, field_id))
    {
        int32 ret;
        p_brg_glb->ipsg_max_port_binding = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD, field_id))
    {
        int32 ret;
        p_brg_glb->cur_ipv4_source_guard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE, field_id))
    {
        cdb_enum_str2val(glb_port_isolate_strs, GLB_PORT_ISOLATE_MAX, str);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_INSTANCE, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_brg_glb->instance, sizeof(p_brg_glb->instance)/4, err);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_brg_glb->pvlan_isolate_id, sizeof(p_brg_glb->pvlan_isolate_id)/4, err);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_INSTANCE, field_id))
    {
        int32 ret;
        p_brg_glb->max_instance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID, field_id))
    {
        int32 ret;
        p_brg_glb->max_pvlan_isolate_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_loop_errdis_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_loop_max_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_loop_add_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_loop_disc_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_loop_curr_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP, field_id))
    {
        int32 ret;
        p_brg_glb->trap_delay_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN, field_id))
    {
        int32 ret;
        p_brg_glb->trap_delay_down = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING, field_id))
    {
        int32 ret;
        p_brg_glb->fdb_all_flushing = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_brg_glb->mac_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_brg_glb->ipv4_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_brg_glb->ipv6_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE, field_id))
    {
        int32 ret;
        p_brg_glb->max_mirror_mac_escape = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS, field_id))
    {
        int32 ret;
        p_brg_glb->l2protocol_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT, field_id))
    {
        int32 ret;
        p_brg_glb->vlan_mapping_entry_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1, field_id))
    {
        int32 ret;
        p_brg_glb->vlan_tpid_type1 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT, field_id))
    {
        int32 ret;
        p_brg_glb->tpid_type1_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2, field_id))
    {
        int32 ret;
        p_brg_glb->vlan_tpid_type2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT, field_id))
    {
        int32 ret;
        p_brg_glb->tpid_type2_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_brg_global_table_parser(char** array, char* key_value,tbl_brg_global_t *p_brg_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BRG_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_brg_global_key_str2val(key_value, p_brg_glb));

    for(i=1; i<TBL_BRG_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_brg_global_field_value_parser( array[j++], i, p_brg_glb));
    }

    return PM_E_NONE;
}

int32
tbl_brg_global_dump_one(tbl_brg_global_t *p_brg_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BRG_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB].name,
            p_brg_glb->max_static_fdb);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB].name,
            p_brg_glb->max_l2mcast_fdb);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME].name,
            p_brg_glb->fdb_aging_time);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME].name,
            p_brg_glb->oper_fdb_aging_time);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE].name,
            p_brg_glb->hw_learning_enable);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING].name,
            p_brg_glb->ipsg_max_port_binding);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD].name,
            p_brg_glb->cur_ipv4_source_guard);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE].name, 
            cdb_enum_val2str(glb_port_isolate_strs, GLB_PORT_ISOLATE_MAX, p_brg_glb->port_isolate_mode));
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_INSTANCE].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_brg_glb->instance, sizeof(p_brg_glb->instance)/4));
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_brg_glb->pvlan_isolate_id, sizeof(p_brg_glb->pvlan_isolate_id)/4));
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAX_INSTANCE].name,
            p_brg_glb->max_instance);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID].name,
            p_brg_glb->max_pvlan_isolate_id);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN].name,
            p_brg_glb->fdb_loop_errdis_en);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE].name,
            p_brg_glb->fdb_loop_max_size);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE].name,
            p_brg_glb->fdb_loop_add_rate);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT].name,
            p_brg_glb->fdb_loop_disc_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE].name,
            p_brg_glb->fdb_loop_curr_size);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP].name,
            p_brg_glb->trap_delay_up);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN].name,
            p_brg_glb->trap_delay_down);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING].name,
            p_brg_glb->fdb_all_flushing);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS].name,
            p_brg_glb->mac_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS].name,
            p_brg_glb->ipv4_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS].name,
            p_brg_glb->ipv6_based_vlan_class);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE].name,
            p_brg_glb->max_mirror_mac_escape);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS].name,
            p_brg_glb->l2protocol_cos);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT].name,
            p_brg_glb->vlan_mapping_entry_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1].name,
            p_brg_glb->vlan_tpid_type1);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT].name,
            p_brg_glb->tpid_type1_ref_cnt);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2].name,
            p_brg_glb->vlan_tpid_type2);
    }
    if (FLD_MATCH(TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT].name,
            p_brg_glb->tpid_type2_ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_brg_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_brg_global_t *p_db_brg_glb = _g_p_tbl_brg_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_brg_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_brg_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_brg_global_t*
tbl_brg_global_init_brg_global()
{
    _g_p_tbl_brg_global = XCALLOC(MEM_TBL_BRG_GLOBAL, sizeof(tbl_brg_global_t));
    return _g_p_tbl_brg_global;
}


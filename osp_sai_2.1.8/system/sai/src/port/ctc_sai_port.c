#include <sai.h>
#include <sal.h>
#include <ctc_port.h>
#include <ctc_api.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_port.h>
#include <ctc_sai_acl.h>
#include <ctc_sai_policer.h>
#include <ctc_sai_isolation.h>
#include <ctc_sai_mirror.h>
#include <ctc_stats.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_vlanmap.h>
#include <ctc_sai_switch.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_cfm.h>

/* modified by liwh for bug 40004, 2016-08-04 */
#ifdef GREATBELT
//#define GLB_DFT_MAX_FRAME_SIZE_0    1632 
#endif
/* liwh end */

#ifdef GOLDENGATE
#include <sys_goldengate_datapath.h>
#include <sys_goldengate_chip.h>
#endif
#undef __MODULE__
#define __MODULE__ SAI_API_PORT

extern sai_status_t ctc_sai_policer_port_set_policer(sai_object_id_t port_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_policer_port_set_egress_policer(sai_object_id_t port_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_scheduler_port_set_scheduler(sai_object_id_t port_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_qos_map_port_set_ctc_qos_map(sai_object_id_t port_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_qos_map_port_set_default_map_init(sai_object_id_t port_id);

extern sai_status_t ctc_sai_policer_get_port_policer_id(sai_object_id_t port_id, sai_attribute_t *attr);

extern sai_status_t ctc_sai_policer_get_port_egress_policer_id(sai_object_id_t port_id, sai_attribute_t *attr);

extern uint64 ctc_sai_policer_get_cir_by_policer_id(uint32 policer_id);

extern uint32 ctc_sai_policer_get_meter_type_by_policer_id(uint32 policer_id);

extern uint32 ctc_sai_policer_get_mode_by_policer_id(uint32 policer_id);

sai_status_t ctc_sai_scheduler_get_port_scheduler_id(sai_object_id_t port_id, sai_attribute_t *attr);

extern sai_status_t ctc_sai_vlanmap_set_type_none(sai_object_id_t port_id);

extern sai_status_t ctc_sai_vlanmap_set_type_replace(sai_object_id_t port_id);

extern sai_status_t ctc_sai_vlanmap_set_type_basic_qinq(sai_object_id_t port_id);

extern sai_status_t ctc_sai_vlanmap_set_type_selective_qinq(sai_object_id_t port_id);

extern sai_status_t ctc_sai_vlanmap_set_default_entry(uint16 gportid, uint16 vid, uint8 flag, uint8 ingress);

extern uint32 hagt_openflow_udf_enable();

int32_t
ctc_sai_port_get_vlan_tag_ctrl (sai_object_id_t port_id, ctc_vlantag_ctl_t* pst_tag_ctrl);

static sai_status_t
ctc_sai_attr_check_speed(ctc_sai_attr_entry_list_t *pattr, void *data);

static ctc_sai_check_u32_range_t g_range_media_type =
{
    .min = SAI_PORT_MEDIA_TYPE_NOT_PRESENT,
    .max = SAI_PORT_MEDIA_TYPE_SFP_COPPER,
};

static ctc_sai_check_u32_range_t g_range_loopback =
{
    .min = SAI_PORT_INTERNAL_LOOPBACK_NONE,
    .max = SAI_PORT_INTERNAL_LOOPBACK_MAC,
};

static ctc_sai_check_u32_range_t g_range_learn_mode =
{
    .min = SAI_PORT_LEARN_MODE_DROP,
    .max = SAI_PORT_LEARN_MODE_CPU_LOG,
};

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id =	SAI_PORT_ATTR_TYPE,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_OPER_STATUS,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_HW_LANE_LIST,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_QUEUE_LIST,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_SUPPORTED_SPEED,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_PRIORITY_GROUP_LIST,
        .type = SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_SPEED,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_attr_check_speed,
            .func_parameter = NULL,
    }
    },
    {
        .id =	SAI_PORT_ATTR_FULL_DUPLEX_MODE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_AUTO_NEG_MODE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_ADMIN_STATE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_MEDIA_TYPE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PORT_MEDIA_TYPE_NOT_PRESENT,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_media_type,
    }
    },
    {
        .id =	SAI_PORT_ATTR_PORT_VLAN_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.u8 = 0,
    },
    {
        .id =	SAI_PORT_ATTR_VLANMAP_TYPE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.u16 = 1,
    },
    {
        .id =	SAI_PORT_ATTR_UNTAG_PVID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_VLAN_RANGE_EN,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_INGRESS_FILTERING,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = TRUE,
    },
    {
        .id =	SAI_PORT_ATTR_DROP_UNTAGGED,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_DROP_TAGGED,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_INTERNAL_LOOPBACK,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PORT_INTERNAL_LOOPBACK_NONE,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_loopback,
    }
    },
    {
        .id =	SAI_PORT_ATTR_FDB_LEARNING,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PORT_LEARN_MODE_HW,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_learn_mode,
    }
    },
    {
        .id =	SAI_PORT_ATTR_UPDATE_DSCP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_MTU,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.u32 = 1514,
    },
    {
        .id =	SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id =	SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PORT_FLOW_CONTROL_DISABLE,
    },
    {
        .id =	SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PACKET_ACTION_DROP,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_packet_action_range,
    }
    },
    {
        .id =	SAI_PORT_ATTR_INGRESS_MIRROR_SESSION,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_EGRESS_MIRROR_SESSION,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_POLICER_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_EGRESS_POLICER_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DEFAULT_TC,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_PRIORITY_GROUP_TO_PFC_PRIORITY_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =	SAI_PORT_ATTR_QOS_WRED_PROFILE_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_QOS_BUFFER_PROFILE_ID,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_META_DATA,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_ISOLATION_GROUP,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id =   SAI_PORT_ATTR_ISOLATION_DIRECTION,
        .type = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

char*
sai_sdb_mac_rx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_rx_stats_ptr_t *rx_ptr)
{
    ctc_stats_mac_rec_t *p_stats = NULL;
    
    if (NULL == rx_ptr)
    {
        sal_snprintf(str, str_len, "null");
        return str;
    }
    p_stats = (ctc_stats_mac_rec_t*)rx_ptr->p_stats;
    if (NULL == p_stats)
    {
        sal_snprintf(str, str_len, "null");
        return str;
    }
    
    sal_sprintf(str, "%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64,
        p_stats->good_ucast_pkts,
        p_stats->good_ucast_bytes,
        p_stats->good_mcast_pkts,
        p_stats->good_mcast_bytes,
        p_stats->good_bcast_pkts,
        p_stats->good_bcast_bytes
        );
    return str;
}

char*
sai_sdb_mac_tx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_tx_stats_ptr_t *tx_ptr)
{
    ctc_stats_mac_snd_t *p_stats = NULL;
    
    if (NULL == tx_ptr)
    {
        sal_snprintf(str, str_len, "null");
        return str;
    }
    p_stats = (ctc_stats_mac_snd_t*)tx_ptr->p_stats;
    if (NULL == p_stats)
    {
        sal_snprintf(str, str_len, "null");
        return str;
    }

    sal_sprintf(str, "%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64"#%"PRIu64,
        p_stats->good_ucast_pkts,
        p_stats->good_ucast_bytes,
        p_stats->good_mcast_pkts,
        p_stats->good_mcast_bytes,
        p_stats->good_bcast_pkts,
        p_stats->good_bcast_bytes
        );
    return str;
}

sai_status_t
ctc_sai_port_objectid_to_gport(sai_object_id_t port_oid, uint32 *pgport)
{
    uint32 ptype = 0;
    uint32 tid = 0;
    uint32 gport = 0;
    ptype = CTC_SAI_OBJECT_TYPE_GET(port_oid);
    if (SAI_OBJECT_TYPE_LAG == ptype)
    {
        tid = CTC_SAI_OBJECT_INDEX_GET(port_oid);
        gport = CTC_MAP_TID_TO_GPORT(tid);
    }
    else if (SAI_OBJECT_TYPE_PORT == ptype)
    {
        gport = CTC_SAI_OBJECT_INDEX_GET(port_oid);
    }
    else
    {
        gport = CTC_SAI_OBJECT_INDEX_GET(port_oid);
    }

    *pgport = gport;
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_gport_to_objectid(uint32 gport, sai_object_id_t *pst_port_oid)
{
    uint32 tid = 0;

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        tid = CTC_GPORT_LINKAGG_ID(gport);
        *pst_port_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
    }
    else
    {
        *pst_port_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, gport);
    }
    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_port_recover_vlan_ctl(uint32 gport)
{
    sai_object_id_t port_oid;
    ctc_vlantag_ctl_t tag_ctrl;
    int32 ret = 0;

    ctc_sai_port_gport_to_objectid(gport, &port_oid);

    sal_memset(&tag_ctrl, 0x0, sizeof(ctc_vlantag_ctl_t));
    ctc_sai_port_get_vlan_tag_ctrl (port_oid, &tag_ctrl);

    ret = ctc_port_set_vlan_ctl (gport, tag_ctrl);
    if(ret)
    {
        return ret;
    }

    return 0;
}

int32
ctc_sai_port_openflow_set_vlan_filter(uint32 gport, bool enable)
{
    sai_object_id_t port_oid;
    ctc_sai_port_gport_to_objectid(gport, &port_oid);
    
    ctc_sai_port_set_port_flag (port_oid, SAI_PORT_FLAG_HYBRID_VLAN_FILTER, enable);
    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_port_openflow_set_openflow_enable(uint32 gport, bool enable)
{
    sai_object_id_t port_oid;
    ctc_sai_port_gport_to_objectid(gport, &port_oid);
    
    ctc_sai_port_set_port_flag (port_oid, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE, enable);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_objectid_to_classid(sai_object_id_t port_oid, uint16 *pclassid)
{
    uint32 ptype = 0;
    uint32 tid = 0;
    uint16 classid = 0;
    ptype = CTC_SAI_OBJECT_TYPE_GET(port_oid);

    stbl_port_t *p_sdb_port = NULL;
    
    if (SAI_OBJECT_TYPE_LAG == ptype)
    {
        tid = CTC_SAI_OBJECT_INDEX_GET(port_oid);
        classid = CTC_MAP_TID_TO_GPORT(tid) & 0xff;
    }
    else if (SAI_OBJECT_TYPE_PORT == ptype)
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_oid);
        if (NULL == p_sdb_port)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        classid = p_sdb_port->port_class_id;
    }
    else
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_oid);
        if (NULL == p_sdb_port)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        classid = p_sdb_port->port_class_id;
    }

    *pclassid = classid;
    return SAI_STATUS_SUCCESS;
}

int32_t
_ctc_sai_acl_get_group_id_by_class_id(stbl_port_t *p_sdb_port, stbl_iter_args_t *pargs)
{
    ctc_sai_acl_get_group_id_by_class_id_node_t *acl_get_group_id_by_class_id = (ctc_sai_acl_get_group_id_by_class_id_node_t*)pargs->argv[0];

    if ((NULL == p_sdb_port) || (NULL == acl_get_group_id_by_class_id))
    {
        return 0;
    }

    if(p_sdb_port->port_class_id == acl_get_group_id_by_class_id->class_id)
    {
        if(CTC_INGRESS == acl_get_group_id_by_class_id->dir)
        {
            acl_get_group_id_by_class_id->group_id = p_sdb_port->group_id_igs;
        }
         else
        {
            acl_get_group_id_by_class_id->group_id = p_sdb_port->group_id_egs;
        }
    }

    return 0;
}

int32_t
ctc_sai_acl_get_group_id_by_class_id(sai_uint32_t class_id, sai_uint32_t dirction)
{
    ctc_sai_acl_get_group_id_by_class_id_node_t acl_get_group_id_by_class_id;

    acl_get_group_id_by_class_id.class_id = class_id;
    acl_get_group_id_by_class_id.dir = dirction;
    acl_get_group_id_by_class_id.group_id = 0;

    stbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &acl_get_group_id_by_class_id;
    
    stbl_port_iterate((STBL_ITER_CB_FUNC)_ctc_sai_acl_get_group_id_by_class_id, &args);
    return acl_get_group_id_by_class_id.group_id;
}
int32_t
ctc_sai_port_get_vlan_tag_ctrl (sai_object_id_t port_id, ctc_vlantag_ctl_t* pst_tag_ctrl)
{  
    stbl_port_t         *p_sdb_port = NULL;
    ctc_vlantag_ctl_t   tag_ctrl  = CTC_VLANCTL_DROP_ALL;
    sai_status_t        ret = SAI_STATUS_SUCCESS;

    /* get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

#if 1
    /*drop all, mirror related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_VLAN_FILTER))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }
#endif
     /*allow all, routed port, vpls. ROUTE_EN flag must check first*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ROUTED_EN)
        || CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FALG_L2VPN_PORT_MODE))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }

    /*drop all, mirror related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_MIRROR_DEST))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL;
        goto vlan_tag_out;
    }

    /* allow all, qinq related */
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }
    /*drop all, agg related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DIS_COLLECTOR)
        || (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && !CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_OPRED)))     
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL;
        goto vlan_tag_out;
    }

    /*allow all, tag ctrl*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ALLOW_TAGGED)
        && CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ALLOW_UNTAGGED))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }

    /*allow all tagged packet*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ALLOW_TAGGED))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL_UNTAGGED;
        goto vlan_tag_out;
    }
    /*allow all untagged packet*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ALLOW_UNTAGGED))
    {
        /* the cvlan tagged packet will be forwarded by access port */
        tag_ctrl = CTC_VLANCTL_DROP_ALL_SVLAN_TAG;
        goto vlan_tag_out;
    }

vlan_tag_out:
    *pst_tag_ctrl = tag_ctrl;
    
    return ret;
}

int32_t
ctc_sai_port_get_receive_en(sai_object_id_t port_id, bool * p_rx_en) 
{
    stbl_port_t     *p_sdb_port = NULL;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    bool rx_en = TRUE;

    /* get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    /* running flags*/
    if (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_RUNNING))
    {
        rx_en = FALSE;
        goto rx_out;
    }

    /*dot1x related*/
    if ((CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DOT1X_UNAUTHED)
        && CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DOT1X_DIR_IN)))
    {
        rx_en = FALSE;
        goto rx_out;
    }

    /* efm related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_EFM_PAR_ACTION_DISCARD))
    {
        rx_en = FALSE; 
        goto rx_out;
    }

    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_MIRROR_DEST))
    {
        rx_en = FALSE; 
        goto rx_out;
    }
    
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_OPRED)))
    {
        rx_en = FALSE; 
        goto rx_out;
    }

    if(CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_IS_ACTIVE)))
    {
        rx_en = FALSE;
    }
rx_out:
    *p_rx_en = rx_en;
    
    return ret;
}


int32_t
ctc_sai_port_get_lag_bind_en(sai_object_id_t port_id, bool * p_lag_bind_en) 
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool lag_bind_en = FALSE;

    /* get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_OPRED))
        && (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_IS_ACTIVE)))
    {
        lag_bind_en = TRUE;
        goto rx_out;
    }
    
rx_out:
    *p_lag_bind_en = lag_bind_en;
    
    return ret;
}


int32_t
ctc_sai_port_get_transmit_en(sai_object_id_t port_id, bool* p_tx_en)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool tx_en = TRUE;

    /*get port flags*/ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    /*3. dot1x related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DOT1X_UNAUTHED)
        && CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DOT1X_DIR_OUT))
    {
        tx_en = FALSE;
        goto tx_out;
    }

    /*mirror dest related*/
    if (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_RUNNING)
        || CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_MIRROR_DEST))
    {
        tx_en = FALSE;
        goto tx_out;
    }


    /*efm related*/
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_EFM_MUX_ACTION_DISCARD))
    {
       tx_en = FALSE; 
       goto tx_out;
    }
  
    /*agg related*/   
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_DIS_DISTRIBUTOR))
    {
        tx_en = FALSE;
        goto tx_out;
    } 
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_OPRED)))
    {
        tx_en = FALSE;
    }

    if(CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGGREGATED)
        && (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_AGG_IS_ACTIVE)))
    {
        tx_en = FALSE;
    }
tx_out:
    *p_tx_en = tx_en;
    
    return ret;
}

int32_t
ctc_sai_port_get_dot1q_type (sai_object_id_t port_id, ctc_dot1q_type_t* pst_dot1q_type)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;   

    *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;

    /*get port flags*/ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

#if 0
    if (CTC_FLAG_ISSET(pst_port->flag, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;        
    }
#endif
    else if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ROUTED_EN))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_NONE; 
        /*no openflow nexthop in gb chip, out port of flow obey dot1q set*/
        if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE))
        {
             *pst_dot1q_type = CTC_DOT1Q_TYPE_SVLAN;
        }
    }
    else if (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_ALLOW_TAGGED))
    {
        //#ifdef GOLDENGATE
        *pst_dot1q_type = CTC_DOT1Q_TYPE_CVLAN;
        //#else
        /*no openflow nexthop in gb chip, out port of flow obey dot1q set*/
        if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE))
        {
            *pst_dot1q_type = CTC_DOT1Q_TYPE_NONE;
        }
        //#endif
    }
    /* strip outer tag when packet is sent from qinq port */
    else if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_CVLAN;
        /* qinq egress scl packet transmit out with double tag */
        //*pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;
    }
    else
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;
    }

    return ret;
}


int32_t
ctc_sai_port_get_ingress_vlan_filter (sai_object_id_t port_id, bool* ingress_vlan_filter)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool ivf = TRUE; 

    /*get port flags*/ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_VLAN_FILTER))
    {
        ivf  = FALSE;    
        goto OUT;
    }
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FALG_L2VPN_ENABLE))
    {
        ivf  = FALSE;    
        goto OUT;
    }
    if (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_INGRESS_FILTER_EN))
    {
        ivf  = FALSE;    
        goto OUT;
    }

OUT:
    *ingress_vlan_filter = ivf;

    return ret;
}


int32_t
ctc_sai_port_get_egress_vlan_filter (sai_object_id_t port_id, bool* egress_vlan_filter)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool evf = TRUE;    

    /*get port flags*/ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_VLAN_FILTER))
    {
        evf  = FALSE;    
        goto OUT;
    }
    
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FALG_L2VPN_ENABLE))
    {
        evf  = FALSE;    
        goto OUT;
    }

    if (!CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_EGRESS_FILTER_EN))
    {
        evf  = FALSE;    
        goto OUT;
    }

OUT:
    *egress_vlan_filter = evf;

    return ret;
}


int32_t
ctc_sai_port_get_src_discard (sai_object_id_t port_id, bool* src_discard)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    bool src_dis = FALSE;    

    /*get port flags*/ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }
   
    if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_PHY_LPBK_EXTERNAL))
    {
        src_dis  = TRUE;
        goto OUT;
    }

OUT:
    *src_discard = src_dis;

    return ret;
}

int32_t
ctc_sai_port_set_lag_bind_en(sai_object_id_t port_id, bool lag_bind_en)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_lag_notifier_t lag_nf;
#ifdef GREATBELT 
    uint32 vid;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
#endif

    /*get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }
    
    /*lag bind enable*/
    if (0 != CTC_SAI_OBJECT_TYPE_GET(p_sdb_port->lag_id))
    {
        if(lag_bind_en)
        {
            ctc_linkagg_add_port(CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->lag_id),
                  CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->key.port_oid));
#ifndef GREATBELT 
            if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                ctc_security_learn_limit_t  old_limit;
                ctc_security_learn_limit_t  new_limit;

                sal_memset(&old_limit, 0, sizeof(old_limit));
                sal_memset(&new_limit, 0, sizeof(new_limit));
                
                old_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
                old_limit.gport      = CTC_SAI_OBJECT_INDEX_GET(port_id);

                ret = ctc_mac_security_get_learn_limit(&old_limit);

                if(CTC_E_NONE != ret)
                {
                    return ctc_sai_get_error_from_sdk_error(ret);
                }

                new_limit = old_limit;
                new_limit.limit_num = p_sdb_port->mac_limit_num;
                ret = ctc_mac_security_set_learn_limit(&new_limit);
                if(CTC_E_NONE != ret)
                {
                    return ctc_sai_get_error_from_sdk_error(ret);
                }
            } 
#endif

/* modified by liwh for bug 54188, 2019-11-04 */
#ifndef TSINGMA
           ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->key.port_oid), 
                CTC_PORT_PROP_LINKAGG_FAILOVER_EN, 1);
#endif
/* liwh end*/

#ifdef GREATBELT
            /*For GB, once port is bind to lag, it should removed from vlan default entry*/
            CTC_BMP_ITER_BEGIN(p_sdb_port->vlan_bmp, vid)
            {
                l2dflt_addr.fid = vid;
                l2dflt_addr.l2mc_grp_id = vid;
                l2dflt_addr.member.mem_port = CTC_SAI_OBJECT_INDEX_GET(port_id);
                ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
                
            }
            CTC_BMP_ITER_END(p_sdb_port->vlan_bmp, vid);
#endif
            lag_nf.op       = CTC_SAI_LAG_NF_NEW_LAG_MEMBER;
            lag_nf.lag_id   = p_sdb_port->lag_id;
            lag_nf.lag_member_port_id   = p_sdb_port->key.port_oid;
            ctc_sai_lag_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_NEW_LAG_MEMBER, &lag_nf);

        }
        else
        {
            ctc_linkagg_remove_port(CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->lag_id),
              CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->key.port_oid));

#ifdef GREATBELT
            /*For GB, once port is bind to lag, it should removed from vlan default entry*/
            CTC_BMP_ITER_BEGIN(p_sdb_port->vlan_bmp, vid)
            {
                l2dflt_addr.fid = vid;
                l2dflt_addr.l2mc_grp_id = vid;
                l2dflt_addr.member.mem_port = CTC_SAI_OBJECT_INDEX_GET(port_id);
                ctc_l2_add_port_to_default_entry(&l2dflt_addr);
                
            }
            CTC_BMP_ITER_END(p_sdb_port->vlan_bmp, vid);
#endif
            lag_nf.op       = CTC_SAI_LAG_NF_DEL_LAG_MEMBER;
            lag_nf.lag_id   = p_sdb_port->lag_id;
            lag_nf.lag_member_port_id   = p_sdb_port->key.port_oid;
            ctc_sai_lag_evnet_notifier_call_notifiers(CTC_SAI_LAG_NF_DEL_LAG_MEMBER, &lag_nf);
        }
    }
    return ret;
}

int32
ctc_sai_check_port_is_l3if(sai_object_id_t port_oid)
{
    stbl_rif_master_t *p_master = stbl_rif_get_master();
    stbl_rif_t *p_db_rif = NULL;
    uint32 rif_id = 0;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (rif_id = 0; rif_id < GLB_MAX_L3IF_NUM; rif_id++)
    {
        p_db_rif = p_master->array[rif_id];
        if (NULL == p_db_rif)
        {
            continue;
        }
        if(p_db_rif->port_oid == port_oid)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32_t
ctc_sai_set_port_recieve_change_affect_openflow(stbl_port_t *p_sdb_port, uint32 gportid, uint32 receive_en)
{
#ifdef USW
    #ifdef OFPRODUCT
    if(receive_en)
    {
        if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE))
        {
             if(hagt_openflow_udf_enable())
            {
                 ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, CTC_INGRESS, CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT);
            }
             else
            {
                 ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, CTC_INGRESS, CTC_ACL_TCAM_LKUP_TYPE_L2_L3);
            }
        }
    }
    else
    {
        if (CTC_FLAG_ISSET(p_sdb_port->flag, SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE))
        {
            ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0);
        }
    }
    #endif
#endif
    return 0;
}

int32_t
 ctc_sai_port_set_port_flag (sai_object_id_t port_id, ctc_sai_port_flag_t flag, bool enable)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_port_filed_info_t curr_status; 
    uint32 gportid = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_SUCCESS;
    }

    gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);

    /*get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }
    
    /*set port flags*/
    if (enable)
    {
        CTC_SET_FLAG(p_sdb_port->flag, flag);
    }
    else
    {
        CTC_UNSET_FLAG(p_sdb_port->flag, flag);
    }

    /*get vlan tag ctrl, transmit enable, and dot1q type*/
    ctc_sai_port_get_vlan_tag_ctrl (port_id, &curr_status.tag_ctrl);
    ctc_sai_port_get_transmit_en (port_id, &curr_status.transmit_en);
    ctc_sai_port_get_receive_en (port_id, &curr_status.receive_en);
    ctc_sai_port_get_dot1q_type (port_id, &curr_status.dot1q_type);
    ctc_sai_port_get_ingress_vlan_filter(port_id, &curr_status.ingress_vlan_filter);
    ctc_sai_port_get_egress_vlan_filter(port_id, &curr_status.egress_vlan_filter);
    ctc_sai_port_get_src_discard(port_id, &curr_status.src_discard);
    ctc_sai_port_get_lag_bind_en(port_id, &curr_status.lag_bind_en);
    
    if (curr_status.tag_ctrl != p_sdb_port->tag_ctrl)
    {
        ret = ctc_port_set_vlan_ctl (gportid, curr_status.tag_ctrl);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->tag_ctrl = curr_status.tag_ctrl;
    }

    /* Added by kcao for bug 44867 2017-08-02, should set LAG member port to LAG, and then set TX/RX */
    if (curr_status.lag_bind_en != p_sdb_port->lag_bind_en)
    {
        /*added by wangl for bug 48166*/
        if(!curr_status.lag_bind_en)
        {
            if (curr_status.transmit_en != p_sdb_port->transmit_en)
            {
                ret = ctc_port_set_transmit_en(gportid, curr_status.transmit_en);
                if(ret)
                {
                    return ret;
                }
                p_sdb_port->transmit_en = curr_status.transmit_en;
            }
            
            if (curr_status.receive_en != p_sdb_port->receive_en)
            {
                ctc_sai_set_port_recieve_change_affect_openflow(p_sdb_port, gportid, curr_status.receive_en);
                ret =ctc_port_set_receive_en(gportid, curr_status.receive_en);
                if(ret)
                {
                    return ret;
                }
                p_sdb_port->receive_en = curr_status.receive_en;
            }
        }
        ret = ctc_sai_port_set_lag_bind_en(port_id, curr_status.lag_bind_en);
        if(ret)
        {
            return ret;
        }
        if(ctc_sai_check_port_is_l3if(port_id))
        {
            if(curr_status.lag_bind_en)
            {
               ctc_port_set_phy_if_en(gportid, curr_status.lag_bind_en);

               /* support nd modified by liwh for bug 47547, 2018-07-24 
                   if not set bridge_en to true, nd packets from agg can't be sent to CPU */
               ctc_port_set_property (gportid, CTC_PORT_PROP_BRIDGE_EN, TRUE);
               /* liwh end */
            }
        }
        
        if(ret)
        {
            return ret;
        }
        p_sdb_port->lag_bind_en = curr_status.lag_bind_en;
    }
   
    if (curr_status.transmit_en != p_sdb_port->transmit_en)
    {
        ret = ctc_port_set_transmit_en(gportid, curr_status.transmit_en);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->transmit_en = curr_status.transmit_en;
    }
    
    if (curr_status.receive_en != p_sdb_port->receive_en)
    {
        ctc_sai_set_port_recieve_change_affect_openflow(p_sdb_port, gportid, curr_status.receive_en);
        ret =ctc_port_set_receive_en(gportid, curr_status.receive_en);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->receive_en = curr_status.receive_en;
    }

    if (curr_status.dot1q_type != p_sdb_port->dot1q_type)
    {
        ret = ctc_port_set_dot1q_type(gportid, curr_status.dot1q_type);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->dot1q_type = curr_status.dot1q_type;
    }    

    if (curr_status.ingress_vlan_filter != p_sdb_port->ingress_vlan_filter)
    {
        ret = ctc_port_set_vlan_filter_en(gportid, CTC_INGRESS, curr_status.ingress_vlan_filter);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->ingress_vlan_filter = curr_status.ingress_vlan_filter;
    }
    
    if (curr_status.egress_vlan_filter != p_sdb_port->egress_vlan_filter)
    {
        ret = ctc_port_set_vlan_filter_en(gportid, CTC_EGRESS, curr_status.egress_vlan_filter);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->egress_vlan_filter= curr_status.egress_vlan_filter;
    }        

    if (curr_status.src_discard != p_sdb_port->src_discard)
    {
        ret = ctc_port_set_srcdiscard_en(gportid, curr_status.src_discard);
        if(ret)
        {
            return ret;
        }
        p_sdb_port->src_discard = curr_status.src_discard;
    }

/* system modified by liwh for bug 53010, 2019-08-14 */
    if (SAI_PORT_FLAG_RUNNING == flag)
    {        
#ifdef GREATBELT
        ctc_sai_cfm_set_lmep_cc_enable_to_sdk_for_port(gportid, enable);
#endif

        /* add by wulx for bug 53996, 2019-10-23 */
#ifndef GREATBELT
#ifndef GOLDENGATE
        ret = ctc_port_set_port_en(gportid, enable);
        if(ret)
        {
            return ret;
        }
#endif
#endif
    }
/* liwh end */


    return ret;   
}

int32_t
ctc_sai_port_set_port_sflow(sai_object_id_t port_id, bool dir, uint16 threshold, bool enable)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    uint32                gportid = 0;
    
    
    gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);

    if (CTC_IS_LINKAGG_PORT(gportid))
    {
        return SAI_STATUS_SUCCESS;
    }
    
    ret = ctc_port_set_random_log_en(gportid, dir, enable);
    if (ret)
    {
        CTC_SAI_DEBUG("ctc_sai_port_set_port_sflow return ret %d for port %d\n", ret, port_id);    
        return ret;
    }

    ret = ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_RANDOM_LOG_RATE, dir, threshold);
    if (ret)
    {
        CTC_SAI_DEBUG("ctc_port_set_direction_property CTC_PORT_DIR_PROP_RANDOM_LOG_RATE return ret %d for port %d\n", 
            ret, port_id);    
        return ret;
    }
    
    if (!enable)
    {
        if (CTC_INGRESS == dir)
        {
            ret = ctc_stats_clear_port_log_stats(gportid, CTC_INGRESS);
        }
        
        if (CTC_EGRESS == dir)
        {
            ret = ctc_stats_clear_port_log_stats(gportid, CTC_EGRESS);
        }

        if (ret)
        {
            CTC_SAI_DEBUG("ctc_stats_clear_port_log_stats return ret %d for port %d\n", 
                ret, port_id);    
            return ret;
        }
    }

    return ret;
}

int32_t
ctc_sai_port_set_phy_en (sai_object_id_t port_id, bool enable)
{
    stbl_port_t *p_sdb_port = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    stbl_rif_t  *p_sdb_rif  = NULL;
    uint32      gportid = 0;

    /* get port flags */ 
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    p_sdb_rif = ctc_routerintf_get_by_port_oid(port_id);
    if (NULL != p_sdb_rif) 
    {
        ctc_sai_port_objectid_to_gport(port_id, &gportid);
        if (enable)
        {
            ctc_port_set_phy_if_en(gportid, TRUE);
        }
        else
        {
            ctc_port_set_phy_if_en(gportid, FALSE);
        }
        ctc_port_set_property(gportid, CTC_PORT_PROP_BRIDGE_EN, 1);
    }

    return ret;
}

int32_t
ctc_sai_port_set_erps_pdu_action (sai_object_id_t port_id, uint8 action)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;  
    uint32 gportid;

    gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    ret = ctc_l2pdu_set_port_action (gportid, CTC_L2PDU_ACTION_INDEX_ERPS, action);
    if (ret)
    {
        CTC_SAI_DEBUG("ctc_sai_port_set_erps_pdu_action return ret %d for port %d\n", ret, port_id);    
    }

    return ret; 
}

int32_t
ctc_sai_port_set_dot1x_pdu_action (sai_object_id_t port_id, uint8 action)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;  
    uint32 gportid;

    gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    ret = ctc_l2pdu_set_port_action (gportid, CTC_L2PDU_ACTION_INDEX_EAPOL, action);
    if (ret)
    {
        CTC_SAI_DEBUG("ctc_sai_port_set_dot1x_pdu_action return ret %d for port %d\n", ret, port_id);    
    }

    return ret; 
}

int32_t
ctc_sai_port_get_port_flag(sai_object_id_t port_id, ctc_sai_port_flag_t* p_flag)
{
    stbl_port_t           *p_sdb_port = NULL;
    sai_status_t          ret = SAI_STATUS_SUCCESS;  
    uint32 gportid;


    gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);
    if (CTC_IS_LINKAGG_PORT(gportid))
    {
        return SAI_STATUS_SUCCESS;
    }

    /*get port flags*/
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    *p_flag = p_sdb_port->flag;

    return ret;    
}

sai_status_t
ctc_sai_for_each_port_object(port_object_cb fn, void* pdata)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    uint8_t                     lchip_num = 0;
    uint8_t                     lchip = 0;
    uint32_t                    lport = 0;
    uint32                      gport = 0;
    uint8                       gchip_id = 0;
#ifdef GOLDENGATE
    sys_datapath_lport_attr_t*  port_attr = NULL;
    int32_t                     sdk_ret = CTC_E_NONE;
#endif
    sai_object_id_t             port_id;
#ifdef GREATBELT 
    uint32_t                    max_count = CTC_MAX_PHY_PORT;
#else
    uint32_t                    max_count = CTC_MAX_LPORT;
#endif

#ifdef USW   /*System modify for copp templetely need to update d2 port 2018-05-30*/
    max_count = 64;
#endif
    
    ctc_get_local_chip_num(&lchip_num);
    if(lchip_num == 0)
    {
        lchip_num = 1;
    }

    for (lchip = 0; lchip < lchip_num; lchip++)
    {
        for (lport = 0; lport < max_count; lport++)
        {
            CTC_ERROR_RETURN(ctc_get_gchip_id(lchip, &gchip_id));

            gport = (gchip_id << 8) | lport;
#ifdef GOLDENGATE
            sdk_ret = sys_goldengate_common_get_port_capability(0, lport, &port_attr);
            if (sdk_ret < 0)
            {
                continue;
            }

            if (port_attr->port_type != SYS_DATAPATH_NETWORK_PORT)
            {
                continue;
            }
#endif
            

            port_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT,(CTC_MAP_LPORT_TO_GPORT(0, gport)));//TODO, temporarily use 0 as gchip
            ret = fn(port_id,pdata);
            if(ret < 0)
            {
                goto out;
            }
        }
    }

out:
    return ret;
}

sai_status_t
ctc_sai_port_get_type(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_operational_status(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    bool                islink_up;

    ret = ctc_port_get_mac_link_up(CTC_SAI_OBJECT_INDEX_GET(port_id),&islink_up);

    if(CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    if(islink_up)
    {
        attr->value.s32 = SAI_PORT_OPER_STATUS_UP;
    }
    else
    {
        attr->value.s32 = SAI_PORT_OPER_STATUS_DOWN;
    }


    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_hw_lane_list(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_supported_breakout_mode(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_current_breakout_mode(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_port_set_crc_error_check_en(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t  ret = 0;
    uint32   crc_error_check_en = attr[0].value.u32;

    ret = ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_PORT_PROP_ERROR_CHECK, crc_error_check_en);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_crc_recalculation_en(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t  ret = 0;
    uint32   crc_recalculation_en = attr[0].value.u32;

    ret = ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_PORT_PROP_CRC_RECALC_EN, crc_recalculation_en);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_speed(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;
    ctc_port_speed_t    speed_mode = CTC_PORT_SPEED_1G;

    switch(attr->value.u32)
    {
    case CTC_SAI_PORT_SPEED_10M:        /* 10M */
        speed_mode = CTC_PORT_SPEED_10M;
        break;
    case CTC_SAI_PORT_SPEED_100M:       /* 100M */
        speed_mode = CTC_PORT_SPEED_100M;
        break;
    case CTC_SAI_PORT_SPEED_1000M:      /* 1G */
        speed_mode = CTC_PORT_SPEED_1G;
        break;
    case CTC_SAI_PORT_SPEED_2G5:      /* 2.5G */
        speed_mode = CTC_PORT_SPEED_2G5;
        break;
    case CTC_SAI_PORT_SPEED_10G:     /* 10G */
        speed_mode = CTC_PORT_SPEED_10G;
        break;
    case CTC_SAI_PORT_SPEED_40G:     /* 40G */
        speed_mode = CTC_PORT_SPEED_40G;
        break;
    case CTC_SAI_PORT_SPEED_100G:     /* 100G */
        speed_mode = CTC_PORT_SPEED_100G;
        break;
    }

    ret = ctc_port_set_speed(CTC_SAI_OBJECT_INDEX_GET(port_id), speed_mode);
#ifdef TAPPRODUCT    /* tap need re-config crc by speed change */
    ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_PORT_PROP_CRC_CHECK_EN, FALSE);
#endif
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_speed(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    ctc_port_speed_t    speed_mode = CTC_PORT_SPEED_1G;

    sal_memset(&speed_mode,0,sizeof(speed_mode));

    ret = ctc_port_get_speed(CTC_SAI_OBJECT_INDEX_GET(port_id), &speed_mode);

    if(CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    switch(speed_mode)
    {
    case CTC_PORT_SPEED_10M:        /* 10M */
        attr->value.u32 = CTC_SAI_PORT_SPEED_10M;
        break;
    case CTC_PORT_SPEED_100M:       /* 100M */
        attr->value.u32 = CTC_SAI_PORT_SPEED_100M;
        break;
    case CTC_PORT_SPEED_1G:      /* 1G */
        attr->value.u32 = CTC_SAI_PORT_SPEED_1000M;
        break;
    case CTC_PORT_SPEED_2G5:      /* 2.5G */
        attr->value.u32 = CTC_SAI_PORT_SPEED_2G5;
        break;
    case CTC_PORT_SPEED_10G:     /* 10G */
        attr->value.u32 = CTC_SAI_PORT_SPEED_10G;
        break;
    case CTC_PORT_SPEED_40G:     /* 40G */
        attr->value.u32= CTC_SAI_PORT_SPEED_40G;
        break;
    case CTC_PORT_SPEED_100G:     /* 100G */
        attr->value.u32 = CTC_SAI_PORT_SPEED_100G;
        break;
    default:
        break;
    }


    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_admin_state(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;

    ret = ctc_port_set_mac_en(CTC_SAI_OBJECT_INDEX_GET(port_id), attr->value.booldata);
    if (CTC_E_PORT_MAC_IS_DISABLE == ret)
    {
        /* shutdown downed port will return this error, ignore */
        ret = CTC_E_NONE;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_admin_state(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    bool                port_enable;

    ret = ctc_port_get_mac_en(CTC_SAI_OBJECT_INDEX_GET(port_id), &port_enable);

    attr->value.booldata = port_enable;

    return ctc_sai_get_error_from_sdk_error(ret);
}


sai_status_t
ctc_sai_port_set_media_type(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_media_type(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_port_set_default_vlan(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;

    ret = ctc_port_set_default_vlan(CTC_SAI_OBJECT_INDEX_GET(port_id), attr->value.u16);

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_default_vlan(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    uint16_t            vlan_id;

    ret = ctc_port_get_default_vlan(CTC_SAI_OBJECT_INDEX_GET(port_id), &vlan_id);

    attr->value.u16 = vlan_id;

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_default_vlan_prioroty(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_default_vlan_prioroty(sai_object_id_t port_id,
                                 sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_vlanmap_type(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    if (SAI_VLAN_MAP_TYPE_NONE == attr->value.u16)
    {
        ctc_sai_vlanmap_set_type_none(port_id);
    }
    else if (SAI_VLAN_MAP_TYPE_VLAN_REPLACE == attr->value.u16)
    {
        ctc_sai_vlanmap_set_type_replace(port_id);
    }
    else if (SAI_VLAN_MAP_TYPE_BASIC_QINQ == attr->value.u16)
    {
        ctc_sai_vlanmap_set_type_basic_qinq(port_id);
    }
    else if (SAI_VLAN_MAP_TYPE_SELECTIVE_QINQ == attr->value.u16)
    {
        ctc_sai_vlanmap_set_type_selective_qinq(port_id);
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_vlanmap_type(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_vlanmap_default_fwd(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;
    uint16              def_fwd_mode = 0;
    
    def_fwd_mode = attr->value.u16;

    if (0 == def_fwd_mode)
    {
        /* vlan translation default discard when vm tale miss matched */
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DISCARD, TRUE));
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DISCARD, FALSE));
    }
    else if (1 == def_fwd_mode)
    {
        /* vlan translation default forward when vm tale miss matched */
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, TRUE));
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, FALSE));
    }
    else if (2 == def_fwd_mode)
    {
        /* qinq default discard */
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DISCARD, TRUE));
    }
    else if (3 == def_fwd_mode)
    {
        /* qinq default forward and do nothing */
        CTC_ERROR_RETURN(ctc_sai_vlanmap_set_default_entry(port_id, 0, CTC_VLAN_MISS_ACTION_DO_NOTHING, TRUE));
    }
    else
    {
        /* there is out-of-range entry exist for selective qinq skip configure here */
    }
    
    return ret;
}

sai_status_t
ctc_sai_port_get_vlanmap_default_fwd(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;;
}

sai_status_t
ctc_sai_port_set_untag_pvid(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t    ret = SAI_STATUS_SUCCESS;
    bool       untag_dft_vid = FALSE;
    bool       untag_svlan   = FALSE;
    uint32     gport = 0;
    ctc_sai_port_objectid_to_gport(port_id, &gport);
    
    if (SAI_PORT_UNTAG_SVLAN == attr->value.u16)
    {
        untag_dft_vid = FALSE;
        untag_svlan = FALSE;
    }
    else if (SAI_PORT_TAG_SVLAN == attr->value.u16)
    {
        untag_dft_vid = TRUE;
        untag_svlan = FALSE;
    }
    else if (SAI_PORT_UNTAG_PVID == attr->value.u16)
    {
        untag_dft_vid = FALSE;
        untag_svlan = TRUE;
    }
    else
    {
        untag_dft_vid = TRUE;
        untag_svlan = TRUE;
    }

    CTC_ERROR_RETURN(ctc_port_set_untag_dft_vid(gport, untag_dft_vid, untag_svlan));

    return ret;
}

sai_status_t
ctc_sai_port_get_untag_pvid(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_vlan_range_en(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t  ret = 0;
    uint32   gportid = 0;
    bool     enable  = FALSE;
    ctc_vlan_range_info_t vrange_info;

    ctc_sai_port_objectid_to_gport(port_id, &gportid);
    sal_memset(&vrange_info, 0, sizeof(vrange_info));
    vrange_info.direction = CTC_INGRESS;
    vrange_info.vrange_grpid = (attr->value.u32 & 0xFF) - 1;
    enable = (attr->value.u32 >> 16) & 0xF;

    ctc_port_set_vlan_range(gportid, &vrange_info, enable);

    return ret;
}

sai_status_t
ctc_sai_port_get_vlan_range_en(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_ingress_filtering(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;
    bool                enable = false;
    
    enable = attr->value.booldata;
    ctc_sai_port_set_port_flag(port_id, SAI_PORT_FLAG_INGRESS_FILTER_EN, enable);
    ctc_sai_port_set_port_flag(port_id, SAI_PORT_FLAG_EGRESS_FILTER_EN, enable);

    return ret;
}

sai_status_t
ctc_sai_port_get_ingress_filtering(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    bool                vlan_filter;
    uint32              gport = 0;
    
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    ret = ctc_port_get_vlan_filter_en(gport,CTC_INGRESS,&vlan_filter);

    attr->value.booldata = vlan_filter;

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_drop_untagged(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = 0;
#if 0
    ctc_vlantag_ctl_t   mode = CTC_VLANCTL_DROP_ALL_UNTAGGED;

    if(false == attr->value.booldata)
    {
        mode = CTC_VLANCTL_ALLOW_ALL_PACKETS;
    }

    ret = ctc_port_set_vlan_ctl(CTC_SAI_OBJECT_INDEX_GET(port_id), mode);
#endif 
    ret = ctc_sai_port_set_port_flag(port_id,SAI_PORT_FLAG_ALLOW_UNTAGGED, (!attr->value.booldata));

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_drop_untagged(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    ctc_vlantag_ctl_t   mode = CTC_VLANCTL_DROP_ALL_UNTAGGED;

    ret = ctc_port_get_vlan_ctl(CTC_SAI_OBJECT_INDEX_GET(port_id), &mode);

    if(CTC_VLANCTL_DROP_ALL_UNTAGGED == mode)
    {
        attr->value.booldata = true;
    }
    else
    {
        attr->value.booldata = false;
    }

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_untagged_vlan(sai_object_id_t port_id,
                                 const sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    uint16_t vid = 0;
    uint32_t untagged_vlan = 0;
    uint32_t gport = 0;     
    int32_t  ret = 0;
    bool tagged = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    untagged_vlan = attr[0].value.u32;
    vid = attr[1].value.u16;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    tagged = (untagged_vlan) ? 0 : 1;
    ret = ctc_vlan_set_tagged_port(vid, gport, tagged);
    
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_drop_tagged(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    int32_t ret = 0;
    uint32 type_is_access = FALSE;
    uint32_t gport = 0;
#if 0
    ctc_vlantag_ctl_t   mode = CTC_VLANCTL_DROP_ALL_TAGGED;

    if(false == attr->value.booldata)
    {
        mode = CTC_VLANCTL_ALLOW_ALL_PACKETS;
    }
    ret = ctc_port_set_vlan_ctl(CTC_SAI_OBJECT_INDEX_GET(port_id), mode);
#endif
    
    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    type_is_access = attr->value.booldata;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    ret = ctc_sai_port_set_port_flag(port_id,SAI_PORT_FLAG_ALLOW_TAGGED, (!attr->value.booldata));

    p_sdb_port->type_is_access = type_is_access;
    if ((!type_is_access) && (p_sdb_port->vlanclass_active_type))
    {
        ret = ctc_sai_port_set_vlanclass_port_flag(gport, type_is_access, TRUE);
    }

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_drop_tagged(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t             ret = 0;
    ctc_vlantag_ctl_t   mode = CTC_VLANCTL_DROP_ALL_TAGGED;

    ret = ctc_port_get_vlan_ctl(CTC_SAI_OBJECT_INDEX_GET(port_id), &mode);

    if(CTC_VLANCTL_DROP_ALL_TAGGED == mode)
    {
        attr->value.booldata = true;
    }
    else
    {
        attr->value.booldata = false;
    }

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_loopback(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_loopback(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_fdb_learning(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = 0;
    bool enable = FALSE;
    int32_t sdk_ret = 0;

    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    if (SAI_PORT_LEARN_MODE_HW == attr->value.s32)
    {
        enable = TRUE;
    }
    sdk_ret = ctc_port_set_learning_en(gport, enable);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_get_fdb_learning(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_default_cos(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = 0;
    uint32_t ret = 0;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    ret = ctc_port_set_property(gport, CTC_PORT_PROP_DEFAULT_PCP,attr->value.u32);
    
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_qos_trust(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = 0;
    uint32_t ret = 0;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    if(SAI_PORT_QOS_TRUST_PORT == attr->value.u32)
    {
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_QOS_POLICY,CTC_QOS_TRUST_PORT);
    }
    else if(SAI_PORT_QOS_TRUST_DSCP == attr->value.u32)
    {
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_QOS_POLICY,CTC_QOS_TRUST_DSCP);
    }
    else //default trust cos
    {
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_QOS_POLICY,CTC_QOS_TRUST_STAG_COS);
    }
    
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_update_dscp(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = 0;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
#ifdef USW
    if(attr->value.booldata)
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_DSCP_SELECT_MODE, CTC_DSCP_SELECT_MAP);
    }
    else
    {
        /*modify by yangl for bug 52168 2019-09-19*/
        //ctc_port_set_property(gport, CTC_PORT_PROP_DSCP_SELECT_MODE, CTC_DSCP_SELECT_PACKET);
        ctc_port_set_property(gport, CTC_PORT_PROP_DSCP_SELECT_MODE, CTC_DSCP_SELECT_NONE);
    }
#else
    if(attr->value.booldata)
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_DSCP_EN, 1);
    }
    else
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_DSCP_EN, 0);
    }
#endif
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_update_dscp(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_update_cos(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = 0;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    if(attr->value.booldata)
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, 1);
    }
    else
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, 0);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_ip_source_guard(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    ctc_port_scl_property_t prop;
    int32_t                 sdk_ret = 0;    
    uint32                  gport = 0;

    ctc_sai_port_objectid_to_gport(port_id, &gport);
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));
    prop.direction      = CTC_INGRESS;
#ifdef TSINGMA
        prop.scl_id         = 0;
#else
        prop.scl_id         = 1;
#endif
    prop.action_type    = CTC_PORT_SCL_ACTION_TYPE_SCL;
    prop.class_id       = 63;

    if (attr->value.s32 == GLB_IP_BINDING_SCL_TYPE_IP)
    {
        prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA;
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP_SINGLE;
    }
    else if (attr->value.s32 == GLB_IP_BINDING_SCL_TYPE_MAC)
    {
        prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA;
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP_SINGLE;
    }
    else
    {
        prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    }

    sdk_ret = ctc_port_set_scl_property(gport, &prop);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_set_mtu(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32 gportid = CTC_SAI_OBJECT_INDEX_GET(port_id);
    bool enable = 0;    
    int32_t sdk_ret = 0;

#ifdef DUET2
    sdk_ret = ctc_port_get_mac_en(gportid, &enable);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    if (enable)
    {
        ctc_port_set_mac_en(gportid, FALSE);
        sdk_ret = ctc_port_set_max_frame(gportid, attr->value.u32);
        ctc_port_set_mac_en(gportid, TRUE);
    }
    else
    {
        sdk_ret = ctc_port_set_max_frame(gportid, attr->value.u32);
    }
#else
    (void)enable;
    sdk_ret = ctc_port_set_max_frame(gportid, attr->value.u32);
#endif

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_get_mtu(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t     sdk_ret = 0;

    sdk_ret = ctc_port_get_max_frame(CTC_SAI_OBJECT_INDEX_GET(port_id),
        &attr->value.u32);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_get_global_storm_control()
{
    int32_t    ret = CTC_E_NONE;  
    
    ctc_security_stmctl_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_security_stmctl_glb_cfg_t));
    
    ret = ctc_storm_ctl_get_global_cfg(&glb_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_flood_storm_control(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t    ret = CTC_E_NONE;
    uint32     policer_id = 0;  
    uint32     policer_cir = 0;
    uint32     meter_type = 0;
    uint32     policer_mode = 0;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    
    if(SAI_NULL_OBJECT_ID != policer_id)
    {
        policer_mode = ctc_sai_policer_get_mode_by_policer_id(policer_id);
        if (SAI_POLICER_MODE_STORM_CONTROL != policer_mode )
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
        meter_type = ctc_sai_policer_get_meter_type_by_policer_id(policer_id);
        if (SAI_METER_TYPE_PACKETS == meter_type)
        {
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS; 
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
        }
        else if (SAI_METER_TYPE_BYTES == meter_type)
        {
#if defined(GOLDENGATE) || defined(USW)
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_KBPS; 
#else
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            policer_cir = policer_cir * 1000; /*GB use Bps, and GG use KBps*/
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_BPS; 
#endif
        }
        else 
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }  
        stmctl_cfg.storm_en = true;
        stmctl_cfg.threshold = policer_cir;
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;       
    }
    else
    {
        stmctl_cfg.storm_en = false;      
        stmctl_cfg.threshold = 0;    
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS;
    }

    ret = ctc_storm_ctl_set_cfg(&stmctl_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_flood_storm_control(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t ret = CTC_E_NONE;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));
    
    ret = ctc_sai_port_get_global_storm_control();
    if (CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    } 
    stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
    stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    stmctl_cfg.type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;
    
    ret = ctc_storm_ctl_get_cfg(&stmctl_cfg);
    if (stmctl_cfg.storm_en)
    {
        attr->value.booldata = true;
    }
    else
    {
        attr->value.booldata = false;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_broadcast_storm_control(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t    ret = CTC_E_NONE;
    uint32     policer_id = 0;  
    uint32     policer_cir = 0;
    uint32     meter_type = 0;
    uint32     policer_mode = 0;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;    
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));
    
    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    
    if(SAI_NULL_OBJECT_ID != policer_id)
    {
        policer_mode = ctc_sai_policer_get_mode_by_policer_id(policer_id);
        if (SAI_POLICER_MODE_STORM_CONTROL != policer_mode )
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
        meter_type = ctc_sai_policer_get_meter_type_by_policer_id(policer_id);
        if (SAI_METER_TYPE_PACKETS == meter_type)
        {
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS; 
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
        }
        else if (SAI_METER_TYPE_BYTES == meter_type)
        {
#if defined(GOLDENGATE) || defined(USW)
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_KBPS; 
#else
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            policer_cir = policer_cir * 1000; /*GB use Bps, and GG use KBps*/
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_BPS; 
#endif
        }
        else 
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }  
        stmctl_cfg.storm_en = true;
        stmctl_cfg.threshold = policer_cir;
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_BCAST;        
    }
    else
    {
        stmctl_cfg.storm_en = false;      
        stmctl_cfg.threshold = 0;  
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_BCAST;
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS;
    }

    ret = ctc_storm_ctl_set_cfg(&stmctl_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_broadcast_storm_control(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t ret = CTC_E_NONE;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));
    
    stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
    stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    stmctl_cfg.type = CTC_SECURITY_STORM_CTL_BCAST;
    
    ret = ctc_storm_ctl_get_cfg(&stmctl_cfg);
    if (stmctl_cfg.storm_en)
    {
        attr->value.booldata = true;
    }
    else
    {
        attr->value.booldata = false;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_multicast_storm_control(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t    ret = CTC_E_NONE;
    uint32     policer_id = 0;  
    uint32     policer_cir = 0;
    uint32     meter_type = 0;
    uint32     policer_mode = 0;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    
    if(SAI_NULL_OBJECT_ID != policer_id)
    {
        policer_mode = ctc_sai_policer_get_mode_by_policer_id(policer_id);
        if (SAI_POLICER_MODE_STORM_CONTROL != policer_mode )
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
        meter_type = ctc_sai_policer_get_meter_type_by_policer_id(policer_id);
        if (SAI_METER_TYPE_PACKETS == meter_type)
        {
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS; 
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
        }
        else if (SAI_METER_TYPE_BYTES == meter_type)
        {
#if defined(GOLDENGATE) || defined(USW)
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_KBPS; 
#else
            policer_cir = ctc_sai_policer_get_cir_by_policer_id(policer_id);
            policer_cir = policer_cir * 1000; /*GB use Bps, and GG use KBps*/
            stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_BPS; 
#endif
        }
        else 
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }  
        stmctl_cfg.storm_en = true;
        stmctl_cfg.threshold = policer_cir;
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
    
    }
    else
    {
        stmctl_cfg.storm_en = false;      
        stmctl_cfg.threshold = 0;   
        stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        stmctl_cfg.type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS;
    }

    ret = ctc_storm_ctl_set_cfg(&stmctl_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_multicast_storm_control(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t ret = CTC_E_NONE;
    
    ctc_security_stmctl_cfg_t  stmctl_cfg;
    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));

    ret = ctc_sai_port_get_global_storm_control();
    if (CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }   
    stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
    stmctl_cfg.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    stmctl_cfg.type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
    
    ret = ctc_storm_ctl_get_cfg(&stmctl_cfg);    
    if (stmctl_cfg.storm_en)
    {
        attr->value.booldata = true;
    }
    else
    {
        attr->value.booldata = false;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_global_flow_control(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t             ret = CTC_E_NONE;
    stbl_port_t *p_sdb_port = NULL;
    ctc_port_fc_prop_t  fc_prop;
    sal_memset(&fc_prop,0,sizeof(ctc_port_fc_prop_t));
    
    if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    fc_prop.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    fc_prop.enable= TRUE;
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_sdb_port->flowctl_en = TRUE;

    switch(attr->value.s32)
    {
    case SAI_PORT_FLOW_CONTROL_DISABLE:
        fc_prop.dir = CTC_BOTH_DIRECTION;
        fc_prop.enable= FALSE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        p_sdb_port->flowctl_en = FALSE;
        break;
    case SAI_PORT_FLOW_CONTROL_TX_ONLY:
        fc_prop.dir = CTC_EGRESS;
        fc_prop.enable = TRUE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        fc_prop.dir = CTC_INGRESS;
        fc_prop.enable = FALSE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        break;
    case SAI_PORT_FLOW_CONTROL_RX_ONLY:
        fc_prop.dir = CTC_EGRESS;
        fc_prop.enable = FALSE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        fc_prop.dir = CTC_INGRESS;
        fc_prop.enable = TRUE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        break;
    case SAI_PORT_FLOW_CONTROL_BOTH_ENABLE:
        fc_prop.dir = CTC_BOTH_DIRECTION;
        fc_prop.enable= TRUE;
        ret = ctc_port_set_flow_ctl_en(&fc_prop);
        break;
    default:
        break;
    }
    p_sdb_port->flowctl_dir = attr->value.s32;

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_get_global_flow_control(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    int32_t  ret = CTC_E_NONE;

    if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    attr->value.s32 = p_sdb_port->flowctl_dir;
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_max_learned_address(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
#if defined GOLDENGATE || defined (USW)
    int32_t                     ret = CTC_E_NONE;
    ctc_security_learn_limit_t  old_limit;
    ctc_security_learn_limit_t  new_limit;
    stbl_port_t *p_sdb_port = NULL;
    old_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
    old_limit.gport      = CTC_SAI_OBJECT_INDEX_GET(port_id);
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    ret = ctc_mac_security_get_learn_limit(&old_limit);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    new_limit = old_limit;
    new_limit.limit_num = attr->value.u32;
    p_sdb_port->mac_limit_num = attr->value.u32;
    ret = ctc_mac_security_set_learn_limit(&new_limit);

    return ctc_sai_get_error_from_sdk_error(ret);
#else
    //mac limit on GB should be implementioned by hsrv fdb
    return ctc_sai_get_error_from_sdk_error(CTC_E_NONE);
#endif
}

sai_status_t
ctc_sai_port_get_max_learned_address(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
#if defined GOLDENGATE || defined (USW)
    int32_t                     ret = CTC_E_NONE;
    ctc_security_learn_limit_t  old_limit;

    old_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
    old_limit.gport      = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ret = ctc_mac_security_get_learn_limit(&old_limit);

    if(CTC_E_NONE == ret)
    {
        attr->value.u32 = old_limit.limit_num;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
#else
    return ctc_sai_get_error_from_sdk_error(CTC_E_NONE);
#endif
}


sai_status_t
ctc_sai_port_set_fdb_learning_limit_violation(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
#if defined GOLDENGATE || defined (USW)
    int32_t                     ret = CTC_E_NONE;
    ctc_security_learn_limit_t  old_limit;
    ctc_security_learn_limit_t  new_limit;

    old_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
    old_limit.gport      = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ret = ctc_mac_security_get_learn_limit(&old_limit);

    if(CTC_E_NONE != ret)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    new_limit = old_limit;

    switch(attr->value.s32)
    {
    case SAI_PACKET_ACTION_DROP:
        new_limit.limit_action = CTC_MACLIMIT_ACTION_DISCARD;
        break;
    case SAI_PACKET_ACTION_FORWARD:
        new_limit.limit_action = CTC_MACLIMIT_ACTION_FWD;
        break;
    case SAI_PACKET_ACTION_TRAP:
        new_limit.limit_action = CTC_MACLIMIT_ACTION_TOCPU;
        break;
    case SAI_PACKET_ACTION_LOG:
        break;
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }

    ret = ctc_mac_security_set_learn_limit(&new_limit);

    return ctc_sai_get_error_from_sdk_error(ret);
#else
    int32_t                     ret = CTC_E_NONE;
    ctc_maclimit_action_t       action = CTC_MACLIMIT_ACTION_NONE;
    
    uint32 gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    switch(attr->value.u32)
    {
    case SAI_PACKET_ACTION_DROP:
        action = CTC_MACLIMIT_ACTION_DISCARD;
        break;
    case SAI_PACKET_ACTION_FORWARD:
        action = CTC_MACLIMIT_ACTION_NONE;
        break;
    case SAI_PACKET_ACTION_TRAP:
        action = CTC_MACLIMIT_ACTION_TOCPU;
        break;
    case SAI_PACKET_ACTION_LOG:
        action = CTC_MACLIMIT_ACTION_TOCPU_NODISC;
        break;
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }

    ret = ctc_mac_security_set_port_mac_limit(gport, action);
    return ctc_sai_get_error_from_sdk_error(ret);
#endif
}

sai_status_t
ctc_sai_port_get_fdb_learning_limit_violation(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    int32_t                     ret = CTC_E_NONE;
    ctc_security_learn_limit_t  old_limit;

    old_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
    old_limit.gport      = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ret = ctc_mac_security_get_learn_limit(&old_limit);

    if(CTC_E_NONE == ret)
    {
        switch(old_limit.limit_action)
        {
        case CTC_MACLIMIT_ACTION_DISCARD:
            attr->value.s32 = SAI_PACKET_ACTION_DROP;
            break;
        case CTC_MACLIMIT_ACTION_FWD:
            attr->value.s32 = SAI_PACKET_ACTION_FORWARD;
            break;
        case CTC_MACLIMIT_ACTION_TOCPU:
            attr->value.s32 = SAI_PACKET_ACTION_TRAP;
            break;
        default:
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_port_set_ingress_mirror_session(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32   session_id = 0;
    uint8    sdk_session_id = 0;
    uint32   gport = 0;
    int32_t  sdk_ret = 0;
    ctc_mirror_dest_t mirror_dest;
    bool enable;
 
    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        gport = CTC_LPORT_CPU;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(port_id, &gport);
    }

    sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
    
    if (0 == attr->value.objlist.count)
    {
        ctc_mirror_get_port_info(gport, CTC_INGRESS, &enable, &sdk_session_id);
        sdk_ret = ctc_mirror_set_port_en(gport, CTC_INGRESS, FALSE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else if (1 == attr->value.objlist.count)
    {
        session_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
        sdk_session_id = session_id;
        sdk_ret = ctc_mirror_set_port_en(gport, CTC_INGRESS, TRUE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }  
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_get_ingress_mirror_session(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    uint8    sdk_session_id = 0;
    uint32   gport = 0;
    int32_t  sdk_ret = 0;
    bool     enable = FALSE;

    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        gport = CTC_LPORT_CPU;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(port_id, &gport);
    }

    if (0 == attr->value.objlist.count)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else
    {
        sdk_ret = ctc_mirror_get_port_info(gport, CTC_INGRESS, &enable, &sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
        if (enable)
        {
            attr->value.objlist.count = 1;
            attr->value.objlist.list[0] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, sdk_session_id);
        }
        else
        {
            attr->value.objlist.count = 0;
            attr->value.objlist.list = NULL;
        }
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);     
}

sai_status_t
ctc_sai_port_set_egress_mirror_session(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32  session_id = 0;
    uint8   sdk_session_id = 0;
    uint32  gport = 0;
    int32_t sdk_ret = 0;
    ctc_mirror_dest_t mirror_dest;
    bool enable;

    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        gport = CTC_LPORT_CPU;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(port_id, &gport);
    }
    sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
    
    if (0 == attr->value.objlist.count)
    {
        ctc_mirror_get_port_info(gport, CTC_EGRESS, &enable, &sdk_session_id);
        sdk_ret = ctc_mirror_set_port_en(gport, CTC_EGRESS, FALSE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
       
    }
    else if (1 == attr->value.objlist.count)
    {
        session_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.objlist.list[0]);
        sdk_session_id = session_id;
        sdk_ret = ctc_mirror_set_port_en(gport, CTC_EGRESS, TRUE, sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);    
}

sai_status_t
ctc_sai_port_get_egress_mirror_session(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    uint8   sdk_session_id = 0 ;
    uint32  gport = 0;
    int32_t sdk_ret = 0;
    bool    enable = FALSE;

    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        gport = CTC_LPORT_CPU;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(port_id, &gport);
    }
    
    if (0 == attr->value.objlist.count ) 
    {   
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else
    {   
        sdk_ret = ctc_mirror_get_port_info(gport, CTC_EGRESS, &enable, &sdk_session_id);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
        if (enable)
        {
            attr->value.objlist.count = 1;
            attr->value.objlist.list[0] =  CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_MIRROR, sdk_session_id);
        }
        else
        {
            attr->value.objlist.count = 0;
            attr->value.objlist.list = NULL;
        }
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);   
}

sai_status_t
ctc_sai_port_set_priority_flow_control(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
     uint32  gport = 0;
     int32_t sdk_ret = 0;
     uint8   cos_temp = 0;
     uint32  i = 0;
     
     ctc_port_fc_prop_t pfc_cfg;
     ctc_qos_resrc_t resrc;
     
     sal_memset(&pfc_cfg, 0, sizeof(ctc_port_fc_prop_t));
     sal_memset(&resrc, 0, sizeof(ctc_qos_resrc_t));
     
     ctc_sai_port_objectid_to_gport(port_id, &gport);
     pfc_cfg.gport = gport;
     pfc_cfg.dir = CTC_BOTH_DIRECTION;
     pfc_cfg.is_pfc = TRUE;
     cos_temp = attr->value.u8;

     for(i = 0; i < 8; i++)
     {
        if (CTC_IS_BIT_SET(cos_temp,i))
        {
            pfc_cfg.priority_class = i;
            pfc_cfg.enable = TRUE;
            sdk_ret = ctc_port_set_flow_ctl_en(&pfc_cfg);
            if(sdk_ret < 0)
            {
                return ctc_sai_get_error_from_sdk_error(sdk_ret);   
            }
            
            resrc.cfg_type = CTC_QOS_RESRC_CFG_FLOW_CTL;
            resrc.u.flow_ctl.gport = gport;
            resrc.u.flow_ctl.is_pfc = TRUE;
            resrc.u.flow_ctl.priority_class = i;
            resrc.u.flow_ctl.xon_thrd  = SAI_PORT_PFC_XON_THRD;
            resrc.u.flow_ctl.xoff_thrd  = SAI_PORT_PFC_XOFF_THRD;
            resrc.u.flow_ctl.drop_thrd  = SAI_PORT_PFC_DROP_THRD;
            
            sdk_ret = ctc_qos_set_resrc(&resrc);
            if (sdk_ret < 0)
            {
                return ctc_sai_get_error_from_sdk_error(sdk_ret);   
            }
        }
        else
        {
            pfc_cfg.priority_class = i;
            pfc_cfg.enable = FALSE;
            sdk_ret = ctc_port_set_flow_ctl_en(&pfc_cfg);
            if (sdk_ret < 0)
            {
                return ctc_sai_get_error_from_sdk_error(sdk_ret);   
            }
        }
     }
     return ctc_sai_get_error_from_sdk_error(sdk_ret);      
}

int32
ctc_sai_mapping_isolation_param(uint32 isolate_id, sai_port_isolation_direction_t dir, 
    sai_isolaton_group_drop_pkt_type_t type, ctc_port_restriction_t *pst_ctc)
{
    sal_memset(pst_ctc, 0, sizeof(ctc_port_restriction_t));
    
    pst_ctc->mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    pst_ctc->isolated_id = isolate_id;
    #ifdef USW
//  pst_ctc->isolate_group = isolate_id;
    #else
    pst_ctc->isolate_group = isolate_id;
    #endif
    switch (dir)
    {
    case SAI_PORT_ISOLATION_DIRECTION_INGRESS:
        pst_ctc->dir = CTC_INGRESS;
        break;
    case SAI_PORT_ISOLATION_DIRECTION_EGRESS:
        pst_ctc->dir = CTC_EGRESS;
        break;
    case SAI_PORT_ISOLATION_DIRECTION_BOTH:
        pst_ctc->dir = CTC_BOTH_DIRECTION;
        break;
    default:
        pst_ctc->dir = CTC_BOTH_DIRECTION;
        break;        
    }

    switch (type)
    {
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_UC:
        pst_ctc->type = CTC_PORT_ISOLATION_UNKNOW_UCAST;
        break;
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_UC:
        pst_ctc->type = CTC_PORT_ISOLATION_KNOW_UCAST;
        break;
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_MC:
        pst_ctc->type = CTC_PORT_ISOLATION_UNKNOW_MCAST;
        break;
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_MC:
        pst_ctc->type = CTC_PORT_ISOLATION_KNOW_MCAST;
        break;
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_BC:
        pst_ctc->type = CTC_PORT_ISOLATION_BCAST;
        break;
    case SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL:
        pst_ctc->type = CTC_PORT_ISOLATION_ALL;
        break;
    default:
        pst_ctc->type = CTC_PORT_ISOLATION_ALL;
        break;        
    }
    return 0;
}

sai_status_t
_ctc_sai_port_set_isolation_group_pvlan_enable(uint32_t gport, uint32_t isolate_id, stbl_port_t *p_sdb_port)
{
    ctc_port_restriction_t ctc_param;
    int32_t sdk_ret = 0;

    if (SAI_PORT_ISOLATION_PVLAN_TYPE_ISOLATE == p_sdb_port->isolation_type)
    {
        #ifdef USW
//      ctc_param.isolate_group = 0;
        #else
        ctc_param.isolate_group = 0;
        #endif
        ctc_param.isolated_id = 0;
        ctc_param.mode = CTC_PORT_RESTRICTION_PVLAN;
        ctc_param.type = CTC_PORT_PVLAN_ISOLATED;
        ctc_param.dir = 0;
        sdk_ret = ctc_port_set_restriction(gport, &ctc_param);
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    else if (SAI_PORT_ISOLATION_PVLAN_TYPE_PROMISCUOUS == p_sdb_port->isolation_type)
    {
        #ifdef USW
//      ctc_param.isolate_group = 0;
        #else
        ctc_param.isolate_group = 0;
        #endif
        ctc_param.isolated_id = 0;
        ctc_param.mode = CTC_PORT_RESTRICTION_PVLAN;
        ctc_param.type = CTC_PORT_PVLAN_PROMISCUOUS;
        ctc_param.dir = 0;
        sdk_ret = ctc_port_set_restriction(gport, &ctc_param);
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    else if (SAI_PORT_ISOLATION_PVLAN_TYPE_COMMUNITY == p_sdb_port->isolation_type)
    {
        #ifdef USW
//      ctc_param.isolate_group = 0;
        #else
        ctc_param.isolate_group = 0;
        #endif
        ctc_param.isolated_id = isolate_id;
        ctc_param.mode = CTC_PORT_RESTRICTION_PVLAN;
        ctc_param.type = CTC_PORT_PVLAN_COMMUNITY;
        ctc_param.dir = 0;
        sdk_ret = ctc_port_set_restriction(gport, &ctc_param);
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
}

sai_status_t
_ctc_sai_port_set_isolation_group_pvlan_disable(uint32_t gport, stbl_port_t *p_sdb_port)
{
    ctc_port_restriction_t ctc_param;
    int32_t sdk_ret = 0;

    if (SAI_PORT_ISOLATION_TYPE_DEFAULT == p_sdb_port->isolation_type)
    {
        #ifdef USW
//      ctc_param.isolate_group = 0;
        #else
        ctc_param.isolate_group = 0;
        #endif
        ctc_param.isolated_id = 0;
        ctc_param.mode = CTC_PORT_RESTRICTION_DISABLE;
        ctc_param.type = CTC_PORT_PVLAN_NONE;
        ctc_param.dir = 0;
        sdk_ret = ctc_port_set_restriction(gport, &ctc_param);
        return ctc_sai_get_error_from_sdk_error(sdk_ret);    
    }
    else
    {
        return SAI_STATUS_SUCCESS;
    }
}

sai_status_t
ctc_sai_port_set_isolation_group(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    ctc_sai_isolation_group_t *pst_group = NULL;
    stbl_port_t *p_sdb_port = NULL;
    ctc_port_restriction_t ctc_param;
    sai_object_id_t isolate_oid = 0;
    sai_isolaton_group_drop_pkt_type_t type = 0;
    sai_isolaton_group_mode_t mode = 0;
#ifdef USW
    ctc_port_isolation_t port_isolation;
#endif
    uint32_t isolate_id = 0;
    uint32_t gport = 0;
    int32_t sdk_ret = 0;
    int32_t ret = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    isolate_oid = attr->value.oid;
    isolate_id = CTC_SAI_OBJECT_INDEX_GET(isolate_oid);
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 1. PVLAN process */
    if (p_sdb_port->isolation_type)
    {
        /* must be return and not do step 2 */
        return _ctc_sai_port_set_isolation_group_pvlan_enable(gport, isolate_id, p_sdb_port);
    }

    /* 2. Port-Isolation and MLAG process */    
    if (0 == isolate_id)
    {
        if (p_sdb_port->isolation_oid)
        {

        }
        type = SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL;
    }

    if (isolate_id)
    {
        pst_group = ctc_sai_get_isolation_group(isolate_oid);
        if (NULL == pst_group)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        type = pst_group->type;
        mode = pst_group->mode;
    }
    else if (p_sdb_port->isolation_oid)
    {
        pst_group = ctc_sai_get_isolation_group(p_sdb_port->isolation_oid);
        if (NULL == pst_group)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        type = pst_group->type;
        mode = pst_group->mode;
    }
    else
    {
        type = SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL;
        mode = SAI_ISOLATION_GROUP_MODE_ISOLATION;
    }
    
    p_sdb_port->isolation_oid = isolate_oid;

    ctc_sai_mapping_isolation_param(isolate_id, p_sdb_port->isolation_dir, type, &ctc_param);
    sdk_ret = ctc_port_set_restriction(gport, &ctc_param);

#ifdef USW
    if (SAI_ISOLATION_GROUP_MODE_ISOLATION == mode)
    {
        if (isolate_id)
        {
            sal_memset(&port_isolation, 0, sizeof(port_isolation));
            port_isolation.gport = isolate_id;
            port_isolation.use_isolation_id = TRUE;
            port_isolation.pbm[0] = 1<<(isolate_id);
            port_isolation.isolation_pkt_type = CTC_PORT_ISOLATION_ALL;
            ctc_port_set_isolation(0, &port_isolation);
        }
    }
#endif
    
    if (SAI_ISOLATION_GROUP_MODE_MLAG == mode)
    {
        /* is MLAG isolation, should set group 31 to block mcast */
        ctc_sai_mapping_isolation_param(CTC_SAI_ISOLATION_MLAG, p_sdb_port->isolation_dir, type, &ctc_param);
        ctc_param.isolated_id = isolate_id ? TRUE : FALSE;  /* TRUE: isolated, FALSE: not isolated */
        sdk_ret = ctc_port_set_restriction(gport, &ctc_param);
    }

    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    return ret;   
}

sai_status_t
ctc_sai_port_set_isolation_direction(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    uint32_t isolate_dir = 0;
    int32_t sdk_ret = 0;
    int32_t ret = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    isolate_dir = attr->value.s32;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_sdb_port->isolation_dir = isolate_dir;

    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    return ret;
}

sai_status_t
ctc_sai_port_set_isolation_type(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    uint32_t isolation_type = 0;
    uint32_t gport = 0;
    int32_t ret = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    isolation_type = attr->value.s32;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_sdb_port->isolation_type = isolation_type;
    ret = _ctc_sai_port_set_isolation_group_pvlan_disable(gport, p_sdb_port);
    return ret;
}

sai_status_t
ctc_sai_port_get_isolation_group(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    attr->value.oid = p_sdb_port->isolation_oid;

    return SAI_STATUS_SUCCESS; 
}

sai_status_t
ctc_sai_port_get_isolation_direction(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    attr->value.s32 = p_sdb_port->isolation_dir;

    return SAI_STATUS_SUCCESS; 
}

sai_status_t
ctc_sai_port_set_vlanclass_port_flag(uint16 gport, uint8 type_is_access, bool enable)
{
    int32_t ret = 0;
    if (enable)
    {
        /* stag should be removed for trunk port when output, this set is not necessary for access port */
        if (!type_is_access)
        {
            ret = ctc_port_set_dot1q_type(gport, CTC_DOT1Q_TYPE_CVLAN);
            if (ret < 0)
            {
                return ret;
            }
        } 
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(gport, SAI_PORT_FLAG_VLAN_CLASS_ENABLE, TRUE), ret, out);
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, FALSE);  
    }
    else
    {  
        /* set dot1q_type to initial value for trunk port*/
        if (!type_is_access)
        {
           ret = ctc_port_set_dot1q_type(gport, CTC_DOT1Q_TYPE_BOTH);
           if (ret < 0)
           {
               return ret;
           }
        }
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_port_flag(gport, SAI_PORT_FLAG_VLAN_CLASS_ENABLE, FALSE), ret, out);
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, TRUE);
    }
out:
    return ret;
}

sai_status_t
ctc_sai_port_set_vlanclass_group_id(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    ctc_port_scl_property_t scl_prop;
    stbl_port_t *p_sdb_port = NULL;
    uint32_t vclass_group_id = 0;
    uint32_t active_type = 0;
    uint32_t gport = 0;
    int32_t ret = 0;
    
    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    vclass_group_id = attr->value.s32;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    p_sdb_port->vlanclass_group_id = vclass_group_id;
    
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    
    active_type = p_sdb_port->vlanclass_active_type;
    if (active_type)
    {
        if (SAI_PORT_VLANCLASS_ACTIVE_TYPE_MAC == active_type)
        {
            scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_MAC_SA;
        }
        else if (SAI_PORT_VLANCLASS_ACTIVE_TYPE_IP == active_type)
        {
            scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA;
        }
        else if (SAI_PORT_VLANCLASS_ACTIVE_TYPE_PROTOCOL == active_type)
        {
            ctc_port_set_protocol_vlan_en(gport, TRUE);
            scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT;
        }
        else
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.class_id_en = 0;
        scl_prop.class_id = vclass_group_id;

        ctc_port_set_scl_property(gport, &scl_prop);
        ctc_port_set_vlan_domain(gport, CTC_PORT_VLAN_DOMAIN_CVLAN);
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_vlanclass_port_flag(gport, p_sdb_port->type_is_access, TRUE), ret, out);
    }
    else
    {
        ctc_port_set_protocol_vlan_en(gport, FALSE);
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.class_id_en = 0;
        
        ctc_port_set_scl_property(gport, &scl_prop);
        ctc_port_set_vlan_domain(gport, CTC_PORT_VLAN_DOMAIN_SVLAN);
        CTC_SAI_ERROR_GOTO(ctc_sai_port_set_vlanclass_port_flag(gport, p_sdb_port->type_is_access, FALSE), ret, out);
    }
out:
    return ctc_sai_get_error_from_sdk_error(ret);

}

sai_status_t
ctc_sai_port_set_vlanclass_active_type(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;
    uint32_t active_type = 0;
    int32_t sdk_ret = 0;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    active_type = attr->value.s32;
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    p_sdb_port->vlanclass_active_type = active_type;
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_port_set_vlanclass_default_action(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    ctc_scl_default_action_t dft_action;
    stbl_port_t *p_sdb_port = NULL;
    uint32_t default_action = 0;
    uint32_t gport = 0;
    int32_t sdk_ret = 0;
    int32_t ret = 0;
    
    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    default_action = attr->value.s32;
    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    p_sdb_port->vlanclass_default_action = default_action;
    
    sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
    if (default_action)
    {
        CTC_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
    }
    else
    {
        CTC_UNSET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
    }
    dft_action.gport = gport;
    dft_action.action.type = CTC_SCL_ACTION_INGRESS;
    sdk_ret = ctc_scl_set_default_action(&dft_action);
 
    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    return ret;
}

sai_status_t
ctc_sai_port_get_vlanclass_group_id(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    attr->value.s32 = p_sdb_port->vlanclass_group_id;

    return SAI_STATUS_SUCCESS; 
}

sai_status_t
ctc_sai_port_get_vlanclass_active_type(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    attr->value.s32 = p_sdb_port->vlanclass_active_type;

    return SAI_STATUS_SUCCESS; 
}

sai_status_t
ctc_sai_port_get_vlanclass_default_action(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    stbl_port_t *p_sdb_port = NULL;

    if (SAI_OBJECT_TYPE_PORT != CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    attr->value.s32 = p_sdb_port->vlanclass_default_action;

    return SAI_STATUS_SUCCESS; 
}

sai_status_t
ctc_sai_port_set_ingress_samplepacket_enable(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_ingress_samplepacket_enable(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_egress_samplepacket_enable(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_egress_samplepacket_enable(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_get_queue_list(sai_object_id_t port_id,
                                sai_attribute_t *attr)
{
    uint32_t queue_offset = 0;

    attr->value.objlist.count = 13;
    for(queue_offset = 0; queue_offset < 13; queue_offset ++)
    {
        attr->value.objlist.list[queue_offset] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, ((port_id << 16) + queue_offset));
    }
    
    return SAI_STATUS_SUCCESS;
}

/*add by yejl for bug37838*/
int32
ctc_sai_port_set_mac(uint32 gport, mac_addr_t mac_addr)
{
    int32 sdk_ret = 0;
    ctc_port_mac_postfix_t mac_postfix;
    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    CTC_SET_FLAG(mac_postfix.prefix_type, CTC_PORT_MAC_PREFIX_48BIT);
    mac_postfix.port_mac[0] = mac_addr[0];
    mac_postfix.port_mac[1] = mac_addr[1];
    mac_postfix.port_mac[2] = mac_addr[2];
    mac_postfix.port_mac[3] = mac_addr[3];
    mac_postfix.port_mac[4] = mac_addr[4];
    mac_postfix.port_mac[5] = mac_addr[5];
    
    sdk_ret = ctc_port_set_port_mac_postfix(gport, &mac_postfix);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

#ifdef GREATBELT
int32
ctc_sai_port_set_oam_mac(uint32 gport, const sai_attribute_t *attr)
{
    int32 sdk_ret = 0;
    ctc_port_mac_postfix_t mac_postfix;
    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    CTC_SET_FLAG(mac_postfix.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
    mac_postfix.port_mac[0] = attr->value.mac[0];
    mac_postfix.port_mac[1] = attr->value.mac[1];
    mac_postfix.port_mac[2] = attr->value.mac[2];
    mac_postfix.port_mac[3] = attr->value.mac[3];
    mac_postfix.port_mac[4] = attr->value.mac[4];
    mac_postfix.port_mac[5] = attr->value.mac[5];
    mac_postfix.low_8bits_mac = attr->value.mac[5];
    
    sdk_ret = ctc_port_set_port_mac_postfix(gport, &mac_postfix);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}
#endif

int32
ctc_sai_port_set_dhcp_relay_exception_type(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    
    if(attr->value.s32)
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_EXCP_DISCARD_AND_TO_CPU);
    }
    else
    {
        ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_EXCP_NORMAL_FWD);
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_port_set_egress_port_acl(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t    *p_sdb_port = NULL;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    if (attr->value.u32)
    {
        p_sdb_port->egress_acl_en = TRUE;  
        ctc_port_set_direction_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, TRUE);
    }
    else
    {
        p_sdb_port->egress_acl_en = FALSE; 
        ctc_port_set_direction_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, FALSE);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_sflow_en(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t gport = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ctc_sai_port_set_port_sflow(gport, attr->value.sflow.dir, attr->value.sflow.threshold,
        attr->value.sflow.enable);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_metadata(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t    *p_sdb_port = NULL;
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    if(attr->value.u32)
    {
        p_sdb_port->metadata_en = TRUE;   
        ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_PROP_METADATA_OVERWRITE_UDF, TRUE);
    }
    else
    {
        p_sdb_port->metadata_en = FALSE;   
        ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_PROP_METADATA_OVERWRITE_UDF, FALSE);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_set_svlan_tpid(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t    *p_sdb_port = NULL;
    uint32          value = 0;

    /*
     SDK default svlan-tpid mapping
     0: 0x8100
     1: 0x9100
     2: 0x88A8
     3: 0x88A8

     SDK default cvlan-tpid mapping
     0x8100--global
     */
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    switch(attr->value.u32)
    {
    case 0x8100:
        value = 0;
        break;
    case 0x88A8:
        value = 2;
        break;
    case 0x9100:
        value = 1;
        break;
    default :
        value = 0;
        break;
    }

    ctc_port_set_direction_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_DIR_PROP_STAG_TPID_INDEX,CTC_INGRESS, value);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_set_dot1q_ethertype(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32          value = 0;
    sai_attribute_t tpid_attr;

    sal_memset(&tpid_attr, 0, sizeof(tpid_attr));
    /* add for configure svlan tpid on parser ethernet control */
    tpid_attr.value.u32 += (1 << 16);
    ctc_sai_switch_get_svlan_tpid(&tpid_attr);
    if (attr->value.u32 == (tpid_attr.value.u32 & 0x0000ffff))
    {
        value = 1;
    }

    ctc_port_set_stag_tpid_index(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_BOTH_DIRECTION, value);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_reflective(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    stbl_port_t    *p_sdb_port = NULL;
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    if(attr->value.u32)
    {
        p_sdb_port->reflective_bridge_en = 1; 
        ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, 1);
    }
    else
    {
        p_sdb_port->reflective_bridge_en = 0; 
        ctc_port_set_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, 0);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_hash_value_profile_id(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t sdk_ret = 0;
#ifdef TSINGMA
    uint32_t gport = 0;
    uint32 profile_id = 0;

    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    profile_id = attr->value.s32;
    sdk_ret = ctc_port_set_property(gport, CTC_PORT_PROP_LB_HASH_ECMP_PROFILE, profile_id);
    if (sdk_ret)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    sdk_ret = ctc_port_set_property(gport, CTC_PORT_PROP_LB_HASH_LAG_PROFILE, profile_id);
#endif
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_port_set_hash_value_agg_hash(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    int32_t sdk_ret = 0;
#ifdef TSINGMA    
    uint32_t tid = 0;
    uint32 cfg_value = 0;
    
    tid = CTC_SAI_OBJECT_INDEX_GET(port_id);
    cfg_value = attr->value.s32;
    sdk_ret = ctc_linkagg_set_property(tid, CTC_LINKAGG_PROP_LB_HASH_OFFSET, cfg_value);
#endif
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

int32
ctc_sai_port_set_tap_used(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t    gport       = CTC_SAI_OBJECT_INDEX_GET(port_id);
    uint32_t    tap_used    = 0;
    uint32      action_idx  = 0;
    ctc_port_scl_property_t     scl_prop;
    ctc_scl_default_action_t    def_action;
    ctc_qos_queue_cfg_t         que_cfg;
    ctc_port_fc_prop_t          fc_prop;

    tap_used = attr->value.u32;
    
    /* 1: set default config in tap port  */
    if(tap_used)
    {
        /* 1, should not check source port equal to dest port */
        ctc_port_set_port_check_en(gport, FALSE);

        /* 2, l2pdu should forwarding normally */
        for (action_idx = 0; action_idx <= 31; action_idx++)
        {
            ctc_l2pdu_set_port_action(  gport, 
                                        action_idx, 
                                        CTC_PDU_L2PDU_ACTION_TYPE_FWD);
        }

        ctc_l2pdu_set_port_action(  gport, 
                                    CTC_L2PDU_ACTION_INDEX_BPDU, 
                                    CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU);
//#ifndef DUET2
        ctc_port_set_direction_property(CTC_SAI_OBJECT_INDEX_GET(port_id), 
            CTC_PORT_DIR_PROP_STAG_TPID_INDEX_EN,CTC_EGRESS, FALSE);

        /* 3, CRC disable */
        ctc_port_set_property(gport, CTC_PORT_PROP_CRC_CHECK_EN, FALSE);

        /* 4, dot1q type */
        ctc_port_set_property(gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH);

        /* 5, dot1q type */
        ctc_port_set_untag_dft_vid(gport, FALSE, FALSE);

        /* 6, don't add default vlan */
        ctc_port_set_property(gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, TRUE);

        /* 7, disable filter */
        ctc_port_set_vlan_filter_en(gport, CTC_INGRESS, FALSE);
        ctc_port_set_vlan_filter_en(gport, CTC_EGRESS, FALSE);

        /* 8, allow all vlan */
        ctc_port_set_vlan_ctl(gport, CTC_VLANCTL_ALLOW_ALL_PACKETS);

		/* 5, set scl key type */
#if 0/* acl init */
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id         = 0;
        scl_prop.direction      = CTC_INGRESS;
        scl_prop.tcam_type      = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
        scl_prop.action_type    = CTC_PORT_SCL_ACTION_TYPE_FLOW;
        ctc_port_set_scl_property(gport, &scl_prop);
#endif
        /* 5, set scl key type */
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id         = 1;
        scl_prop.direction      = CTC_INGRESS;
        scl_prop.hash_type      = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
        scl_prop.action_type    = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(gport, &scl_prop);

        /* 6, set scl action */
        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport                        = gport;
#if 0
        def_action.action.u.igs_action.flag     = CTC_SCL_IGS_ACTION_FLAG_REDIRECT;
        def_action.action.u.igs_action.nh_id    = CTC_NH_RESERVED_NHID_FOR_DROP;
#else
#ifndef USW
    #ifdef GREATBELT
        def_action.action.u.igs_action.flag     = CTC_SCL_IGS_ACTION_FLAG_REDIRECT;
        def_action.action.u.igs_action.nh_id    = CTC_NH_RESERVED_NHID_FOR_DROP;
    #endif
    #ifdef GOLDENGATE
	    def_action.action.u.igs_action.flag     = CTC_SCL_IGS_ACTION_FLAG_DENY_BRIDGE;
    #endif
        
#endif
#endif		
        ctc_scl_set_default_action(&def_action);

        sal_memset(&que_cfg, 0, sizeof(que_cfg));
        que_cfg.type = CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU;    
        que_cfg.value.value_32 = gport << 16 | GLB_DFT_MAX_FRAME_SIZE_1;
        ctc_qos_set_queue(&que_cfg);

        /* 7, by pass pause packet */
        sal_memset(&fc_prop, 0, sizeof(fc_prop));
        fc_prop.gport           = gport;
        fc_prop.priority_class  = 0;
        fc_prop.pfc_class       = 0;
        fc_prop.is_pfc          = FALSE;
        fc_prop.enable          = FALSE;
        fc_prop.dir             = CTC_INGRESS;
        ctc_port_set_flow_ctl_en(&fc_prop);

        /* 14, by pass arp dhcp */
        ctc_port_set_property(  gport, 
                                CTC_PORT_PROP_L3PDU_ARP_ACTION, 
                                CTC_PORT_ARP_ACTION_TYPE_FW);
        ctc_port_set_property(  gport, 
                                CTC_PORT_PROP_L3PDU_DHCP_ACTION, 
                                CTC_PORT_DHCP_ACTION_TYPE_FW);

        ctc_port_set_property(  gport, 
                                CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, 
                                TRUE);

#ifdef GOLDENGATE
        /* 15, set inner hash-calc */
        ctc_port_set_property(gport, CTC_PORT_PROP_EXTERN_ENABLE, FALSE);
        ctc_port_set_property(gport, CTC_PORT_PROP_SNOOPING_PARSER, TRUE);
        ctc_port_set_property(gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, TRUE);

        /* enable nvgre */
        ctc_port_set_property(  gport, 
                                CTC_PORT_PROP_NVGRE_ENABLE, 
                                TRUE);
#endif

        ctc_port_set_property(gport, CTC_PORT_PROP_MAX_FRAME_SIZE, GLB_DFT_MAX_FRAME_SIZE_1);

#ifdef USW
        /* BUG 50872 enable nvgre fix agg hash use nvgre vsid */
        ctc_port_set_property(gport, CTC_PORT_PROP_NVGRE_ENABLE, TRUE);
        /* BUG 51041 crc update for D2&TM */
        ctc_port_set_property(gport, CTC_PORT_PROP_CHK_CRC_EN, FALSE);

        ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_EGRESS, 0x0);
        /* acl lkup use metadata used in tunnel inner match */
        ctc_port_set_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, TRUE);
#else
        ctc_port_set_property(gport, CTC_PORT_PROP_ERROR_CHECK, FALSE);
#endif
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_port_set_port_monitor_latencty_en(sai_object_id_t gport, uint32 enable)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    ctc_monitor_config_t monitor_config;

    sal_memset(&monitor_config, 0, sizeof(monitor_config));	

    monitor_config.gport = gport;
    monitor_config.value = enable;
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_EN;
    monitor_config.monitor_type = CTC_MONITOR_LATENCY;

    ret = ctc_monitor_set_config(&monitor_config);
    return ret;
}

sai_status_t
ctc_sai_port_set_monitor_latency_enable(sai_object_id_t port_id,const sai_attribute_t *attr)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    uint32_t gport = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ctc_sai_port_set_port_monitor_latencty_en(gport, attr->value.u32);
    return ret;
}

sai_status_t
ctc_sai_port_set_port_monitor_latencty_thrd(sai_object_id_t gport, uint32 *list)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    ctc_monitor_config_t monitor_config;
    
    monitor_config.gport = gport;
    monitor_config.value = list[0];
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MIN;
    monitor_config.monitor_type = CTC_MONITOR_LATENCY;
    ret = ctc_monitor_set_config(&monitor_config);

    monitor_config.gport = gport;
    monitor_config.value = list[1];
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MAX;
    monitor_config.monitor_type = CTC_MONITOR_LATENCY;
    ret = ctc_monitor_set_config(&monitor_config);

    return ret;
}

sai_status_t
ctc_sai_port_set_monitor_latency_threshold(sai_object_id_t port_id,const sai_attribute_t *attr)
{
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    uint32_t gport = CTC_SAI_OBJECT_INDEX_GET(port_id);

    ctc_sai_port_set_port_monitor_latencty_thrd(gport, attr->value.u32list.list);
    return ret;
}

#define ________SAI_PORT_DEBUG_FUNC
sai_status_t
ctc_sai_set_port_attribute_debug_param(
        sai_attribute_t *lag_ports,
	_In_ sai_object_id_t port_id,
	_In_ const sai_attribute_t *attr)
{
    uint32     policer_id = 0;  
    uint32_t  ctc_scheduler_id = 0;
    uint32_t  qos_map_id = 0;
    uint32_t        port_idx = 0;
    sai_object_id_t port_obj = 0;

    for(port_idx = 0; port_idx < lag_ports->value.objlist.count;port_idx++)
    {
        port_obj = lag_ports->value.objlist.list[port_idx];
        CTC_SAI_DEBUG("in:port_id 0x%llx", port_obj);

        switch (attr->id)
        {
        case SAI_PORT_ATTR_SPEED:
            switch(attr->value.u32)
            {
                case CTC_SAI_PORT_SPEED_10M:        /* 10M */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_10M");
                    break;
                case CTC_SAI_PORT_SPEED_100M:       /* 100M */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_100M");
                    break;
                case CTC_SAI_PORT_SPEED_1000M:      /* 1G */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_1000M");
                    break;
                case CTC_SAI_PORT_SPEED_2G5:      /* 2.5G */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_2G5");
                    break;
                case CTC_SAI_PORT_SPEED_10G:     /* 10G */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_10G");
                    break;
                case CTC_SAI_PORT_SPEED_40G:     /* 40G */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_40G");
                    break;
                case CTC_SAI_PORT_SPEED_100G:     /* 100G */
                    CTC_SAI_DEBUG("in:CTC_SAI_PORT_SPEED_100G");
                    break;
            }
            break;
        case SAI_PORT_ATTR_ADMIN_STATE:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_ADMIN_STATE %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_MEDIA_TYPE:
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            break;

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_PORT_VLAN_ID %u", attr->value.u16);
            break;

        case SAI_PORT_ATTR_VLANMAP_TYPE:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_VLANMAP_TYPE %u", attr->value.u16);
            break;

        case SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD %u", attr->value.booldata);
            break;

        case SAI_PORT_ATTR_UNTAG_PVID:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_UNTAG_PVID %u", attr->value.booldata);
            break;

        case SAI_PORT_ATTR_VLAN_RANGE_EN:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_VLAN_RANGE_EN %u", attr->value.booldata);
            break;

        case SAI_PORT_ATTR_INGRESS_FILTERING:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_INGRESS_FILTERING %u", attr->value.booldata);
            break;
            
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI %u", attr->value.aclfield.data.u8);
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI_MASK %u", attr->value.aclfield.mask.u8);
            break;

        case SAI_PORT_ATTR_FDB_LEARNING:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_FDB_LEARNING %d", attr->value.s32);
            break;

        case SAI_PORT_ATTR_UPDATE_DSCP:
            break;

        case SAI_PORT_ATTR_MTU:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_MTU %u", attr->value.u32);
            break;

        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID %u", policer_id);
            break;

        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID %u", policer_id);
            break;

        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID %u", policer_id);
            break;

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            switch(attr->value.s32)
            {
                case SAI_PORT_FLOW_CONTROL_DISABLE:
                    CTC_SAI_DEBUG("in:SAI_PORT_FLOW_CONTROL_DISABLE");
                    break;
                case SAI_PORT_FLOW_CONTROL_TX_ONLY:
                    CTC_SAI_DEBUG("in:SAI_PORT_FLOW_CONTROL_TX_ONLY");
                    break;
                case SAI_PORT_FLOW_CONTROL_RX_ONLY:
                    CTC_SAI_DEBUG("in:SAI_PORT_FLOW_CONTROL_RX_ONLY");
                    break;
                case SAI_PORT_FLOW_CONTROL_BOTH_ENABLE:
                    CTC_SAI_DEBUG("in:SAI_PORT_FLOW_CONTROL_BOTH_ENABLE");
                    break;
                default:
                    CTC_SAI_DEBUG("in:NOT SUPPORT");
                    break;
            }
            break;

        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
            break;

        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            switch(attr->value.s32)
            {
            case SAI_PACKET_ACTION_DROP:
                CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_DROP");
                break;
            case SAI_PACKET_ACTION_FORWARD:
                CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_FORWARD");
                break;
            case SAI_PACKET_ACTION_TRAP:
                CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_TRAP");
                break;
            case SAI_PACKET_ACTION_LOG:
                CTC_SAI_DEBUG("in:SAI_PACKET_ACTION_LOG");
                break;
            default:
                CTC_SAI_DEBUG("in:NOT SUPPORT");
                break;
            }
            break;
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            if (0 == attr->value.objlist.count)
            {
                CTC_SAI_DEBUG("in:SET_PORT_INGRESS_MIRROR_DISABLE");
            }
            else
            {
                CTC_SAI_DEBUG("in:SET_PORT_INGRESS_MIRROR_ENABLE");
            }
            break;
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            if (0 == attr->value.objlist.count)
            {
                CTC_SAI_DEBUG("in:SET_PORT_EGRESS_MIRROR_DISABLE");
            }
            else
            {
                CTC_SAI_DEBUG("in:SET_PORT_EGRESS_MIRROR_ENABLE");
            }
            break;

        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            break;
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            break;

        case SAI_PORT_ATTR_POLICER_ID:
            policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_POLICER_ID %u", policer_id);
            break;
        case SAI_PORT_ATTR_EGRESS_POLICER_ID:
            policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_EGRESS_POLICER_ID %u", policer_id);
            break;
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID %u", ctc_scheduler_id);
            break;
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP:    
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP:
            qos_map_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP %u", qos_map_id);
            break;
        case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_QOS_DEFAULT_TC");
            break;     
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL %u", attr->value.u8);
            break;
        case SAI_PORT_ATTR_ISOLATION_GROUP:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_ISOLATION_GROUP 0x%llx", attr->value.oid);
            break;
        case SAI_PORT_ATTR_ISOLATION_DIRECTION:
            CTC_SAI_DEBUG("in:SAI_PORT_ATTR_ISOLATION_DIRECTION %d", attr->value.s32);
            break;
        default :
            break;

        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_port_attribute_debug_param(
	_In_ sai_object_id_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list)
{
    uint32_t         attr_idx    = 0;
    sai_attribute_t* attr        = NULL;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;

        switch(attr->id)
        {
        case SAI_PORT_ATTR_TYPE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_TYPE");
            break;
        case SAI_PORT_ATTR_OPER_STATUS:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_OPER_STATUS %d", attr->value.s32);
            break;
        case SAI_PORT_ATTR_HW_LANE_LIST:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_HW_LANE_LIST");
            break;
        case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE");
            break;
        case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE");
            break;
        case SAI_PORT_ATTR_SPEED:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_SPEED %u", attr->value.u32);
            break;
        case SAI_PORT_ATTR_ADMIN_STATE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_ADMIN_STATE %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_MEDIA_TYPE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_MEDIA_TYPE");
            break;
        case SAI_PORT_ATTR_PORT_VLAN_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_PORT_VLAN_ID %u", attr->value.u16);
            break;
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY");
            break;
        case SAI_PORT_ATTR_VLANMAP_TYPE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INGRESS_FILTERING %u", attr->value.u16);
            break;
        case SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INGRESS_FILTERING %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_UNTAG_PVID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_UNTAG_PVID %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_VLAN_RANGE_EN:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_VLAN_RANGE_EN %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_INGRESS_FILTERING:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INGRESS_FILTERING %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_DROP_UNTAGGED %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_DROP_TAGGED:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_DROP_TAGGED %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INTERNAL_LOOPBACK");
            break;
        case SAI_PORT_ATTR_FDB_LEARNING:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_FDB_LEARNING");
            break;
        case SAI_PORT_ATTR_UPDATE_DSCP:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_UPDATE_DSCP");
            break;
        case SAI_PORT_ATTR_MTU:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_MTU %u", attr->value.u32);
            break;    
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID %u", attr->value.booldata);
            break;
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID %u", attr->value.booldata);
            break;    
        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL %d", attr->value.s32);
            break;
        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
            break;
        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION");
            break;
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INGRESS_MIRROR_SESSION");
            break;
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_EGRESS_MIRROR_SESSION %u", attr->value.objlist.count);
            break;
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE");
            break;
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE");
            break;
        case SAI_PORT_ATTR_QOS_QUEUE_LIST:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_QOS_QUEUE_LIST %u", attr->value.objlist.count);
            break;
        case SAI_PORT_ATTR_POLICER_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_POLICER_ID 0x%llx", attr->value.oid);
            break;
        case SAI_PORT_ATTR_EGRESS_POLICER_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_EGRESS_POLICER_ID 0x%llx", attr->value.oid);
            break;
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID 0x%llx", attr->value.oid);
            break;
        case SAI_PORT_ATTR_ISOLATION_GROUP:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_ISOLATION_GROUP 0x%llx", attr->value.oid);
            break;
        case SAI_PORT_ATTR_ISOLATION_DIRECTION:
            CTC_SAI_DEBUG("out:SAI_PORT_ATTR_ISOLATION_DIRECTION %d", attr->value.s32);
            break;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_port_stats_debug_param(_In_ sai_object_id_t port_id,
					_In_ const sai_port_stat_counter_t *counter_ids,
					_In_ uint32_t number_of_counters,
					_Out_ uint64_t* counters)
{
    uint32_t         attr_idx    = 0;

    CTC_SAI_DEBUG_PACKET("in:port_id 0x%llx number_of_counters %u", port_id, number_of_counters);
    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch (counter_ids[attr_idx])
        {
            case SAI_PORT_STAT_IF_IN_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_UCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_UCAST_PKTS 0x%"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_DISCARDS:
            case SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS:
            case SAI_PORT_STAT_IF_IN_VLAN_DISCARDS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_IN_DISCARDS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_ERRORS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_ERRORS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_BROADCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_BROADCAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_MULTICAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_MULTICAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_OCTETS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_UCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_UCAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_DISCARDS:
            case SAI_PORT_STAT_IF_OUT_QLEN:    
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_DISCARDS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_ERRORS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_ERRORS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS %"PRIu64, counters[attr_idx]);
                break; 
            case SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS %"PRIu64, counters[attr_idx]);
                break; 
            case SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS %"PRIu64, counters[attr_idx]);
                break; 
            case SAI_PORT_STAT_ETHER_STATS_FRAGMENTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_FRAGMENTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_JABBERS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_JABBERS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec/snd:SAI_PORT_STAT_ETHER_STATS_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_PKTS:
                CTC_SAI_DEBUG_PACKET("rec/snd:SAI_PORT_STAT_ETHER_STATS_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_COLLISIONS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_COLLISIONS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IP_IN_RECEIVES:
            case SAI_PORT_STAT_IP_IN_OCTETS:
            case SAI_PORT_STAT_IP_IN_UCAST_PKTS:
            case SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS:
            case SAI_PORT_STAT_IP_IN_DISCARDS:
            case SAI_PORT_STAT_IP_OUT_OCTETS:
            case SAI_PORT_STAT_IP_OUT_UCAST_PKTS:
            case SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS:
            case SAI_PORT_STAT_IP_OUT_DISCARDS:
            case SAI_PORT_STAT_IPV6_IN_RECEIVES:
            case SAI_PORT_STAT_IPV6_IN_OCTETS:
            case SAI_PORT_STAT_IPV6_IN_UCAST_PKTS:
            case SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS:
            case SAI_PORT_STAT_IPV6_IN_MCAST_PKTS:
            case SAI_PORT_STAT_IPV6_IN_DISCARDS:
            case SAI_PORT_STAT_IPV6_OUT_OCTETS:
            case SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS:
            case SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS:
            case SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS:
            case SAI_PORT_STAT_IPV6_OUT_DISCARDS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IP_IN_RECEIVES %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_PKTS %"PRIu64, counters[attr_idx]);
                break;  
            case SAI_PORT_STAT_IF_IN_OAM_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_OAM_PKTS %"PRIu64, counters[attr_idx]);
                break;                
            case SAI_PORT_STAT_IF_IN_RUNTS_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_RUNTS_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_GIANTS_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_GIANTS_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_FRAME_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_FRAME_PKTS %"PRIu64, counters[attr_idx]);
                break;  
            case SAI_PORT_STAT_IF_IN_PAUSE_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_PAUSE_PKTS %"PRIu64, counters[attr_idx]);
                break;  
            case SAI_PORT_STAT_IF_IN_BAD_PKTS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_BAD_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_IN_BAD_OCTETS:
                CTC_SAI_DEBUG_PACKET("rec:SAI_PORT_STAT_IF_IN_BAD_OCTETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_OAM_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_OAM_PKTS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS %"PRIu64, counters[attr_idx]);
                break;                
            case SAI_PORT_STAT_IF_OUT_PAUSE_PKTS:
                CTC_SAI_DEBUG_PACKET("snd:SAI_PORT_STAT_IF_OUT_PAUSE_PKTS %"PRIu64, counters[attr_idx]);
                break; 
            default:
                continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_PORT_API_FUNC
/*
* Routine Description:
*   Set port attribute value.
*
* Arguments:
*    [in] port_id - port id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_port_attribute(
	_In_ sai_object_id_t port_id,
	_In_ const sai_attribute_t *attr)
{
    sai_status_t            ret = SAI_STATUS_SUCCESS;
    uint32_t                port_idx = 0;
    uint32_t                port_max = CTC_MAX_LPORT;
    sai_object_id_t         port_obj = 0;
    sai_object_id_t         parr_port_id[CTC_MAX_LPORT];
    stbl_port_t             *p_sdb_port = NULL;
    sai_attribute_t         lag_ports;

    CTC_SAI_PTR_VALID_CHECK(attr);

    sal_memset(&lag_ports,0,sizeof(sai_attribute_t));

    ret = ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr);

    if(ret != SAI_STATUS_SUCCESS)
    {
        return ret;
    }

    if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
        if (NULL == p_sdb_port)
        {
            ret = SAI_STATUS_NO_MEMORY;
            goto out;
        }
        port_max = CTC_MAX_LPORT;
        lag_ports.value.objlist.list = parr_port_id;
        lag_ports.value.objlist.count= port_max;
        CTC_SAI_ERROR_GOTO(ctc_sai_lag_get_ports(port_id,&lag_ports),ret,out);
    }
    else
    {
        lag_ports.value.objlist.list = &port_id;
        lag_ports.value.objlist.count= 1;
    }

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_set_port_attribute_debug_param(&lag_ports, port_id, attr);

    for(port_idx = 0; port_idx < lag_ports.value.objlist.count;port_idx++)
    {
        port_obj = lag_ports.value.objlist.list[port_idx];
        switch(attr->id)
        {
        case SAI_PORT_ATTR_SPEED:
            ret = ctc_sai_port_set_speed(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ADMIN_STATE:
            ret = ctc_sai_port_set_admin_state(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MEDIA_TYPE:
            ret = ctc_sai_port_set_media_type(port_obj,attr);
            break;
        case SAI_PORT_ATTR_PORT_VLAN_ID:
            ret = ctc_sai_port_set_default_vlan(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            ret = ctc_sai_port_set_default_vlan_prioroty(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANMAP_TYPE:
            ret = ctc_sai_port_set_vlanmap_type(port_obj, attr);
            break;
        case SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD:
            ret = ctc_sai_port_set_vlanmap_default_fwd(port_obj, attr);
            break;
        case SAI_PORT_ATTR_UNTAG_PVID:
            ret = ctc_sai_port_set_untag_pvid(port_obj, attr);
            break;
        case SAI_PORT_ATTR_VLAN_RANGE_EN:
            ret = ctc_sai_port_set_vlan_range_en(port_obj, attr);
            break;
        case SAI_PORT_ATTR_UNTAGGED_VLAN:
            ret = ctc_sai_port_set_untagged_vlan(port_obj, attr);
            break;
        case SAI_PORT_ATTR_INGRESS_FILTERING:
            ret = ctc_sai_port_set_ingress_filtering(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            ret = ctc_sai_port_set_drop_untagged(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DROP_TAGGED:
            ret = ctc_sai_port_set_drop_tagged(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            ret = ctc_sai_port_set_loopback(port_obj,attr);
            break;
        case SAI_PORT_ATTR_FDB_LEARNING:
            ret = ctc_sai_port_set_fdb_learning(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DEFAULT_COS:
            ret = ctc_sai_port_set_default_cos(port_obj,attr);
            break;
        case SAI_PORT_ATTR_QOS_TRUST:
            ret = ctc_sai_port_set_qos_trust(port_obj,attr);
            break;
        case SAI_PORT_ATTR_UPDATE_DSCP:
            ret = ctc_sai_port_set_update_dscp(port_obj,attr);
            break;
        case SAI_PORT_ATTR_UPDATE_COS:
            ret = ctc_sai_port_set_update_cos(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MTU:
            ret = ctc_sai_port_set_mtu(port_obj,attr);
            break;
        case SAI_PORT_ATTR_IP_SOURCE_GUARD:
            ret = ctc_sai_port_set_ip_source_guard(port_obj, attr);
            break;            
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_set_flood_storm_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_set_broadcast_storm_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_set_multicast_storm_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            ret = ctc_sai_port_set_global_flow_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            ret = ctc_sai_port_set_max_learned_address(port_obj,attr);
            break;
        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            ret = ctc_sai_port_set_fdb_learning_limit_violation(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            ret = ctc_sai_port_set_ingress_mirror_session(port_obj,attr);
            break;
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            ret = ctc_sai_port_set_egress_mirror_session(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            ret = ctc_sai_port_set_ingress_samplepacket_enable(port_obj,attr);
            break;
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            ret = ctc_sai_port_set_egress_samplepacket_enable(port_obj,attr);
            break;
        case SAI_PORT_ATTR_POLICER_ID:
            ret = ctc_sai_policer_port_set_policer(port_obj,attr);
            break;
        case SAI_PORT_ATTR_EGRESS_POLICER_ID:
            ret = ctc_sai_policer_port_set_egress_policer(port_obj,attr);
            break;
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            ret = ctc_sai_scheduler_port_set_scheduler(port_obj,attr);
            break;
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP:
            ret = ctc_sai_qos_map_port_set_ctc_qos_map(port_obj,attr);
            break;
        case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            ctc_sai_qos_map_port_set_default_map_init(port_obj);
            break;     
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            ret = ctc_sai_port_set_priority_flow_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ISOLATION_GROUP:
            ret = ctc_sai_port_set_isolation_group(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ISOLATION_DIRECTION:
            ret = ctc_sai_port_set_isolation_direction(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ISOLATION_TYPE:
            ret = ctc_sai_port_set_isolation_type(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_GROUP_ID:
            ret = ctc_sai_port_set_vlanclass_group_id(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE:
            ret = ctc_sai_port_set_vlanclass_active_type(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_DEFAULT_ACTION:
            ret = ctc_sai_port_set_vlanclass_default_action(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DHCPRELAY_EXCEPTION_TYPE:
            ret = ctc_sai_port_set_dhcp_relay_exception_type(port_obj,attr);
            break;
#ifdef TAPPRODUCT            
        case SAI_PORT_ATTR_EGRESS_ACL_EN:
            ret = ctc_sai_port_set_egress_port_acl(port_obj, attr);
            break;
#endif            
        case SAI_PORT_ATTR_CUSTOM_TAP_USED:
            ret = ctc_sai_port_set_tap_used(port_obj,attr);
            break;
        case SAI_PORT_ATTR_SFLOW_EN:
            ret = ctc_sai_port_set_sflow_en(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MONITOR_LATENCY_EN:
            ret = ctc_sai_port_set_monitor_latency_enable(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MONITOR_LATENCY_THRESHOLD:
            ret = ctc_sai_port_set_monitor_latency_threshold(port_obj,attr);
            break;
        case SAI_PORT_ATTR_CUSTOM_METADATA:
            ret = ctc_sai_port_set_metadata(port_obj,attr);
            break;
        case SAI_PORT_ATTR_CUSTOM_SVLAN_TPID:
            ret = ctc_sai_port_set_set_svlan_tpid(port_obj,attr);
            break;
        case SAI_PORT_ATTR_CUSTOM_DOT1Q_ETHERTYPE:
            ret = ctc_sai_port_set_set_dot1q_ethertype(port_obj,attr);
            break;
        case SAI_PORT_ATTR_HASH_VALUE_PROFILE_ID:
            ret = ctc_sai_port_set_hash_value_profile_id(port_obj,attr);
            break;
#ifndef TAPPRODUCT            
        case SAI_PORT_ATTR_CUSTOM_RELFECTIVE_EN:
            ret = ctc_sai_port_set_reflective(port_obj,attr);
            break;
#endif            

/* system modified by liwh for bug 53010, 2019-07-29 */
#ifdef GREATBELT
        case SAI_PORT_ATTR_PORT_OAM_MAC:
            ret = ctc_sai_port_set_oam_mac(port_obj,attr);
            break;
#endif 
/* liwh end */

        case SAI_PORT_ATTR_PORT_CRC_ERROR_CHECK_EN:
            ret = ctc_sai_port_set_crc_error_check_en(port_obj,attr);
            break;

        case SAI_PORT_ATTR_PORT_CRC_RECALCULATION_EN:
            ret = ctc_sai_port_set_crc_recalculation_en(port_obj,attr);
            break;
            
        default :
            break;
        }
    }

    /* save agg db use to sync member_port */
    switch (attr->id)
    {
        case SAI_PORT_ATTR_EGRESS_ACL_EN:
            if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                p_sdb_port->egress_acl_en = attr->value.u32;
            }
            break;
        case SAI_PORT_ATTR_CUSTOM_METADATA:
            if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                p_sdb_port->metadata_en = attr->value.u32;
            }
            break;

        case SAI_PORT_ATTR_CUSTOM_RELFECTIVE_EN:
            if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                p_sdb_port->reflective_bridge_en = attr->value.u32;
            }
            break;
		case SAI_PORT_ATTR_DHCPRELAY_EXCEPTION_TYPE:
			if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                p_sdb_port->dhcprelay_exception = attr->value.s32;
            }
            break;
        case SAI_PORT_ATTR_VLANMAP_TYPE:
            ret = ctc_sai_port_set_vlanmap_type(port_id, attr);
            break;
        case SAI_PORT_ATTR_HASH_VALUE_AGG_HASH:
            if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
            {
                ret = ctc_sai_port_set_hash_value_agg_hash(port_id,attr);
            }
        default:
            break;
    }

out:
    return ret;
}

/*
* Routine Description:
*   Get port attribute value.
*
* Arguments:
*    [in] port_id - port id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_port_attribute(
	_In_ sai_object_id_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t* attr        = NULL;
    sai_attribute_t  lag_ports;
    sai_object_id_t port_obj = 0;
    uint32_t         attr_idx    = 0;
    sai_object_id_t oid_list[CTC_MAX_LPORT];

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:port_id %llx, attr_count 0x%u", port_id, attr_count);

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&lag_ports,0,sizeof(sai_attribute_t));

    if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        lag_ports.value.objlist.list = oid_list;
        CTC_SAI_ERROR_GOTO(ctc_sai_lag_get_ports(port_id,&lag_ports),ret,out);
        port_obj = lag_ports.value.objlist.list[0];
    }
    else
    {
        port_obj = port_id;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_PORT_ATTR_TYPE:
            ret = ctc_sai_port_get_type(port_id,attr);
            break;
        case SAI_PORT_ATTR_OPER_STATUS:
            ret = ctc_sai_port_get_operational_status(port_obj,attr);
            break;
        case SAI_PORT_ATTR_HW_LANE_LIST:
            ret= ctc_sai_port_get_hw_lane_list(port_obj,attr);
            break;
        case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE:
            ret= ctc_sai_port_get_supported_breakout_mode(port_obj,attr);
            break;
        case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE:
            ret= ctc_sai_port_get_current_breakout_mode(port_obj,attr);
            break;
        case SAI_PORT_ATTR_SPEED:
            ret = ctc_sai_port_get_speed(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ADMIN_STATE:
            ret = ctc_sai_port_get_admin_state(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MEDIA_TYPE:
            ret = ctc_sai_port_get_media_type(port_obj,attr);
            break;
        case SAI_PORT_ATTR_PORT_VLAN_ID:
            ret = ctc_sai_port_get_default_vlan(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            ret = ctc_sai_port_get_default_vlan_prioroty(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INGRESS_FILTERING:
            ret = ctc_sai_port_get_ingress_filtering(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            ret = ctc_sai_port_get_drop_untagged(port_obj,attr);
            break;
        case SAI_PORT_ATTR_DROP_TAGGED:
            ret = ctc_sai_port_get_drop_tagged(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            ret = ctc_sai_port_get_loopback(port_obj,attr);
            break;
        case SAI_PORT_ATTR_FDB_LEARNING:
            ret = ctc_sai_port_get_fdb_learning(port_obj,attr);
            break;
        case SAI_PORT_ATTR_UPDATE_DSCP:
            ret = ctc_sai_port_get_update_dscp(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MTU:
            ret = ctc_sai_port_get_mtu(port_obj,attr);
            break;
    
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_get_flood_storm_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_get_broadcast_storm_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            ret = ctc_sai_port_get_multicast_storm_control(port_obj,attr);
            break;
    
        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            ret = ctc_sai_port_get_global_flow_control(port_obj,attr);
            break;
        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            ret = ctc_sai_port_get_max_learned_address(port_obj,attr);
            break;
        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            ret = ctc_sai_port_get_fdb_learning_limit_violation(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            ret = ctc_sai_port_get_ingress_mirror_session(port_obj,attr);
            break;
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            ret = ctc_sai_port_get_egress_mirror_session(port_obj,attr);
            break;
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            ret = ctc_sai_port_get_ingress_samplepacket_enable(port_obj,attr);
            break;
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            ret = ctc_sai_port_get_egress_samplepacket_enable(port_obj,attr);
            break;
        case SAI_PORT_ATTR_QOS_QUEUE_LIST:
            ret = ctc_sai_port_get_queue_list(port_obj,attr);
            break;
        case SAI_PORT_ATTR_POLICER_ID:
            ret = ctc_sai_policer_get_port_policer_id(port_obj, attr);
            break;
        case SAI_PORT_ATTR_EGRESS_POLICER_ID:
            ret = ctc_sai_policer_get_port_egress_policer_id(port_obj, attr);
            break;
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            ret = ctc_sai_scheduler_get_port_scheduler_id(port_obj, attr);
            break;
        case SAI_PORT_ATTR_ISOLATION_GROUP:
            ret = ctc_sai_port_get_isolation_group(port_obj,attr);
            break;
        case SAI_PORT_ATTR_ISOLATION_DIRECTION:
            ret = ctc_sai_port_get_isolation_direction(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_GROUP_ID:
            ret = ctc_sai_port_get_vlanclass_group_id(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE:
            ret = ctc_sai_port_get_vlanclass_active_type(port_obj,attr);
            break;
        case SAI_PORT_ATTR_VLANCLASS_DEFAULT_ACTION:
            ret = ctc_sai_port_get_vlanclass_default_action(port_obj,attr);
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }
    ctc_sai_get_port_attribute_debug_param(port_id, attr_count, attr_list);
out:
    return ret;
}

/**
 * Routine Description:
 *   @brief Clear port statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] counter_ids - specifies the array of counter ids
 *    @param[in] number_of_counters - number of counters in the array
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
 sai_status_t 
ctc_sai_clear_port_stats(_In_ sai_object_id_t port_id,
                                _In_ const sai_port_stat_counter_t *counter_ids,
                                _In_ uint32_t number_of_counters)
{
    CTC_SAI_DEBUG_FUNC();

    return SAI_STATUS_NOT_SUPPORTED;
}



/**
 * Routine Description:
 *   @brief Clear port's all statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t ctc_sai_clear_port_all_stats(_In_ sai_object_id_t port_id)
{
    stbl_port_t *p_sdb_port = NULL;

    CTC_SAI_DEBUG_FUNC();

    if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        ctc_stats_clear_mac_stats(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_STATS_MAC_STATS_RX);
        ctc_stats_clear_mac_stats(CTC_SAI_OBJECT_INDEX_GET(port_id), CTC_STATS_MAC_STATS_TX);

        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
        if (p_sdb_port)
        {
            if (p_sdb_port->rx_stats_ptr.p_stats)
            {
                sal_memset(p_sdb_port->rx_stats_ptr.p_stats, 0x0, sizeof(ctc_stats_mac_rec_t));
            }
            if (p_sdb_port->tx_stats_ptr.p_stats)
            {
                sal_memset(p_sdb_port->tx_stats_ptr.p_stats, 0x0, sizeof(ctc_stats_mac_snd_t));
            }
        }
    }
    else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
        if (p_sdb_port)
        {
            if (p_sdb_port->rx_stats_ptr.p_stats)
            {
                sal_memset(p_sdb_port->rx_stats_ptr.p_stats, 0x0, sizeof(ctc_stats_mac_rec_t));
            }
            if (p_sdb_port->tx_stats_ptr.p_stats)
            {
                sal_memset(p_sdb_port->tx_stats_ptr.p_stats, 0x0, sizeof(ctc_stats_mac_snd_t));
            }
        }
    }
    
    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_map_ctc_to_sai(uint32_t number_of_counters, const sai_port_stat_counter_t *counter_ids, uint64_t *counters, 
                        ctc_stats_mac_rec_t *pmac_rec, ctc_stats_mac_snd_t *pmac_snd)
{
    uint32_t         attr_idx = 0;

    for (attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
        case SAI_PORT_STAT_IF_IN_OCTETS:
            counters[attr_idx] = pmac_rec->good_63_bytes+ \
                                 pmac_rec->bad_63_bytes + \
                                 pmac_rec->bytes_64 + \
                                 pmac_rec->bytes_65_to_127 + \
                                 pmac_rec->bytes_128_to_255 + \
                                 pmac_rec->bytes_256_to_511 + \
                                 pmac_rec->bytes_512_to_1023 + \
                                 pmac_rec->bytes_1024_to_1518 + \
                                 pmac_rec->good_1519_bytes + \
                                 pmac_rec->bad_1519_bytes + \
                                 pmac_rec->good_jumbo_bytes + \
                                 pmac_rec->bad_jumbo_bytes;
            break;
            
        case SAI_PORT_STAT_IF_IN_UCAST_PKTS:
            counters[attr_idx] = pmac_rec->good_ucast_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS:
            counters[attr_idx] = pmac_rec->good_mcast_pkts + \
                                 pmac_rec->good_bcast_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_DISCARDS:
        case SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS:
        case SAI_PORT_STAT_IF_IN_VLAN_DISCARDS:
            counters[attr_idx] = 0;
            break;
        case SAI_PORT_STAT_IF_IN_ERRORS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts + \
                                 pmac_rec->fcs_error_pkts + \
                                 pmac_rec->alignment_error_pkts + \
                                 pmac_rec->mac_overrun_pkts;
            break;

        case SAI_PORT_STAT_IF_IN_BROADCAST_PKTS:
            counters[attr_idx] = pmac_rec->good_bcast_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_MULTICAST_PKTS:
            counters[attr_idx] = pmac_rec->good_mcast_pkts;
            break;

        case SAI_PORT_STAT_IF_OUT_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_63 + \
                                 pmac_snd->bytes_64 + \
                                 pmac_snd->bytes_65_to_127 + \
                                 pmac_snd->bytes_128_to_255 + \
                                 pmac_snd->bytes_256_to_511 + \
                                 pmac_snd->bytes_512_to_1023 + \
                                 pmac_snd->bytes_1024_to_1518 + \
                                 pmac_snd->bytes_1519 + \
                                 pmac_snd->jumbo_bytes;
            break;
        case SAI_PORT_STAT_IF_OUT_UCAST_PKTS:
            counters[attr_idx] = pmac_snd->good_ucast_pkts;
            break;
        case SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS:
            counters[attr_idx] = pmac_snd->good_bcast_pkts + \
                                 pmac_snd->good_mcast_pkts;
            break;
        case SAI_PORT_STAT_IF_OUT_DISCARDS:
        case SAI_PORT_STAT_IF_OUT_QLEN:    
            counters[attr_idx] = 0;
            break;
        case SAI_PORT_STAT_IF_OUT_ERRORS:
            counters[attr_idx] = pmac_snd->fcs_error_pkts;
            break;

        case SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS:
            counters[attr_idx] = pmac_snd->good_bcast_pkts;
            break;
        case SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS:
            counters[attr_idx] = pmac_snd->good_mcast_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS:
            counters[attr_idx] = pmac_rec->mac_overrun_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS:
            counters[attr_idx] = pmac_snd->good_mcast_pkts + \
                                 pmac_rec->good_mcast_pkts;
            break; 
        case SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS:
            counters[attr_idx] = pmac_snd->good_bcast_pkts + \
                                 pmac_rec->good_bcast_pkts;
            break; 
        case SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS:
            counters[attr_idx] = pmac_rec->good_undersize_pkts;
            break; 
        case SAI_PORT_STAT_ETHER_STATS_FRAGMENTS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_64 + \
                                 pmac_rec->bytes_64;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_65_to_127 + \
                                 pmac_rec->bytes_65_to_127;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_128_to_255 + \
                                 pmac_rec->bytes_128_to_255;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_256_to_511 + \
                                 pmac_rec->bytes_256_to_511;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_512_to_1023 + \
                                 pmac_rec->bytes_512_to_1023;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS:
            counters[attr_idx] = pmac_snd->bytes_1024_to_1518 + \
                                 pmac_rec->bytes_1024_to_1518;
            break;
        case SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS:
            counters[attr_idx] = pmac_rec->good_1519_pkts+ \
                                 pmac_rec->good_jumbo_pkts + \
                                 pmac_snd->pkts_1519 + \
                                 pmac_snd->jumbo_pkts;
            break;
        case SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS:
            counters[attr_idx] = pmac_rec->good_1519_pkts+ \
                                 pmac_rec->good_jumbo_pkts;
            break;
        case SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS:
            counters[attr_idx] = pmac_snd->pkts_1519 + \
                                 pmac_snd->jumbo_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_JABBERS:
            counters[attr_idx] = pmac_rec->jabber_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_OCTETS:
            counters[attr_idx] = pmac_rec->good_63_bytes + \
                                 pmac_rec->bad_63_bytes + \
                                 pmac_rec->bytes_64 + \
                                 pmac_rec->bytes_65_to_127 + \
                                 pmac_rec->bytes_128_to_255 + \
                                 pmac_rec->bytes_256_to_511 + \
                                 pmac_rec->bytes_512_to_1023 + \
                                 pmac_rec->bytes_1024_to_1518 + \
                                 pmac_rec->good_1519_bytes + \
                                 pmac_rec->bad_1519_bytes + \
                                 pmac_rec->good_jumbo_bytes + \
                                 pmac_rec->bad_jumbo_bytes + \
                                 pmac_snd->bytes_63 + \
                                 pmac_snd->bytes_64 + \
                                 pmac_snd->bytes_65_to_127 + \
                                 pmac_snd->bytes_128_to_255 + \
                                 pmac_snd->bytes_256_to_511 + \
                                 pmac_snd->bytes_512_to_1023 + \
                                 pmac_snd->bytes_1024_to_1518 + \
                                 pmac_snd->bytes_1519 + \
                                 pmac_snd->jumbo_bytes;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts + \
                                 pmac_rec->pkts_64 + \
                                 pmac_rec->pkts_65_to_127 + \
                                 pmac_rec->pkts_128_to_255 + \
                                 pmac_rec->pkts_256_to_511 + \
                                 pmac_rec->pkts_512_to_1023 + \
                                 pmac_rec->pkts_1024_to_1518 + \
                                 pmac_rec->good_1519_pkts + \
                                 pmac_rec->bad_1519_pkts + \
                                 pmac_rec->good_jumbo_pkts + \
                                 pmac_rec->bad_jumbo_pkts + \
                                 pmac_snd->pkts_63 + \
                                 pmac_snd->pkts_64 + \
                                 pmac_snd->pkts_65_to_127 + \
                                 pmac_snd->pkts_128_to_255 + \
                                 pmac_snd->pkts_256_to_511 + \
                                 pmac_snd->pkts_512_to_1023 + \
                                 pmac_snd->pkts_1024_to_1518 + \
                                 pmac_snd->pkts_1519 + \
                                 pmac_snd->jumbo_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_COLLISIONS:
            counters[attr_idx] = pmac_rec->collision_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS:
            counters[attr_idx] = pmac_rec->alignment_error_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS:
            counters[attr_idx] = pmac_snd->pkts_63 + \
                                 pmac_snd->pkts_64 + \
                                 pmac_snd->pkts_65_to_127 + \
                                 pmac_snd->pkts_128_to_255 + \
                                 pmac_snd->pkts_256_to_511 + \
                                 pmac_snd->pkts_512_to_1023 + \
                                 pmac_snd->pkts_1024_to_1518 + \
                                 pmac_snd->pkts_1519 + \
                                 pmac_snd->jumbo_pkts - \
                                 pmac_snd->fcs_error_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts + \
                                 pmac_rec->pkts_64 + \
                                 pmac_rec->pkts_65_to_127 + \
                                 pmac_rec->pkts_128_to_255 + \
                                 pmac_rec->pkts_256_to_511 + \
                                 pmac_rec->pkts_512_to_1023 + \
                                 pmac_rec->pkts_1024_to_1518 + \
                                 pmac_rec->good_1519_pkts + \
                                 pmac_rec->bad_1519_pkts + \
                                 pmac_rec->good_jumbo_pkts + \
                                 pmac_rec->bad_jumbo_pkts + \
                                 pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts - \
                                 pmac_rec->fcs_error_pkts - \
                                 pmac_rec->alignment_error_pkts - \
                                 pmac_rec->mac_overrun_pkts;
            break;
        case SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS:
        case SAI_PORT_STAT_IP_IN_RECEIVES:
        case SAI_PORT_STAT_IP_IN_OCTETS:
        case SAI_PORT_STAT_IP_IN_UCAST_PKTS:
        case SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IP_IN_DISCARDS:
        case SAI_PORT_STAT_IP_OUT_OCTETS:
        case SAI_PORT_STAT_IP_OUT_UCAST_PKTS:
        case SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IP_OUT_DISCARDS:
        case SAI_PORT_STAT_IPV6_IN_RECEIVES:
        case SAI_PORT_STAT_IPV6_IN_OCTETS:
        case SAI_PORT_STAT_IPV6_IN_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_MCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_DISCARDS:
        case SAI_PORT_STAT_IPV6_OUT_OCTETS:
        case SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_DISCARDS:
        case SAI_PORT_STAT_GREEN_DISCARD_DROPPED_PACKETS:
        case SAI_PORT_STAT_GREEN_DISCARD_DROPPED_BYTES:
        case SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_PACKETS:
        case SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_BYTES:
        case SAI_PORT_STAT_RED_DISCARD_DROPPED_PACKETS:
        case SAI_PORT_STAT_RED_DISCARD_DROPPED_BYTES:
        case SAI_PORT_STAT_DISCARD_DROPPED_PACKETS:
        case SAI_PORT_STAT_DISCARD_DROPPED_BYTES:
            counters[attr_idx] = 0;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_64;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_65_to_127;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_128_to_255;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_256_to_511;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_512_to_1023;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS:
            counters[attr_idx] = pmac_rec->pkts_1024_to_1518;
            break;
            
        case SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS:
            counters[attr_idx] = 0;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_64;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_64;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_128_to_255;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_256_to_511;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_512_to_1023;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_1024_to_1518;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS:
            counters[attr_idx] = pmac_snd->pkts_1519;
            break;
            
        case SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS: 
            counters[attr_idx] = 0;
            break;
            
        case SAI_PORT_STAT_IF_IN_PKTS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts + \
                                 pmac_rec->pkts_64 + \
                                 pmac_rec->pkts_65_to_127 + \
                                 pmac_rec->pkts_128_to_255 + \
                                 pmac_rec->pkts_256_to_511 + \
                                 pmac_rec->pkts_512_to_1023 + \
                                 pmac_rec->pkts_1024_to_1518 + \
                                 pmac_rec->good_1519_pkts + \
                                 pmac_rec->bad_1519_pkts + \
                                 pmac_rec->good_jumbo_pkts + \
                                 pmac_rec->bad_jumbo_pkts;
            break;  
        case SAI_PORT_STAT_IF_IN_OAM_PKTS:
            counters[attr_idx] = pmac_rec->gmac_good_oam_pkts;
            break;                
        case SAI_PORT_STAT_IF_IN_RUNTS_PKTS:
            counters[attr_idx] = pmac_rec->good_undersize_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_GIANTS_PKTS:
            counters[attr_idx] = pmac_rec->good_1519_pkts + \
                                 pmac_rec->good_jumbo_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS:
            counters[attr_idx] = pmac_rec->fcs_error_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS:
            counters[attr_idx] = pmac_rec->fcs_error_bytes;
            break;
        case SAI_PORT_STAT_IF_IN_FRAME_PKTS:
            counters[attr_idx] = pmac_rec->good_63_pkts + \
                                 pmac_rec->bad_63_pkts;
            break;  
        case SAI_PORT_STAT_IF_IN_PAUSE_PKTS:
            counters[attr_idx] = pmac_rec->good_pause_pkts;
            break;  

        case SAI_PORT_STAT_IF_IN_BAD_PKTS:
            counters[attr_idx] = pmac_rec->good_undersize_pkts + \
                                 pmac_rec->jabber_pkts + \
                                 pmac_rec->collision_pkts + \
                                 pmac_rec->fcs_error_pkts + \
                                 pmac_rec->alignment_error_pkts;
            break;
        case SAI_PORT_STAT_IF_IN_BAD_OCTETS:
            counters[attr_idx] = pmac_rec->good_undersize_bytes + \
                                 pmac_rec->jabber_bytes + \
                                 pmac_rec->collision_bytes + \
                                 pmac_rec->fcs_error_bytes + \
                                 pmac_rec->alignment_error_bytes;
            break;
            
        case SAI_PORT_STAT_IF_OUT_PKTS:
            counters[attr_idx] = pmac_snd->pkts_63 + \
                                 pmac_snd->pkts_64 + \
                                 pmac_snd->pkts_65_to_127 + \
                                 pmac_snd->pkts_128_to_255 + \
                                 pmac_snd->pkts_256_to_511 + \
                                 pmac_snd->pkts_512_to_1023 + \
                                 pmac_snd->pkts_1024_to_1518 + \
                                 pmac_snd->pkts_1519 + \
                                 pmac_snd->jumbo_pkts;
            break;

        case SAI_PORT_STAT_IF_OUT_OAM_PKTS:
            counters[attr_idx] = pmac_snd->good_oam_pkts;
            break;
        case SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS:
            counters[attr_idx] = pmac_snd->fcs_error_bytes;
            break;
            
        case SAI_PORT_STAT_IF_OUT_PAUSE_PKTS:
            counters[attr_idx] = pmac_snd->good_pause_pkts;
            break; 

        default:
            continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static int32
_ctc_sai_update_lag_stats(ctc_stats_mac_rec_t *p_lag_rec, ctc_stats_mac_snd_t *p_lag_snd, 
                          ctc_stats_mac_rec_t *p_port_rec, ctc_stats_mac_snd_t *p_port_snd, uint32 is_add)
{
    if (is_add)
    {
        p_lag_rec->good_ucast_pkts           += p_port_rec->good_ucast_pkts;
        p_lag_rec->good_ucast_bytes          += p_port_rec->good_ucast_bytes;
        p_lag_rec->good_mcast_pkts           += p_port_rec->good_mcast_pkts;
        p_lag_rec->good_mcast_bytes          += p_port_rec->good_mcast_bytes;
        p_lag_rec->good_bcast_pkts           += p_port_rec->good_bcast_pkts;
        p_lag_rec->good_bcast_bytes          += p_port_rec->good_bcast_bytes;
        p_lag_rec->good_pause_pkts           += p_port_rec->good_pause_pkts;
        p_lag_rec->good_pause_bytes          += p_port_rec->good_pause_bytes;
        p_lag_rec->good_normal_pause_pkts    += p_port_rec->good_normal_pause_pkts;
        p_lag_rec->good_normal_pause_bytes   += p_port_rec->good_normal_pause_bytes;
        p_lag_rec->good_pfc_pause_pkts       += p_port_rec->good_pfc_pause_pkts;
        p_lag_rec->good_pfc_pause_bytes      += p_port_rec->good_pfc_pause_bytes;
        p_lag_rec->good_control_pkts         += p_port_rec->good_control_pkts;
        p_lag_rec->good_control_bytes        += p_port_rec->good_control_bytes;
        p_lag_rec->jabber_pkts               += p_port_rec->jabber_pkts;
        p_lag_rec->jabber_bytes              += p_port_rec->jabber_bytes;
        p_lag_rec->collision_pkts            += p_port_rec->collision_pkts;
        p_lag_rec->collision_bytes           += p_port_rec->collision_bytes;
        p_lag_rec->fcs_error_pkts            += p_port_rec->fcs_error_pkts;
        p_lag_rec->fcs_error_bytes           += p_port_rec->fcs_error_bytes;
        p_lag_rec->alignment_error_pkts      += p_port_rec->alignment_error_pkts;
        p_lag_rec->alignment_error_bytes     += p_port_rec->alignment_error_bytes;
        p_lag_rec->mac_overrun_pkts          += p_port_rec->mac_overrun_pkts;
        p_lag_rec->mac_overrun_bytes         += p_port_rec->mac_overrun_bytes;
        p_lag_rec->good_oversize_pkts        += p_port_rec->good_oversize_pkts;
        p_lag_rec->good_oversize_bytes       += p_port_rec->good_oversize_bytes;
        p_lag_rec->good_undersize_pkts       += p_port_rec->good_undersize_pkts;
        p_lag_rec->good_undersize_bytes      += p_port_rec->good_undersize_bytes;
        p_lag_rec->gmac_good_oam_pkts        += p_port_rec->gmac_good_oam_pkts;
        p_lag_rec->gmac_good_oam_bytes       += p_port_rec->gmac_good_oam_bytes;
        p_lag_rec->good_63_pkts              += p_port_rec->good_63_pkts;
        p_lag_rec->good_63_bytes             += p_port_rec->good_63_bytes;
        p_lag_rec->bad_63_pkts               += p_port_rec->bad_63_pkts;
        p_lag_rec->bad_63_bytes              += p_port_rec->bad_63_bytes;
        p_lag_rec->good_1519_pkts            += p_port_rec->good_1519_pkts;
        p_lag_rec->good_1519_bytes           += p_port_rec->good_1519_bytes;
        p_lag_rec->bad_1519_pkts             += p_port_rec->bad_1519_pkts;
        p_lag_rec->bad_1519_bytes            += p_port_rec->bad_1519_bytes;
        p_lag_rec->good_jumbo_pkts           += p_port_rec->good_jumbo_pkts;
        p_lag_rec->good_jumbo_bytes          += p_port_rec->good_jumbo_bytes;
        p_lag_rec->bad_jumbo_pkts            += p_port_rec->bad_jumbo_pkts;
        p_lag_rec->bad_jumbo_bytes           += p_port_rec->bad_jumbo_bytes;
        p_lag_rec->pkts_64                   += p_port_rec->pkts_64;
        p_lag_rec->bytes_64                  += p_port_rec->bytes_64;
        p_lag_rec->pkts_65_to_127            += p_port_rec->pkts_65_to_127;
        p_lag_rec->bytes_65_to_127           += p_port_rec->bytes_65_to_127;
        p_lag_rec->pkts_128_to_255           += p_port_rec->pkts_128_to_255;
        p_lag_rec->bytes_128_to_255          += p_port_rec->bytes_128_to_255;
        p_lag_rec->pkts_256_to_511           += p_port_rec->pkts_256_to_511;
        p_lag_rec->bytes_256_to_511          += p_port_rec->bytes_256_to_511;
        p_lag_rec->pkts_512_to_1023          += p_port_rec->pkts_512_to_1023;
        p_lag_rec->bytes_512_to_1023         += p_port_rec->bytes_512_to_1023;
        p_lag_rec->pkts_1024_to_1518         += p_port_rec->pkts_1024_to_1518;
        p_lag_rec->bytes_1024_to_1518        += p_port_rec->bytes_1024_to_1518;

        p_lag_snd->good_ucast_pkts           += p_port_snd->good_ucast_pkts;
        p_lag_snd->good_ucast_bytes          += p_port_snd->good_ucast_bytes;
        p_lag_snd->good_mcast_pkts           += p_port_snd->good_mcast_pkts;
        p_lag_snd->good_mcast_bytes          += p_port_snd->good_mcast_bytes;
        p_lag_snd->good_bcast_pkts           += p_port_snd->good_bcast_pkts;
        p_lag_snd->good_bcast_bytes          += p_port_snd->good_bcast_bytes;
        p_lag_snd->good_pause_pkts           += p_port_snd->good_pause_pkts;
        p_lag_snd->good_pause_bytes          += p_port_snd->good_pause_bytes;
        p_lag_snd->good_control_pkts         += p_port_snd->good_control_pkts;
        p_lag_snd->good_control_bytes        += p_port_snd->good_control_bytes;
        p_lag_snd->good_oam_pkts             += p_port_snd->good_oam_pkts;
        p_lag_snd->good_oam_bytes            += p_port_snd->good_oam_bytes;
        p_lag_snd->pkts_63                   += p_port_snd->pkts_63;
        p_lag_snd->bytes_63                  += p_port_snd->bytes_63;
        p_lag_snd->pkts_64                   += p_port_snd->pkts_64;
        p_lag_snd->bytes_64                  += p_port_snd->bytes_64;
        p_lag_snd->pkts_65_to_127            += p_port_snd->pkts_65_to_127;
        p_lag_snd->bytes_65_to_127           += p_port_snd->bytes_65_to_127;
        p_lag_snd->pkts_128_to_255           += p_port_snd->pkts_128_to_255;
        p_lag_snd->bytes_128_to_255          += p_port_snd->bytes_128_to_255;
        p_lag_snd->pkts_256_to_511           += p_port_snd->pkts_256_to_511;
        p_lag_snd->bytes_256_to_511          += p_port_snd->bytes_256_to_511;
        p_lag_snd->pkts_512_to_1023          += p_port_snd->pkts_512_to_1023;
        p_lag_snd->bytes_512_to_1023         += p_port_snd->bytes_512_to_1023;
        p_lag_snd->pkts_1024_to_1518         += p_port_snd->pkts_1024_to_1518;
        p_lag_snd->bytes_1024_to_1518        += p_port_snd->bytes_1024_to_1518;
        p_lag_snd->pkts_1519                 += p_port_snd->pkts_1519;
        p_lag_snd->bytes_1519                += p_port_snd->bytes_1519;
        p_lag_snd->jumbo_pkts                += p_port_snd->jumbo_pkts;
        p_lag_snd->jumbo_bytes               += p_port_snd->jumbo_bytes;
        p_lag_snd->mac_underrun_pkts         += p_port_snd->mac_underrun_pkts;
        p_lag_snd->mac_underrun_bytes        += p_port_snd->mac_underrun_bytes;
        p_lag_snd->fcs_error_pkts            += p_port_snd->fcs_error_pkts;
        p_lag_snd->fcs_error_bytes           += p_port_snd->fcs_error_bytes;
    }
    else
    {
        p_lag_rec->good_ucast_pkts           -= p_port_rec->good_ucast_pkts;
        p_lag_rec->good_ucast_bytes          -= p_port_rec->good_ucast_bytes;
        p_lag_rec->good_mcast_pkts           -= p_port_rec->good_mcast_pkts;
        p_lag_rec->good_mcast_bytes          -= p_port_rec->good_mcast_bytes;
        p_lag_rec->good_bcast_pkts           -= p_port_rec->good_bcast_pkts;
        p_lag_rec->good_bcast_bytes          -= p_port_rec->good_bcast_bytes;
        p_lag_rec->good_pause_pkts           -= p_port_rec->good_pause_pkts;
        p_lag_rec->good_pause_bytes          -= p_port_rec->good_pause_bytes;
        p_lag_rec->good_normal_pause_pkts    -= p_port_rec->good_normal_pause_pkts;
        p_lag_rec->good_normal_pause_bytes   -= p_port_rec->good_normal_pause_bytes;
        p_lag_rec->good_pfc_pause_pkts       -= p_port_rec->good_pfc_pause_pkts;
        p_lag_rec->good_pfc_pause_bytes      -= p_port_rec->good_pfc_pause_bytes;
        p_lag_rec->good_control_pkts         -= p_port_rec->good_control_pkts;
        p_lag_rec->good_control_bytes        -= p_port_rec->good_control_bytes;
        p_lag_rec->jabber_pkts               -= p_port_rec->jabber_pkts;
        p_lag_rec->jabber_bytes              -= p_port_rec->jabber_bytes;
        p_lag_rec->collision_pkts            -= p_port_rec->collision_pkts;
        p_lag_rec->collision_bytes           -= p_port_rec->collision_bytes;
        p_lag_rec->fcs_error_pkts            -= p_port_rec->fcs_error_pkts;
        p_lag_rec->fcs_error_bytes           -= p_port_rec->fcs_error_bytes;
        p_lag_rec->alignment_error_pkts      -= p_port_rec->alignment_error_pkts;
        p_lag_rec->alignment_error_bytes     -= p_port_rec->alignment_error_bytes;
        p_lag_rec->mac_overrun_pkts          -= p_port_rec->mac_overrun_pkts;
        p_lag_rec->mac_overrun_bytes         -= p_port_rec->mac_overrun_bytes;
        p_lag_rec->good_oversize_pkts        -= p_port_rec->good_oversize_pkts;
        p_lag_rec->good_oversize_bytes       -= p_port_rec->good_oversize_bytes;
        p_lag_rec->good_undersize_pkts       -= p_port_rec->good_undersize_pkts;
        p_lag_rec->good_undersize_bytes      -= p_port_rec->good_undersize_bytes;
        p_lag_rec->gmac_good_oam_pkts        -= p_port_rec->gmac_good_oam_pkts;
        p_lag_rec->gmac_good_oam_bytes       -= p_port_rec->gmac_good_oam_bytes;
        p_lag_rec->good_63_pkts              -= p_port_rec->good_63_pkts;
        p_lag_rec->good_63_bytes             -= p_port_rec->good_63_bytes;
        p_lag_rec->bad_63_pkts               -= p_port_rec->bad_63_pkts;
        p_lag_rec->bad_63_bytes              -= p_port_rec->bad_63_bytes;
        p_lag_rec->good_1519_pkts            -= p_port_rec->good_1519_pkts;
        p_lag_rec->good_1519_bytes           -= p_port_rec->good_1519_bytes;
        p_lag_rec->bad_1519_pkts             -= p_port_rec->bad_1519_pkts;
        p_lag_rec->bad_1519_bytes            -= p_port_rec->bad_1519_bytes;
        p_lag_rec->good_jumbo_pkts           -= p_port_rec->good_jumbo_pkts;
        p_lag_rec->good_jumbo_bytes          -= p_port_rec->good_jumbo_bytes;
        p_lag_rec->bad_jumbo_pkts            -= p_port_rec->bad_jumbo_pkts;
        p_lag_rec->bad_jumbo_bytes           -= p_port_rec->bad_jumbo_bytes;
        p_lag_rec->pkts_64                   -= p_port_rec->pkts_64;
        p_lag_rec->bytes_64                  -= p_port_rec->bytes_64;
        p_lag_rec->pkts_65_to_127            -= p_port_rec->pkts_65_to_127;
        p_lag_rec->bytes_65_to_127           -= p_port_rec->bytes_65_to_127;
        p_lag_rec->pkts_128_to_255           -= p_port_rec->pkts_128_to_255;
        p_lag_rec->bytes_128_to_255          -= p_port_rec->bytes_128_to_255;
        p_lag_rec->pkts_256_to_511           -= p_port_rec->pkts_256_to_511;
        p_lag_rec->bytes_256_to_511          -= p_port_rec->bytes_256_to_511;
        p_lag_rec->pkts_512_to_1023          -= p_port_rec->pkts_512_to_1023;
        p_lag_rec->bytes_512_to_1023         -= p_port_rec->bytes_512_to_1023;
        p_lag_rec->pkts_1024_to_1518         -= p_port_rec->pkts_1024_to_1518;
        p_lag_rec->bytes_1024_to_1518        -= p_port_rec->bytes_1024_to_1518;

        p_lag_snd->good_ucast_pkts           -= p_port_snd->good_ucast_pkts;
        p_lag_snd->good_ucast_bytes          -= p_port_snd->good_ucast_bytes;
        p_lag_snd->good_mcast_pkts           -= p_port_snd->good_mcast_pkts;
        p_lag_snd->good_mcast_bytes          -= p_port_snd->good_mcast_bytes;
        p_lag_snd->good_bcast_pkts           -= p_port_snd->good_bcast_pkts;
        p_lag_snd->good_bcast_bytes          -= p_port_snd->good_bcast_bytes;
        p_lag_snd->good_pause_pkts           -= p_port_snd->good_pause_pkts;
        p_lag_snd->good_pause_bytes          -= p_port_snd->good_pause_bytes;
        p_lag_snd->good_control_pkts         -= p_port_snd->good_control_pkts;
        p_lag_snd->good_control_bytes        -= p_port_snd->good_control_bytes;
        p_lag_snd->good_oam_pkts             -= p_port_snd->good_oam_pkts;
        p_lag_snd->good_oam_bytes            -= p_port_snd->good_oam_bytes;
        p_lag_snd->pkts_63                   -= p_port_snd->pkts_63;
        p_lag_snd->bytes_63                  -= p_port_snd->bytes_63;
        p_lag_snd->pkts_64                   -= p_port_snd->pkts_64;
        p_lag_snd->bytes_64                  -= p_port_snd->bytes_64;
        p_lag_snd->pkts_65_to_127            -= p_port_snd->pkts_65_to_127;
        p_lag_snd->bytes_65_to_127           -= p_port_snd->bytes_65_to_127;
        p_lag_snd->pkts_128_to_255           -= p_port_snd->pkts_128_to_255;
        p_lag_snd->bytes_128_to_255          -= p_port_snd->bytes_128_to_255;
        p_lag_snd->pkts_256_to_511           -= p_port_snd->pkts_256_to_511;
        p_lag_snd->bytes_256_to_511          -= p_port_snd->bytes_256_to_511;
        p_lag_snd->pkts_512_to_1023          -= p_port_snd->pkts_512_to_1023;
        p_lag_snd->bytes_512_to_1023         -= p_port_snd->bytes_512_to_1023;
        p_lag_snd->pkts_1024_to_1518         -= p_port_snd->pkts_1024_to_1518;
        p_lag_snd->bytes_1024_to_1518        -= p_port_snd->bytes_1024_to_1518;
        p_lag_snd->pkts_1519                 -= p_port_snd->pkts_1519;
        p_lag_snd->bytes_1519                -= p_port_snd->bytes_1519;
        p_lag_snd->jumbo_pkts                -= p_port_snd->jumbo_pkts;
        p_lag_snd->jumbo_bytes               -= p_port_snd->jumbo_bytes;
        p_lag_snd->mac_underrun_pkts         -= p_port_snd->mac_underrun_pkts;
        p_lag_snd->mac_underrun_bytes        -= p_port_snd->mac_underrun_bytes;
        p_lag_snd->fcs_error_pkts            -= p_port_snd->fcs_error_pkts;
        p_lag_snd->fcs_error_bytes           -= p_port_snd->fcs_error_bytes;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
_ctc_sai_get_port_stats(_In_ sai_object_id_t port_id,
                        _In_ const sai_port_stat_counter_t *counter_ids,
                        _In_ uint32_t number_of_counters,
                        _Out_ uint64_t* counters)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    ctc_mac_stats_t      mac_tx_stats;
    ctc_mac_stats_t      mac_rx_stats;
    ctc_stats_mac_rec_t  *pmac_rec = NULL;
    ctc_stats_mac_snd_t  *pmac_snd = NULL;
    stbl_port_t          *p_sdb_port = NULL;
    stbl_port_t          *p_sdb_lag = NULL;

    sal_memset(&mac_rx_stats, 0x0, sizeof(ctc_mac_stats_t));
    sal_memset(&mac_tx_stats, 0x0, sizeof(ctc_mac_stats_t));

    mac_tx_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    mac_rx_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    
    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return ret;
    }

    /* 1. if is LAG member, should delete LAG member count to LAG group */
    if (p_sdb_port->lag_id)
    {
        p_sdb_lag = ctc_sai_port_entry_get_by_portid(p_sdb_port->lag_id);
        if (p_sdb_lag)
        {
            _ctc_sai_update_lag_stats(p_sdb_lag->rx_stats_ptr.p_stats, p_sdb_lag->tx_stats_ptr.p_stats, p_sdb_port->rx_stats_ptr.p_stats, p_sdb_port->tx_stats_ptr.p_stats, FALSE);
        }
    }

    /* 2. get stats from SDK */
    if(!ctc_stats_get_mac_stats(
        CTC_SAI_OBJECT_INDEX_GET(port_id),
        CTC_STATS_MAC_STATS_RX,
        &mac_rx_stats))
    {
        pmac_rec = &mac_rx_stats.u.stats_detail.stats.rx_stats;        
    }
    else
    {
        return ret;
    }
    if(!ctc_stats_get_mac_stats(
        CTC_SAI_OBJECT_INDEX_GET(port_id),
        CTC_STATS_MAC_STATS_TX,
        &mac_tx_stats))
    {
        pmac_snd = &mac_tx_stats.u.stats_detail.stats.tx_stats;
    }
    else
    {
        return ret;
    }

    /* 3. Store to SAI port DB */
    sal_memcpy(p_sdb_port->rx_stats_ptr.p_stats, pmac_rec, sizeof(ctc_stats_mac_rec_t));
    sal_memcpy(p_sdb_port->tx_stats_ptr.p_stats, pmac_snd, sizeof(ctc_stats_mac_snd_t));
    
    /* 4. if is LAG member, should add LAG member count to LAG group */
    if (p_sdb_port->lag_id)
    {
        p_sdb_lag = ctc_sai_port_entry_get_by_portid(p_sdb_port->lag_id);
        if (p_sdb_lag)
        {
            _ctc_sai_update_lag_stats(p_sdb_lag->rx_stats_ptr.p_stats, p_sdb_lag->tx_stats_ptr.p_stats, p_sdb_port->rx_stats_ptr.p_stats, p_sdb_port->tx_stats_ptr.p_stats, TRUE);
        }
    }

    /* 5. if need to SAI, map data */
    if (counters)
    {
        ctc_sai_map_ctc_to_sai(number_of_counters, counter_ids, counters, pmac_rec, pmac_snd);
    }

    return ret;
}

sai_status_t
_ctc_sai_get_lag_stats(_In_ sai_object_id_t port_id,
                        _In_ const sai_port_stat_counter_t *counter_ids,
                        _In_ uint32_t number_of_counters,
                        _Out_ uint64_t* counters)
{

    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    stbl_port_t *p_sai_lag = NULL;
    
    p_sai_lag = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sai_lag)
    {
        return ret;
    }
    
    ctc_sai_map_ctc_to_sai(number_of_counters, counter_ids, counters, p_sai_lag->rx_stats_ptr.p_stats, p_sai_lag->tx_stats_ptr.p_stats);

    return ret;
}

/*
* Routine Description:
*   Get port statistics counters.
*
* Arguments:
*    [in] port_id - port id
*    [in] counter_ids - specifies the array of counter ids
*    [in] number_of_counters - number of counters in the array
*    [out] counters - array of resulting counter values.
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_port_stats(_In_ sai_object_id_t port_id,
                        _In_ const sai_port_stat_counter_t *counter_ids,
                        _In_ uint32_t number_of_counters,
                        _Out_ uint64_t* counters)
{
    if (SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return _ctc_sai_get_port_stats(port_id, counter_ids, number_of_counters, counters);
    }
    else if (SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        return _ctc_sai_get_lag_stats(port_id, counter_ids, number_of_counters, counters);
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
}

sai_status_t
ctc_sai_port_bind_lag_id(
    _In_  sai_object_id_t   port_id,
    _In_  sai_object_id_t   lag_id)
{
    stbl_port_t *p_sdb_port = NULL;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    p_sdb_port->lag_id = lag_id;

    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_port_get_lag_id(
    _In_  uint32 gport)
{
    stbl_port_t *p_sdb_port = NULL;
    uint32 lagid = 0;

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        lagid = CTC_GPORT_LINKAGG_ID(gport);
    }
    else
    {
        p_sdb_port = ctc_sai_port_entry_get_by_gport(gport);
        if (NULL == p_sdb_port)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        if (0 != p_sdb_port->lag_id)
        {
            lagid = CTC_SAI_OBJECT_INDEX_GET(p_sdb_port->lag_id);
        }
    }

    return lagid;
}

stbl_port_t*
ctc_sai_port_get_port(
    _In_  sai_object_id_t port_id)
{
    stbl_port_t *p_sdb_port = NULL;

    if(SAI_OBJECT_TYPE_PORT == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    }
    else if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(port_id))
    {
        p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    }
    
    return p_sdb_port;
}

uint32
ctc_sai_port_make_acl_group_id(uint32 dir, uint32 gport)
{
    stbl_port_t *p_sdb_port = NULL;
    uint16 group_id = 0;

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        group_id = CTC_GPORT_LINKAGG_ID(gport);
        return (dir == CTC_INGRESS) ? (group_id) : (group_id + CTC_SAI_PORT_GROUP_LAG_NUM);
    }

    p_sdb_port = ctc_sai_port_entry_get_by_gport(gport);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return (dir == CTC_INGRESS) ? (p_sdb_port->group_id_igs) : (p_sdb_port->group_id_egs);
}

uint32
ctc_sai_port_get_ttp_acl_group_id(uint32 dir, uint32 gport)
{
    stbl_port_t *p_sdb_port = NULL;

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    p_sdb_port = ctc_sai_port_entry_get_by_gport(gport);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return (dir == CTC_INGRESS) ? (p_sdb_port->ttp_group_id_igs) : SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t
ctc_sai_port_set_default(
	_In_  sai_object_id_t 	port_id)
{
    uint32_t                    idx = 0;
    sai_attribute_t             attr;
    uint32_t                    gport = 0;
    ctc_sai_attr_entry_list_t*   pport_default_attr;
    uint16                  queue_index = 0;
    ctc_qos_queue_cfg_t     que_cfg;
    stbl_port_t    *p_sdb_port = NULL;
    stbl_port_glb_t *p_sdb_port_glb = stbl_port_glb_get_port_glb();

    sal_memset(&que_cfg, 0x0, sizeof(ctc_qos_queue_cfg_t));

    p_sdb_port = ctc_sai_port_entry_db_alloc(port_id);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries, NULL, 0, &pport_default_attr);

    while(SAI_ATTR_ID_END != g_sai_attr_entries[idx].id)
    {
        if(true == pport_default_attr[idx].valid)
        {
            attr.id         = g_sai_attr_entries[idx].id;
            attr.value      = pport_default_attr[idx].value;
            ctc_sai_set_port_attribute(port_id, &attr);
        }

        idx++;
    }

    ctc_sai_port_objectid_to_gport(port_id, &gport);

    if (0 != ctclib_opb_alloc_offset(&p_sdb_port_glb->group_id_igs_opb, &(p_sdb_port->group_id_igs)))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    if (0 != ctclib_opb_alloc_offset(&p_sdb_port_glb->group_id_egs_opb, &(p_sdb_port->group_id_egs)))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    if (0 != ctclib_opb_alloc_offset(&p_sdb_port_glb->ttp_group_id_igs_opb, &(p_sdb_port->ttp_group_id_igs)))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    if (0 != ctclib_opb_alloc_offset(&p_sdb_port_glb->port_class_id_opb, &(p_sdb_port->port_class_id)))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    ctc_sai_acl_port_enable(CTC_INGRESS, port_id);
    ctc_sai_acl_port_enable(CTC_EGRESS,  port_id);

    ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, CTC_PORT_ARP_ACTION_TYPE_FW_EX);
    ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_PORT_ARP_ACTION_TYPE_FW_EX);
    ctc_sai_port_set_port_flag(port_id,SAI_PORT_FLAG_ALLOW_UNTAGGED, TRUE);
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU);
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_SLOW_PROTO, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU);
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_ERPS, CTC_PDU_L2PDU_ACTION_TYPE_FWD);
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_LLDP, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU);
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_EAPOL, CTC_PDU_L2PDU_ACTION_TYPE_DISCARD);
    /*added by yejl for bug 47118, 2018-12-20*/
    ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_PAUSE_DMAC, CTC_PDU_L2PDU_ACTION_TYPE_DISCARD);

    ctc_port_set_receive_en(gport, FALSE);
    ctc_port_set_transmit_en(gport, FALSE);

    /*modified by hansf,port default trust cos,port default replace cos 2017/09/13*/
    ctc_port_set_property(gport, CTC_PORT_PROP_QOS_POLICY,CTC_QOS_TRUST_STAG_COS);
    ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, 0);

    /*added by hansf for tunnel inner hash*/
#if defined(GOLDENGATE) || defined(USW)
    ctc_port_set_property(gport, CTC_PORT_PROP_SNOOPING_PARSER, TRUE);
    ctc_port_set_property(gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, TRUE);
#endif
    /*ended by hansf*/
    
    for(queue_index = 0; queue_index < 8; queue_index ++)
    {
        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
        que_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;

        que_cfg.value.stats.stats_en = 1;
        que_cfg.value.stats.queue.gport = gport;
        que_cfg.value.stats.queue.queue_id = queue_index;
        ctc_qos_set_queue(&que_cfg);
    }

    if (pport_default_attr)
    {
        ctc_sai_attr_free_attr_entry_list(pport_default_attr);
        pport_default_attr = NULL;
    }

/* modified by liwh for bug 40004, 2016-08-04 */
/* bug 53663 TM set DsChannelizeMode.portMaxLen */
#if defined(GREATBELT) || defined(TSINGMA)
    ctc_port_set_max_frame(gport, GLB_DFT_MAX_FRAME_SIZE_0);
#endif
/* liwh end */

#ifdef GREATBELT
    ctc_port_set_property(gport, CTC_PORT_PROP_LOGIC_PORT, gport);
#else
    /* Added by kcao for bug 47646 2018-06-14, gport on slice1 is 0x80xx > 0x3FFF, cannot set into DsPhyPortExt.defaultLogicSrcPort */
    ctc_port_set_property(gport, CTC_PORT_PROP_LOGIC_PORT, CTC_MAP_GPORT_TO_LPORT(gport));
#endif

#ifdef OFPRODUCT
    ctc_port_set_property(gport, CTC_PORT_PROP_PTP_EN, 1);

#ifdef GREATBELT
    ctc_port_set_property(gport, CTC_PORT_PROP_MAC_TS_EN, 1);
#endif
#endif
  
    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_port_alloc_timestamp (sai_object_id_t port_id, uint32_t *ptimestamp_nhid)
{
    stbl_port_t             *p_sdb_port = NULL;
    ctc_misc_nh_param_t     nh_param;
    uint32_t                nexthopid    = 0;
    sai_status_t            ret 		 = SAI_STATUS_SUCCESS;
    
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    nh_param.type = CTC_MISC_NH_TYPE_OVER_L2_WITH_TS;
    sal_memcpy(nh_param.misc_param.over_l2edit.mac_da,
        ctc_sai_get_sys_info()->timestamp.dmac, sizeof(sai_mac_t));
    sal_memcpy(nh_param.misc_param.over_l2edit.mac_sa,
        ctc_sai_get_sys_info()->timestamp.smac, sizeof(sai_mac_t));
    nh_param.misc_param.over_l2edit.ether_type = ctc_sai_get_sys_info()->timestamp.type;

CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64",timestamp_count = %d,"
              "dmac=%02x:%02x:%02x:%02x:%02x:%02x,"
              "dmac=%02x:%02x:%02x:%02x:%02x:%02x\n",
            __FUNCTION__, __LINE__, port_id, p_sdb_port->timestamp_count,
            nh_param.misc_param.over_l2edit.mac_da[0],
            nh_param.misc_param.over_l2edit.mac_da[1],
            nh_param.misc_param.over_l2edit.mac_da[2],
            nh_param.misc_param.over_l2edit.mac_da[3],
            nh_param.misc_param.over_l2edit.mac_da[4],
            nh_param.misc_param.over_l2edit.mac_da[5],
            nh_param.misc_param.over_l2edit.mac_sa[0],
            nh_param.misc_param.over_l2edit.mac_sa[1],
            nh_param.misc_param.over_l2edit.mac_sa[2],
            nh_param.misc_param.over_l2edit.mac_sa[3],
            nh_param.misc_param.over_l2edit.mac_sa[4],
            nh_param.misc_param.over_l2edit.mac_sa[5]);

    if (p_sdb_port->timestamp_count == 0)
    {
        nh_param.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthopid),ret,err);
        CTC_SAI_ERROR_GOTO(ctc_nh_add_misc(nexthopid, &nh_param),ret,err1);

        p_sdb_port->timestamp_nexthopid = nexthopid;

        CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64",timestamp_nhid = %d\n",
            __FUNCTION__, __LINE__, port_id, p_sdb_port->timestamp_nexthopid);
    }

    p_sdb_port->timestamp_count++;
    *ptimestamp_nhid = p_sdb_port->timestamp_nexthopid;

    CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64",timestamp_count = %d\n",
            __FUNCTION__, __LINE__, port_id, p_sdb_port->timestamp_count);
    
    return SAI_STATUS_SUCCESS;
err1:
    ctc_sai_nexthop_free_offset(nexthopid);
err:
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64"\n", __FUNCTION__, __LINE__, port_id);
    return ret;
}

int32_t
ctc_sai_port_free_timestamp (sai_object_id_t port_id)
{
    stbl_port_t             *p_sdb_port = NULL;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_sdb_port->timestamp_count--;

    CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64",timestamp_count = %d\n",
            __FUNCTION__, __LINE__, port_id, p_sdb_port->timestamp_count);
    
    if (p_sdb_port->timestamp_count <= 0 ){
        CTC_SAI_DEBUG("%s%d:port_oid = 0x%"PRIx64",timestamp_nexthopid = %d\n",
            __FUNCTION__, __LINE__, port_id, p_sdb_port->timestamp_nexthopid);
        
        ctc_nh_remove_misc(p_sdb_port->timestamp_nexthopid);
        ctc_sai_nexthop_free_offset(p_sdb_port->timestamp_nexthopid);
    }
    
    return SAI_STATUS_SUCCESS;
}

int32_t
ctc_sai_port_get_metadata(sai_object_id_t port_id)
{
    stbl_port_t             *p_sdb_port = NULL;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_id);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return p_sdb_port->metadata_en;
}

#define ________SAI_PORT_INNER_FUNC

sai_status_t
__port_init_set_default_cb(
	_In_  sai_object_id_t   port_id,
	_In_  void              *pdata)
{
    return ctc_sai_port_set_default(port_id);
}

stbl_port_t*
ctc_sai_port_entry_get_by_gport(uint32 gport)
{
    stbl_port_key_t sdb_port_key;
    sai_object_id_t port_oid;

    ctc_sai_port_gport_to_objectid(gport, &port_oid);
    sdb_port_key.port_oid = port_oid;
    return stbl_port_get_port(&sdb_port_key);
}

stbl_port_t*
ctc_sai_port_entry_get_by_portid(sai_object_id_t port_oid)
{
    stbl_port_key_t sdb_port_key;
    
    sdb_port_key.port_oid = port_oid;
    return stbl_port_get_port(&sdb_port_key);
}

stbl_port_t*
ctc_sai_port_entry_db_alloc(sai_object_id_t port_oid)
{
    stbl_port_t sdb_port;
    stbl_port_t *p_sdb_port = NULL;
    uint32 gport = 0;
    
    sal_memset(&sdb_port, 0, sizeof(sdb_port));
    ctc_sai_port_objectid_to_gport(port_oid, &gport);
    sdb_port.key.port_oid = port_oid;
    sdb_port.gport = gport;
    stbl_port_add_port(&sdb_port);
    p_sdb_port = stbl_port_get_port(&sdb_port.key);

    p_sdb_port->rx_stats_ptr.p_stats = XCALLOC(MEM_STBL_PORT_RX_STATS, sizeof(ctc_stats_mac_rec_t));
    p_sdb_port->tx_stats_ptr.p_stats = XCALLOC(MEM_STBL_PORT_TX_STATS, sizeof(ctc_stats_mac_snd_t));
    
    return p_sdb_port;
}

int32
ctc_sai_port_entry_db_free(sai_object_id_t port_oid)
{
    stbl_port_key_t sdb_port_key;
    stbl_port_t *p_sdb_port = NULL;
    
    sal_memset(&sdb_port_key, 0, sizeof(sdb_port_key));
    sdb_port_key.port_oid = port_oid;
    p_sdb_port = stbl_port_get_port(&sdb_port_key);
    if (NULL == p_sdb_port)
    {
        return 0;
    }

    if (p_sdb_port->rx_stats_ptr.p_stats)
    {
        XFREE(MEM_STBL_PORT_RX_STATS, p_sdb_port->rx_stats_ptr.p_stats);
    }
    if (p_sdb_port->tx_stats_ptr.p_stats)
    {
        XFREE(MEM_STBL_PORT_TX_STATS, p_sdb_port->tx_stats_ptr.p_stats);
    }
    stbl_port_del_port(&sdb_port_key);
    
    return 0;
}

sai_status_t
ctc_sai_port_set_all_default()
{
    stbl_port_glb_t *p_sdb_port_glb = stbl_port_glb_get_port_glb();

    if (0 != ctclib_opb_create(&p_sdb_port_glb->group_id_igs_opb, CTC_SAI_PORT_GROUP_IGS_ID_BASE, CTC_SAI_PORT_GROUP_IGS_ID_MAX, "port_igs_group"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    if (0 != ctclib_opb_create(&p_sdb_port_glb->group_id_egs_opb, CTC_SAI_PORT_GROUP_EGS_ID_BASE, CTC_SAI_PORT_GROUP_EGS_ID_MAX, "port_egs_group"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    if (0 != ctclib_opb_create(&p_sdb_port_glb->ttp_group_id_igs_opb, CTC_SAI_PORT_TTP_GROUP_IGS_ID_BASE, CTC_SAI_PORT_TTP_GROUP_IGS_ID_MAX, "port_igs_ttp_group"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    if (0 != ctclib_opb_create(&p_sdb_port_glb->port_class_id_opb, CTC_SAI_PORT_CLASS_ID_BASE, CTC_SAI_PORT_CLASS_ID_MAX, "port_class_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    ctc_sai_for_each_port_object(__port_init_set_default_cb, NULL);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
__check_port_mac_link(
    _In_ sai_object_id_t portid,
    _In_ void *port_info)
{
    uint32_t              port_statc     = false;
    sai_object_list_t    port_list;
    stbl_port_t *p_sdb_port = NULL;
    sai_port_oper_status_t  status = SAI_PORT_OPER_STATUS_UNKNOWN;
    sai_port_oper_status_notification_t noti_data;

    p_sdb_port = ctc_sai_port_entry_get_by_portid(portid);
    if (NULL == p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
#ifdef _GLB_UML_SYSTEM_
    if(!ctc_port_get_mac_en(
        CTC_SAI_OBJECT_INDEX_GET(portid),
        (bool*)&port_statc))
#else
    if(!ctc_port_get_mac_link_up(
        CTC_SAI_OBJECT_INDEX_GET(portid),
        (bool*)&port_statc))
#endif
    {
        if(port_statc)
        {
            status = SAI_PORT_OPER_STATUS_UP;

            if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(p_sdb_port->lag_id))
            {
                sal_memset(&port_list,0,sizeof(sai_object_list_t));
                port_list.count = 1;
                port_list.list  = &portid;
                //ctc_sai_lag_add_ports(psaiport_entry->lag_id,&port_list);
            }
        }
        else
        {
            status = SAI_PORT_OPER_STATUS_DOWN;
        }
        if (p_sdb_port->port_state != status)
        {
            p_sdb_port->port_state = status;
            if(ctc_sai_get_sys_info()->sai_switch_notification_table.on_port_state_change)
            {
                noti_data.port_id = portid;
                noti_data.port_state = p_sdb_port->port_state;
                ctc_sai_get_sys_info()->sai_switch_notification_table.on_port_state_change(
                    1, &noti_data);
            }
        }
    }

    return 0;
}

void
__port_mac_link_state_thread(void* user_param)
{
    while (1)
    {
        ctc_sai_for_each_port_object(__check_port_mac_link, NULL);
        sal_task_sleep(1000);
    }

    return;
}

int32_t
ctc_port_lag_notifier_callback(
    _In_  struct ctc_sai_notifier_block *nb,
    _In_  uint32_t action,
    _In_  void *data)
{
    sai_attribute_t         attr               = {0};
    stbl_port_t             *p_sdb_lag = NULL;
    ctc_sai_lag_notifier_t  *plag_nf        = data;

    p_sdb_lag = ctc_sai_port_entry_get_by_portid(plag_nf->lag_id);
    if (NULL == p_sdb_lag)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

#ifdef TAPPRODUCT
    attr.id = SAI_PORT_ATTR_CUSTOM_METADATA;
    attr.value.u32 = action == CTC_SAI_LAG_NF_NEW_LAG_MEMBER ? 
        p_sdb_lag->metadata_en : 0;
    ctc_sai_port_set_metadata(plag_nf->lag_member_port_id, &attr);
#if defined(GOLDENGATE) || defined(USW)
    attr.id = SAI_PORT_ATTR_EGRESS_ACL_EN;
    attr.value.u32 = action == CTC_SAI_LAG_NF_NEW_LAG_MEMBER ? 
        p_sdb_lag->egress_acl_en : 0;
    ctc_sai_port_set_egress_port_acl(plag_nf->lag_member_port_id, &attr);
#endif
#endif
#ifndef TAPPRODUCT
    attr.id = SAI_PORT_ATTR_CUSTOM_RELFECTIVE_EN;
    attr.value.u32 = action == CTC_SAI_LAG_NF_NEW_LAG_MEMBER ? 
        p_sdb_lag->reflective_bridge_en: 0;
    ctc_sai_port_set_reflective(plag_nf->lag_member_port_id, &attr);

	attr.id = SAI_PORT_ATTR_DHCPRELAY_EXCEPTION_TYPE;
    attr.value.u32 = action == CTC_SAI_LAG_NF_NEW_LAG_MEMBER ? 
        p_sdb_lag->dhcprelay_exception: 0;
    ctc_sai_port_set_dhcp_relay_exception_type(plag_nf->lag_member_port_id, &attr);
#endif    
    return 0;
}

static struct ctc_sai_notifier_block nb_lag_notifier_cb = {
    .notifier_call = ctc_port_lag_notifier_callback,
};

sai_status_t
__port_init_mode_fn(
	_In_  ctc_sai_api_reg_info_t	*preg,
	_In_  void						*private_data)
{
    ctc_sai_lag_evnet_notifier_register(&nb_lag_notifier_cb);

    sdb_mac_rx_stats_ptr_val2str_register_fn(sai_sdb_mac_rx_stats_ptr_val2str);
    sdb_mac_tx_stats_ptr_val2str_register_fn(sai_sdb_mac_tx_stats_ptr_val2str);
    preg->init_status = INITIALIZED;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
__port_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t
ctc_sai_attr_check_speed(ctc_sai_attr_entry_list_t *pattr, void *data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    switch(pattr->value.u32)
    {
    case CTC_SAI_PORT_SPEED_10M:
    case CTC_SAI_PORT_SPEED_100M:
    case CTC_SAI_PORT_SPEED_1000M:
    case CTC_SAI_PORT_SPEED_2G5:
    case CTC_SAI_PORT_SPEED_10G:
    case CTC_SAI_PORT_SPEED_40G:
    case CTC_SAI_PORT_SPEED_100G:
        ret = SAI_STATUS_SUCCESS;
        break;
    default:
        ret = SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr->attr_index;
        break;
    }

    return ret;
}

/* define sai 0.9.3 */
static sai_port_api_t      g_sai_port_api_func = {
    .set_port_attribute		= ctc_sai_set_port_attribute,
    .get_port_attribute		= ctc_sai_get_port_attribute,
    .get_port_stats			= ctc_sai_get_port_stats,
    .clear_port_stats       = ctc_sai_clear_port_stats,
    .clear_port_all_stats   = ctc_sai_clear_port_all_stats,
    
};

static ctc_sai_api_reg_info_t g_port_api_reg_info = {
        .id  		= SAI_API_PORT,
        .init_func 	= __port_init_mode_fn,
        .exit_func 	= __port_exit_mode_fn,
        .api_method_table = &g_sai_port_api_func,
        .private_data     = NULL,
};

#define ________SAI_PORT_OUTER_FUNC
sai_status_t
ctc_sai_port_init()
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    
    api_reg_register_fn(&g_port_api_reg_info);
    return ret;
}


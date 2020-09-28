
#ifndef __QOS_MGR_H__
#define __QOS_MGR_H__

typedef struct
{
    char *name;
    uint8 priority;
    uint8 color;
    uint8 qid;
    uint8 class;
    uint8 weight;
}qos_phb_prop_t;
int32 
qos_mgr_get_default_queue_class(int32 qid);

int32 
qos_mgr_get_default_queue_weight(int32 qid);

int32
qos_mgr_cmd_process_domain(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_mpls_domain(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_policer_res(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_policer_action_profile(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_policer_profile(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_queue_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_port_shape_profile(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_drop_profile(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_queue_smart_buffer(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_show_queue_drop(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para);

glb_qos_color_t
qos_mgr_map_str2color(char* str);

glb_qos_phb_type_t
qos_mgr_map_str2phb(char* str);

int32
qos_mgr_cmd_get_port_queue_shape(char *ifname, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_get_port_queue_drop(char *ifname, cfg_cmd_para_t *para);

int32
qos_mgr_cmd_get_port_shape(char *ifname, cfg_cmd_para_t *para);

int32
qos_mgr_set_qos_enable(bool enable);

int32
qos_mgr_port_queue_update_handle(int32 op_type, int32 ifindex, int32 qid, int32 dir, int32 type);

int32
qos_mgr_init_default_drop(char* name);

int32
qos_mgr_set_queue_class(int32 ifindex, int32 qid, int32 class);

int32
qos_mgr_set_queue_weight(int32 ifindex, int32 qid, int32 weight);

int32
qos_mgr_set_port_default_cos(int32 ifindex, int32 cos);

int32
qos_mgr_set_port_replace(int32 ifindex, int32 replace);

int32
qos_mgr_set_port_trust(int32 ifindex, int32 trust);

int32
qos_mgr_set_port_shape(int32 ifindex, bool enable, int32 rate);

int32
qos_mgr_set_queue_drop_mode(int32 ifindex, int32 qid, int32 mode);

int32
qos_mgr_set_queue_ecn_enable(int32 ifindex, int32 qid, int32 enable);

int32
qos_mgr_set_glb_monitor_latency_range(tbl_qos_global_t *p_glb,  uint32 latency_range[GLB_MONITOR_LATENCY_RANGE_ID_MAX]);

/* start */
int32
qos_mgr_start(switch_master_t *master);

int32
qos_mgr_stop(switch_master_t *master);

int32
cpu_traffic_mgr_start(switch_master_t *master);

int32
qos_mgr_map_policer_by_name(char *name, glb_qos_policer_t* p_policer);

int32
qos_mgr_set_glb_buffer_event_enable(tbl_qos_global_t  *p_glb, uint32 enable);

int32
qos_mgr_set_glb_buffer_event_threshold(tbl_qos_global_t *p_glb, uint32 low_thrd , uint32 high_thrd);

int32
qos_monitor_init(void);

int32
qos_mgr_policer_res_check(char *policer_name, bool direct_in, bool port_policer, uint32 *count);

int32
qos_mgr_policer_res_add(char *policer_name, bool direct_in, bool port_policer);

int32
qos_mgr_policer_res_del(char *policer_name, bool direct_in, bool port_policer);

int32
qos_mgr_policer_res_update_op(tbl_qos_policer_profile_t *policer_new, tbl_qos_policer_profile_t *policer_old, uint32 op, bool *flow_resource_full);

#endif /* !__QOS_MGR_H__ */

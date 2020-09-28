
#ifndef __IF_MGR_H__
#define __IF_MGR_H__
#include "switch.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"

#define IF_VLAN_DEFAULT_BW     1000000  /* kilobits */
#define IF_TUNNEL_DEFAULT_BW   9        /* kilobits */
#define IF_LO_DEFAULT_BW       1000000  /* kilobits */

#define IF_PFC_PRIORITY_DEFAULT  3  /* default pfc priority */

#define IF_DEFAULT_IPG_SIZE    160  /* ipg size in bits, 20 bytes */

struct if_stats_rate_master_info_s
{
    ctclib_hash_t *if_stats_hash;              /* -> if_port_stats_rate_db */
};
typedef struct if_stats_rate_master_info_s if_stats_rate_master_info_t;


struct if_port_rate_info_s 
{
    tbl_port_stats_rate_key_t key; 
    uint8 index;
    u_int64_t octets_rcv[GLB_IF_STATS_STORE_CNT];
    u_int64_t pkts_rcv[GLB_IF_STATS_STORE_CNT];
    u_int64_t octets_send[GLB_IF_STATS_STORE_CNT];
    u_int64_t pkts_send[GLB_IF_STATS_STORE_CNT];
    sal_time_t start_time;  /* port stats current time */
    sal_time_t store_time;  /* port stats last store time */
    uint64 store_poll_cnt; /* port stats save per min times cnt*/
};
typedef struct if_port_rate_info_s if_port_rate_info_t;

enum log_threshold_state_e
{
    LOG_THRESHOLD_DISABLE = 0,
    LOG_THRESHOLD_ENABLE,
    LOG_THRESHOLD_OVER,
    LOG_THRESHOLD_RESUME,
    LOG_THRESHOLD_STATE_MAX
};
typedef enum log_threshold_state_e log_threshold_state_t;


#define IF_GET_INDEX_BY_NAME(IFINDEX, IFNAME) \
do { \
    rc = tbl_interface_get_ifindex_by_name((IFNAME)); \
    if (rc < 0) \
    { \
        return PM_E_INVALID_IFNAME; \
    } \
    (IFINDEX) = rc; \
} while(0)

int32
if_mgr_set_kernel_mac_addr(tbl_interface_t *p_db_if);

int32
if_mgr_set_desc(tbl_interface_t *p_if);

int32
if_mgr_set_svlan_tpid(tbl_interface_t *p_if);

int32
if_mgr_set_speed(tbl_interface_t *p_if);

int32
if_mgr_set_default_mtu(tbl_interface_t *p_db_if);

int32
if_mgr_set_duplex(tbl_interface_t *p_if);

int32
if_mgr_set_loopback(tbl_interface_t *p_if);

int32
if_mgr_set_metadata(tbl_interface_t *p_if);

int32
if_mgr_set_media(tbl_interface_t *p_if);

int32
if_mgr_set_unidir(tbl_interface_t *p_if);

int32
if_mgr_set_fec(tbl_interface_t *p_if);

int32
if_mgr_set_poe(tbl_interface_t *p_if);

int32
if_mgr_set_reflective(tbl_interface_t *p_if);

int32
if_mgr_set_flowcontrol_send(tbl_interface_t *p_if);

int32
if_mgr_set_flowcontrol_receive(tbl_interface_t *p_if);

int32
if_mgr_set_pfc_en(tbl_interface_t *p_if);

int32
if_mgr_set_pfc_priority(tbl_interface_t *p_db_if);

int32
if_mgr_set_jumboframe_en(tbl_interface_t *p_if, uint32 jumboframe_en, uint32 iter_member);

int32
if_mgr_set_isolate_group_id(tbl_interface_t *p_if, uint32 isolate_group_id, uint32 iter_member);

int32
if_mgr_set_pvlan_mode(tbl_interface_t *p_if, uint32 pvlan_type, uint32 iter_member);

int32
if_mgr_set_pvlan_primary_vlan(tbl_interface_t *p_if, vid_t primary_vlan);

int32
if_mgr_set_pvlan_secodary_vlan(tbl_interface_t *p_if, uint32 pvlan_type, vid_t primary_vlan, vid_t comunity_vlan);

int32
if_mgr_set_pvlan(tbl_interface_t *p_if, uint32 pvlan_type, vid_t primary_vlan, vid_t community_vlan, uint32 iter_member);

int32
if_mgr_init_pvlan_handle(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

int32    
if_mgr_init_trunk_untag_handle(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);


int32
if_mgr_mapping_community_vlan_to_group(tbl_interface_t *p_if, vid_t community_vlan);

int32
if_mgr_unmapping_community_vlan_to_group(tbl_interface_t *p_db_if, vid_t community_vlan);

int32
if_mgr_set_mode(tbl_interface_t *p_if, uint32 mode, uint32 iter_member);

int32
if_mgr_set_vlan_type(tbl_interface_t *p_if, uint32 vlan_type, uint32 iter_member);

int32
if_mgr_set_up(tbl_interface_t* p_if, uint32 enable, uint32 iter_member, uint32 admin);

int32
if_up_sync_when_process(tbl_interface_t* p_if, uint32 enable);

int32
if_mgr_set_bandwidth(tbl_interface_t *p_if);

int32
if_mgr_set_mtu(tbl_interface_t *p_db_if, uint32 mtu, uint32 iter_member);

int32
if_mgr_set_tap_used(tbl_interface_t *p_db_if, uint32 used, uint32 iter_member);

int32
if_mgr_add_port_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if, cfg_cmd_para_t *para);

int32
if_mgr_add_vlan_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_add_loopback_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_add_cpu_if();

int32
if_mgr_add_lag_if(char *ifname, tbl_interface_t *p_db_if_mem, ds_lag_t *p_lag);

int32
if_mgr_del_lag_if(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem);

int32
if_mgr_del_port_if(tbl_interface_t *p_db_if);

int32
if_mgr_del_vlan_if(tbl_interface_t *p_db_if);

int32
if_mgr_add_l2gre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_add_l3gre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_add_nvgre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_add_vxlan_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if);

int32
if_mgr_del_if(char *ifname, cfg_cmd_para_t *para);

int32
if_mgr_add_if_to_lag(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, ds_lag_t *p_lag);

int32
if_mgr_del_if_from_lag(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem);

int32
if_mgr_set_if_config_up();

int32
if_mgr_get_default_bandwidth(tbl_interface_t *p_if, uint32 *p_bw);

int32
if_mgr_lag_if_running_update(tbl_interface_t *p_db_if_lag);

int32
if_mgr_vlan_if_running_update(tbl_vlan_t *p_db_vlan);

int32
if_mgr_start(switch_master_t *master);

int32
if_mgr_stop(switch_master_t *master);

int32
if_mgr_cmd_process_interface(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_process_aclqos_if(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_process_interface_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_show_bandwidth_usage(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
_if_mgr_cmd_set_input_rate_threshold(tbl_port_stats_rate_t *p_db_stats_rate, 
    uint8 threshold, uint8 resumerate, cfg_cmd_para_t *para);

int32
_if_mgr_cmd_set_output_rate_threshold(tbl_port_stats_rate_t *p_db_stats_rate, 
    uint8 threshold, uint8 resumerate, cfg_cmd_para_t *para);

int32
_if_mgr_cmd_set_output_discard_threshold(tbl_port_stats_rate_t *p_db_stats_rate, 
    uint32 threshold, uint32 interval, cfg_cmd_para_t *para);

int32
_if_mgr_cmd_set_load_interval(tbl_port_stats_rate_t *p_db_stats_rate, uint16 interval, cfg_cmd_para_t *para);

int32
if_mgr_get_vlan_if_count();

int32
if_mgr_get_loopback_count();

uint32
if_mgr_get_max_frame_size(tbl_interface_t *p_db_if);

int32
if_mgr_get_if_stats(tbl_interface_t *p_if, tbl_iter_args_t *pargs);

int32
if_mgr_clear_if_stats(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

int32
if_mgr_set_storm_control(tbl_interface_t *p_db_if, uint32 rate,
                                            uint32 type, uint32 mode);

int32
if_mgr_update_storm_control_level(tbl_interface_t *p_db_if, ds_storm_control_t *p_storm_ctl);

uint32
if_mgr_get_link_speed(uint8 speed);

#ifdef CONFIG_VLANCLASS
int32
if_mgr_set_vlanclass_add_group_to_if(tbl_interface_t *p_if, uint32 vclass_group_id, uint32 vclass_active_type);

int32
if_mgr_set_vlanclass_del_group_from_if(tbl_interface_t *p_if);

int32
if_mgr_set_vlanclass_default_action(tbl_interface_t *p_if, uint32 default_action);
#endif /* CONFIG_VLANCLASS */

#ifdef CONFIG_SFLOW
int32
if_mgr_set_sflow_counter_enable(tbl_interface_t *p_db_if, uint32 sflow_counter_en);

int32
if_mgr_set_sflow_flow_enable(tbl_interface_t *p_db_if, uint32 flow_en, uint32 flow_ingress, uint32 flow_egress);

int32
if_mgr_sflow_if_map_rate (uint32 from, uint8 *out);

int32
if_mgr_set_sflow_rate(tbl_interface_t *p_db_if, uint32 weight);

int32
if_mgr_sflow_pkt_tx(uint8 *p_pkt, uint32 pkt_len);
#endif /* CONFIG_SFLOW */

int32
if_mgr_set_all_if_down();

int32
if_mgr_set_opf_intf_pkt_default(tbl_openflow_interface_t *p_opf_intf);

int32
if_mgr_packet_type(char *pkt_type);

int32
if_mgr_packet_action(char *pkt_act);

int32
if_mgr_set_openflow_enable(tbl_interface_t *p_db_if);

int32
if_mgr_set_openflow_disable(tbl_interface_t *p_db_if);

int32
if_mgr_isolate_group_process(tbl_interface_t *p_db_if, uint32 new_group_id);

int32
if_mgr_check_packet_tx_condition(tbl_interface_t *p_db_if, glb_pkt_type_t type);

int32
if_mgr_cmd_process_isolation_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_process_pvlan_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_process_pfc_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_cmd_process_isolation(char **argv, int32 argc, cfg_cmd_para_t *para);

int32 
if_mgr_cmd_process_openflow_interface(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_set_arpinsp_trust_enable(tbl_interface_t *p_if, uint32 trust_enable, uint32 iter_member);

int32 
if_mgr_set_arp_numberlimit_enable(tbl_interface_t *p_if, uint32 numberlimit_enable);

int32 
if_mgr_set_arp_numberlimit_number(tbl_interface_t *p_if, uint32 numberlimit_number);

int32 
if_mgr_set_arp_numberlimit_violate(tbl_interface_t *p_if, uint32 numberlimit_violate);

int32
if_mgr_set_latency_event_enable(tbl_interface_t *p_if, uint32 enable);  

int32
if_mgr_set_latency_event_threshold(tbl_interface_t *p_if, uint32 low_thrd, uint32 high_thrd);

int32
if_mgr_set_latency_event_threshold_default(tbl_interface_t *p_if);

int32
if_mgr_set_dot1q_ethertype(tbl_interface_t *p_db_if, uint32 tpid_value, 
        cfg_cmd_para_t *para, uint32 iter_member);

int32
if_mgr_cmd_process_openflow_tunnel_interface(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_mgr_set_l2protocol_uplink(tbl_interface_t *p_if);

int32
if_mgr_set_l2protocol_vlan(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if);

int32
if_mgr_set_l2protocol_action(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if);

int32
if_mgr_set_l2protocol_evc(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if);

int32
if_mgr_del_ds_l2protocol(tbl_interface_t *p_if);

int32
if_mgr_set_flow_statistics(tbl_interface_t *p_db_if, uint8 enable);

int32
if_mgr_set_hash_value_profile_select(tbl_interface_t *p_if);

#define SWITCH_CHECK_PKT_TX_CONDTION(_P_DB_IF_, _PKT_TYPE_) if_mgr_check_packet_tx_condition(_P_DB_IF_, _PKT_TYPE_)

/* modified by liwh for bug 53010, 2019-07-22 */
int32
if_mgr_clear_interface_cfm_meps( tbl_interface_t *p_db_if);

int32
if_mgr_clear_cfm_meps_by_interface_vlan( tbl_interface_t *p_db_if, vid_t vid);
/* liwh end */

#endif /* !__IF_MGR_H__ */

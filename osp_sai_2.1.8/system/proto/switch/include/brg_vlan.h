
#ifndef __BRG_VLAN_H__
#define __BRG_VLAN_H__

#define VLAN_SEC_LOG(fmt, args...) \
do { \
        log_sys(M_MOD_VLAN_SECURITY, E_WARNING, fmt, ##args); \
} while (0)

int32
brg_vlan_add_vlan(vid_t vid);

int32
brg_vlan_del_vlan(vid_t vid);

int32
brg_vlan_set_name(tbl_vlan_t *p_vlan);

int32
brg_vlan_set_enable(tbl_vlan_t *p_vlan);

int32
brg_vlan_set_mac_learning_en(tbl_vlan_t *p_vlan);

int32
brg_vlan_set_security_action(tbl_vlan_t *p_vlan, bool notify);

int32
brg_vlan_set_max_mac_count(tbl_vlan_t *p_vlan, bool notify);

int32
brg_vlan_stats_operation(tbl_vlan_t *p_vlan);

int32
brg_vlan_get_cur_mac_count(tbl_vlan_t *p_vlan);

int32
brg_vlan_start(switch_master_t *master);

int32
brg_vlan_stop(switch_master_t *master);

int32
brg_vlan_cmd_process_vlan(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_vlan_cmd_process_show_vlan(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_pvlan_cmd_process_pvlan(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_vlan_security_log(vid_t vid, mac_addr_t src_mac, mac_addr_t dest_mac, char *if_name);

int32
brg_vlan_set_arpinsp_enable_callback(tbl_vlan_t* p_db_vlan, bool arpinsp_enable);

int32
brg_vlan_cmd_arpinsp_enable_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
brg_vlan_cmd_process_show_dot1q_ethertype(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__BRG_VLAN_H__ */


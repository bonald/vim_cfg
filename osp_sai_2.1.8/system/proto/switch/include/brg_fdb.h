
#ifndef __BRG_FDB_H__
#define __BRG_FDB_H__

#define BRG_LOG(fmt, args...) \
do { \
        log_sys(M_MOD_FDB, E_NOTICE, fmt, ##args); \
} while (0)

int32
brg_fdb_flush_static_fdb();

int32
brg_fdb_flush_static_fdb_vlan(vid_t vid);

int32
brg_fdb_flush_static_fdb_port(uint32 ifindex);

int32
brg_fdb_flush_static_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
brg_fdb_flush_l2mc_fdb();

int32
brg_fdb_flush_l2mc_fdb_vlan(vid_t vid);

int32
brg_fdb_flush_l2mc_fdb_port(uint32 ifindex);

int32
brg_fdb_flush_l2mc_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
brg_fdb_flush_l2mc_fdb_mac(uint8 *mac);

int32
brg_fdb_flush_static_fdb_mac(uint8 *mac);

int32
brg_fdb_flush_dynamic_fdb();

int32
brg_fdb_flush_dynamic_fdb_vlan(vid_t vid);

int32
brg_fdb_flush_dynamic_fdb_port(uint32 ifindex);

int32
brg_fdb_flush_dynamic_fdb_mac(mac_addr_t mac);

int32
brg_fdb_flush_dynamic_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
brg_mcfdb_add_mcfdb(tbl_mcfdb_t *p_mcfdb, uint32 ifindex);

int32
brg_mcfdb_del_mcfdb(tbl_mcfdb_t *p_mcfdb, uint32 ifindex, uint32 *deleted);

int32
brg_mcfdb_vlan_has_manual_mcfdb(vid_t vid);

int32
brg_fdb_add_fdb(tbl_fdb_t *p_fdb);

int32
brg_fdb_del_fdb(tbl_fdb_t *p_fdb);

int32
brg_psfdb_add_psfdb(tbl_psfdb_t *p_fdb);

int32
brg_psfdb_del_psfdb(tbl_psfdb_t *p_fdb);

int32
brg_fdb_set_flush_fdb(ds_flush_fdb_t *p_flush_fdb);

int32
brg_fdb_is_static_fdb_on_port(uint32 ifindex);

int32
brg_psfdb_flush_static_fdb_mac_vlan_port(uint8 *mac, uint32 ifindex, uint16 vid);

int32
brg_psfdb_flush_static_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
brg_macfilter_add_macfilter(tbl_macfilter_t *p_macflt);

int32
brg_macfilter_del_macfilter(tbl_macfilter_t *p_macflt);

int32
brg_macfilter_flush_macfilter();

int32
brg_fdb_start(switch_master_t *master);

int32
brg_fdb_stop(switch_master_t *master);

int32
brg_fdb_cmd_process_fdb(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_fdb_cmd_process_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_psfdb_cmd_process_psfdb(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_fdb_cmd_process_flushfdb(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_fdb_cmd_process_flush_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_psfdb_cmd_process_flushfdb(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
brg_fdb_cmd_show_resource_fdb(cfg_cmd_para_t *para);

int32
brg_psfdb_flush_static_fdb_mac(uint8 *mac);

int32
brg_psfdb_flush_static_fdb_vlan(vid_t vid);

int32
brg_psfdb_flush_static_fdb_port(uint32 ifindex);

int32
brg_psfdb_flush_static_fdb_port_vlan(uint32 ifindex, vid_t vid);

int32
brg_psfdb_flush_static_fdb();

int32
brg_psfdb_add_psfdb_to_fea(uint32 ifindex);

int32
brg_psfdb_del_psfdb_form_fea(uint32 ifindex);


#endif /* !__BRG_FDB_H__ */


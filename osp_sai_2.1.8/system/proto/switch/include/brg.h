
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#ifndef __BRG_H__
#define __BRG_H__

#define BRG_MAX_STATIC_FDB_NUMBER   1024
#define BRG_MAX_L2MCAST_FDB_NUMBER  2048
#define BRG_INVALID_AGEING_TIME     0

int32
brg_global_set_oper_aging_time(uint32 aging_time);

int32
brg_global_set_aging_time(uint32 aging_time);

int32
brg_global_set_hw_learning_en(uint32 enable);

int32
brg_global_set_ipsg_max_port_binding(uint32 max_num);

int32
brg_global_set_port_isolate_mode(uint32 mode);

int
brg_global_set_trap_delay_linkup_time(uint32 linkup_time);

int
brg_global_set_trap_delay_linkdown_time(uint32 linkdown_time);

int32
brg_global_set_l2protocol_cos(uint32 l2protocol_cos);

int32
brg_fea_init();

int32
brg_start(switch_master_t *master);

int32
brg_stop(switch_master_t *master);

int32
brg_alloc_id(cdb_id_type_t type);

int32
brg_release_id(cdb_id_type_t type, int32 id);

int32
brg_set_brg_max_id(cdb_id_type_t type, int32 max);

int32
brg_alloc_pvlan_group_id(tbl_pvlan_t *p_db_pvlan, tbl_interface_t *p_db_if, vid_t community_vlan);

int32
brg_release_pvlan_group_id(tbl_pvlan_t *p_db_pvlan, tbl_interface_t *p_db_if, vid_t community_vlan);

int32
brg_cmd_process_brg_global(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__BRG_H__ */


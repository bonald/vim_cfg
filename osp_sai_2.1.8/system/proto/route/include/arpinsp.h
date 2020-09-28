#ifndef __ARPINSP_H__
#define __ARPINSP_H__

int32
arpinsp_cmd_process_arpinsp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpinsp_set_arpinsp_enable_state(tbl_arpinsp_t *p_arpinsp);

int32
arpinsp_set_arpinsp_filter_state(tbl_arpinsp_t *p_arpinsp);

int32
arpinsp_set_arpinsp_acl_match(tbl_arpinsp_t *p_arpinsp);

int32
arpinsp_set_arpinsp_dhcp_binding(tbl_arpinsp_t *p_arpinsp); 

int32
arpinsp_set_logbuf_entrynum(uint32 arpinsp_log_entry_num);

int32
arpinsp_set_logbuf_number_and_second(uint32 arpinsp_log_num, uint32 arpinsp_log_second);

int32
arpinsp_set_arpinsp_validate(uint32 arpinsp_validate_flag);

bool
arpinsp_is_NULL(tbl_arpinsp_t * p_db_arpinsp);

int32
arpinsp_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpinsp_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpinsp_cmd_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
arpinsp_start(route_master_t *master);

int32
arpinsp_stop(route_master_t *master);

void
arpratelimit_init();

#endif /* !__ARPINSP_H__ */


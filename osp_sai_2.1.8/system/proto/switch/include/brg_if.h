
#ifndef __BRG_IF_H__
#define __BRG_IF_H__

int32
brg_if_set_vlan_port_bmp(uint32 ifindex, vid_t vid, uint32 is_add);

int32
brg_if_add_vlan_for_allowed_all_port(vid_t vid);

int32
_brg_if_clear_vlan_process(tbl_interface_t *p_db_if, vid_t vid);

int32
brg_if_set_pvid(tbl_interface_t *p_if, vid_t pvid, uint32 iter_member);

int32
brg_if_set_inner_native_vlan(tbl_interface_t *p_if, vid_t inner_native_vid, uint32 iter_member);

int32
brg_if_set_qinq_type(tbl_interface_t *p_if, uint8 qinq_type, uint32 iter_member);

int32
brg_if_set_vlan_translation_en(tbl_interface_t *p_if, uint8 vlan_translation_en, uint32 iter_member);

int32
brg_if_apply_vlanmap_table(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, 
        uint32 is_add, cfg_cmd_para_t *para);

int32
brg_if_set_qinq_default_fwd(tbl_interface_t *p_if, uint8 qinq_default_fwd, uint32 iter_member);

int32
brg_if_set_vlan_trans_default_fwd(tbl_interface_t *p_if, uint8 vlan_trans_default_fwd, uint32 iter_member);

int32
brg_if_set_dot1q_tag_native_en(tbl_interface_t *p_if, uint8 dot1q_tag_native_en, uint32 iter_member);

int32
brg_if_set_allowed_vlan_db(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, uint32 iter_member);

int32
brg_if_set_untagged_vlan_db(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, uint32 iter_member);

int32
brg_if_set_ingress_filter(tbl_interface_t *p_if, ds_brgif_t *p_brgif, uint32 iter_member);

int32
brg_if_set_tpid_type(tbl_interface_t *p_if, ds_brgif_t *p_brgif);

int32
brg_if_set_mac_learning_en(tbl_interface_t *p_if, uint32 enable, uint32 iter_member);

int32
brg_if_update_oper_mac_learning_en(tbl_interface_t *p_db_if);

int32
brg_if_set_allowed_vlan_process(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add, uint32 fdb_flush, uint32 iter_member);

int32
brg_if_set_vlan_type_clear(tbl_interface_t *p_db_if);

int32
brg_if_set_port_security_en(tbl_interface_t *p_if, uint32 enable, uint32 iter_member);

int32
brg_if_set_port_security_mac_maximum(tbl_interface_t *p_if, uint32 num, uint32 iter_member);

int32
brg_if_set_port_security_voilation(tbl_interface_t *p_if, int32 mode, uint32 iter_member);

int32
brg_if_clear_port_security(tbl_interface_t *p_db_if);

int32
brg_if_trunk_clear_vlan_translation(tbl_interface_t *p_db_if, uint32 enable, uint32 iter_member);

int32
brg_if_dot1q_clear_vlan_mapping(tbl_interface_t *p_db_if, uint32 iter_member);


#endif /* !__BRG_IF_H__ */


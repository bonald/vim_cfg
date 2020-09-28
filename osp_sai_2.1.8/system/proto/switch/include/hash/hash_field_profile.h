#ifndef __HASH_FIELD_PROFILE_H__
#define __HASH_FIELD_PROFILE_H__


#define HASH_SHOW_SEG_NUM  2

int32
hash_field_profile_init();

int32
hash_field_profile_start(switch_master_t *master);

int32
hash_field_profile_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hash_field_profile_cmd_show_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hash_field_profile_set_description(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_l2_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_ip_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_ipv6_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_gre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_l3vpn_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_nvgre_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_mpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_vpws_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_vpls_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_vxlan_field_value(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_hash_arithmetic(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_mode_symmetry(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_set_bitmap_disable(tbl_hash_field_profile_t* p_hash_fld_pro);

int32
hash_field_profile_create(uint32 index, char *name);

int32
hash_field_profile_check_in_use(uint32 field_id);

#endif
#ifndef __HASH_VALUE_PROFILE_AND_GLOBAL_H__
#define __HASH_VALUE_PROFILE_AND_GLOBAL_H__

#define HASH_OFFSET_HASH_ARITHMETIC_FIRST              0
#define HASH_OFFSET_HASH_ARITHMETIC_SECOND             16

int32
hash_value_global_init(void);

int32
hash_value_profile_and_global_start(switch_master_t *master);

int32
hash_value_profile_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hash_value_global_cmd_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hash_value_cmd_show_process_hash(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hash_value_field_set_description(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_port_channel_dynamic(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_port_channel_static(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_port_channel_all(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_ecmp_dynamic(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_ecmp_static(tbl_hash_value_profile_t* p_hash_value_pro);

int32
hash_value_field_set_ecmp_all(tbl_hash_value_profile_t* p_hash_value_pro);


tbl_hash_value_profile_t *
hash_value_profile_lookup_by_id(uint8 profile_id);

int32
hash_value_global_set_ecmp_dynamic(tbl_hash_value_global_t* p_hash_value_glb);

int32
hash_value_global_set_ecmp_static(tbl_hash_value_global_t* p_hash_value_glb);

int32
hash_value_global_set_ecmp_all(tbl_hash_value_global_t* p_hash_value_glb);

int32
hash_value_global_set_port_channel(tbl_hash_value_global_t* p_hash_value_glb);

int32
hash_value_global_set_lbid(tbl_hash_value_global_t* p_hash_value_glb);

int32
hash_value_profile_lock(uint8 profile_id);

int32
hash_value_profile_unlock(uint8 profile_id);

int32
hash_value_on_modify_hash_value_profile_ingress(uint32 profile_id);

int32
hash_value_on_modify_profile_agg_hash(uint32 profile_id);

int32
hash_value_on_modify_global_agg_hash(uint32 profile_id);

int32
hash_value_on_modify_hash_value_profile_egress(uint32 profile_id);

#endif
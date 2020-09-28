
#ifndef __TBL_FEA_COPP_TRAP_GROUP_H__
#define __TBL_FEA_COPP_TRAP_GROUP_H__

int32
tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, uint32 sync);

int32
tbl_fea_copp_trap_group_del_fea_copp_trap_group_sync(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key, uint32 sync);

int32
tbl_fea_copp_trap_group_set_fea_copp_trap_group_field_sync(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_fea_copp_trap_group_field_id_t field_id, uint32 sync);

int32
tbl_fea_copp_trap_group_add_fea_copp_trap_group(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group);

int32
tbl_fea_copp_trap_group_del_fea_copp_trap_group(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key);

int32
tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_fea_copp_trap_group_field_id_t field_id);

tbl_fea_copp_trap_group_t*
tbl_fea_copp_trap_group_get_fea_copp_trap_group(tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key);

char*
tbl_fea_copp_trap_group_key_val2str(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char *str, uint32 str_len);

int32
tbl_fea_copp_trap_group_key_str2val(char *str, tbl_fea_copp_trap_group_t *p_fea_copp_trap_group);

int32
tbl_fea_copp_trap_group_dump_one(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, tbl_iter_args_t *pargs);

char*
tbl_fea_copp_trap_group_key_name_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char* str);

char*
tbl_fea_copp_trap_group_key_value_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char* str);

char*
tbl_fea_copp_trap_group_field_name_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, int32 field_id, char* str);

char*
tbl_fea_copp_trap_group_field_value_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, int32 field_id, char* str);

char**
tbl_fea_copp_trap_group_table_dump(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, char** str);

int32
tbl_fea_copp_trap_group_table_parser(char** array, char* key_value, tbl_fea_copp_trap_group_t *p_fea_copp_trap_group);

int32
tbl_fea_copp_trap_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_copp_trap_group_master_t*
tbl_fea_copp_trap_group_get_master();

tbl_fea_copp_trap_group_master_t*
tbl_fea_copp_trap_group_init_fea_copp_trap_group();

#endif /* !__TBL_FEA_COPP_TRAP_GROUP_H__ */


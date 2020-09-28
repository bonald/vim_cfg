
#ifndef __TBL_SYS_SPEC_H__
#define __TBL_SYS_SPEC_H__

int32
tbl_sys_spec_add_sys_spec_sync(tbl_sys_spec_t *p_sys_spec, uint32 sync);

int32
tbl_sys_spec_del_sys_spec_sync(tbl_sys_spec_key_t *p_sys_spec_key, uint32 sync);

int32
tbl_sys_spec_set_sys_spec_field_sync(tbl_sys_spec_t *p_sys_spec, tbl_sys_spec_field_id_t field_id, uint32 sync);

int32
tbl_sys_spec_add_sys_spec(tbl_sys_spec_t *p_sys_spec);

int32
tbl_sys_spec_del_sys_spec(tbl_sys_spec_key_t *p_sys_spec_key);

int32
tbl_sys_spec_set_sys_spec_field(tbl_sys_spec_t *p_sys_spec, tbl_sys_spec_field_id_t field_id);

tbl_sys_spec_t*
tbl_sys_spec_get_sys_spec(tbl_sys_spec_key_t *p_sys_spec_key);

char*
tbl_sys_spec_key_val2str(tbl_sys_spec_t *p_sys_spec, char *str, uint32 str_len);

int32
tbl_sys_spec_key_str2val(char *str, tbl_sys_spec_t *p_sys_spec);

int32
tbl_sys_spec_dump_one(tbl_sys_spec_t *p_sys_spec, tbl_iter_args_t *pargs);

char*
tbl_sys_spec_key_name_dump(tbl_sys_spec_t *p_sys_spec, char* str);

char*
tbl_sys_spec_key_value_dump(tbl_sys_spec_t *p_sys_spec, char* str);

char*
tbl_sys_spec_field_name_dump(tbl_sys_spec_t *p_sys_spec, int32 field_id, char* str);

char*
tbl_sys_spec_field_value_dump(tbl_sys_spec_t *p_sys_spec, int32 field_id, char* str);

char**
tbl_sys_spec_table_dump(tbl_sys_spec_t *p_sys_spec, char** str);

int32
tbl_sys_spec_table_parser(char** array, char* key_value, tbl_sys_spec_t *p_sys_spec);

int32
tbl_sys_spec_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_sys_spec_master_t*
tbl_sys_spec_get_master();

tbl_sys_spec_master_t*
tbl_sys_spec_init_sys_spec();

tbl_sys_spec_t*
tbl_sys_spec_get_sys_spec_by_type(uint32 type);

uint32
tbl_sys_spec_get_sys_spec_field_by_type(uint32 type, int32 field_id);

#endif /* !__TBL_SYS_SPEC_H__ */


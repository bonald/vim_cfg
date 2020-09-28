
#ifndef __TBL_ENABLE_H__
#define __TBL_ENABLE_H__

int32
tbl_enable_add_enable_sync(tbl_enable_t *p_enable, uint32 sync);

int32
tbl_enable_del_enable_sync(tbl_enable_key_t *p_enable_key, uint32 sync);

int32
tbl_enable_set_enable_field_sync(tbl_enable_t *p_enable, tbl_enable_field_id_t field_id, uint32 sync);

int32
tbl_enable_add_enable(tbl_enable_t *p_enable);

int32
tbl_enable_del_enable(tbl_enable_key_t *p_enable_key);

int32
tbl_enable_set_enable_field(tbl_enable_t *p_enable, tbl_enable_field_id_t field_id);

tbl_enable_t*
tbl_enable_get_enable(tbl_enable_key_t *p_enable_key);

char*
tbl_enable_key_val2str(tbl_enable_t *p_enable, char *str, uint32 str_len);

int32
tbl_enable_key_str2val(char *str, tbl_enable_t *p_enable);

int32
tbl_enable_dump_one(tbl_enable_t *p_enable, tbl_iter_args_t *pargs);

char*
tbl_enable_key_name_dump(tbl_enable_t *p_enable, char* str);

char*
tbl_enable_key_value_dump(tbl_enable_t *p_enable, char* str);

char*
tbl_enable_field_name_dump(tbl_enable_t *p_enable, int32 field_id, char* str);

char*
tbl_enable_field_value_dump(tbl_enable_t *p_enable, int32 field_id, char* str);

char**
tbl_enable_table_dump(tbl_enable_t *p_enable, char** str);

int32
tbl_enable_table_parser(char** array, char* key_value, tbl_enable_t *p_enable);

int32
tbl_enable_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_enable_master_t*
tbl_enable_get_master();

tbl_enable_master_t*
tbl_enable_init_enable();

#endif /* !__TBL_ENABLE_H__ */


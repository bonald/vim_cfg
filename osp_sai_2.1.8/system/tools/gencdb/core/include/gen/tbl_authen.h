
#ifndef __TBL_AUTHEN_H__
#define __TBL_AUTHEN_H__

int32
tbl_authen_add_authen_sync(tbl_authen_t *p_authen, uint32 sync);

int32
tbl_authen_del_authen_sync(tbl_authen_key_t *p_authen_key, uint32 sync);

int32
tbl_authen_set_authen_field_sync(tbl_authen_t *p_authen, tbl_authen_field_id_t field_id, uint32 sync);

int32
tbl_authen_add_authen(tbl_authen_t *p_authen);

int32
tbl_authen_del_authen(tbl_authen_key_t *p_authen_key);

int32
tbl_authen_set_authen_field(tbl_authen_t *p_authen, tbl_authen_field_id_t field_id);

tbl_authen_t*
tbl_authen_get_authen(tbl_authen_key_t *p_authen_key);

char*
tbl_authen_key_val2str(tbl_authen_t *p_authen, char *str, uint32 str_len);

int32
tbl_authen_key_str2val(char *str, tbl_authen_t *p_authen);

int32
tbl_authen_dump_one(tbl_authen_t *p_authen, tbl_iter_args_t *pargs);

char*
tbl_authen_key_name_dump(tbl_authen_t *p_authen, char* str);

char*
tbl_authen_key_value_dump(tbl_authen_t *p_authen, char* str);

char*
tbl_authen_field_name_dump(tbl_authen_t *p_authen, int32 field_id, char* str);

char*
tbl_authen_field_value_dump(tbl_authen_t *p_authen, int32 field_id, char* str);

char**
tbl_authen_table_dump(tbl_authen_t *p_authen, char** str);

int32
tbl_authen_table_parser(char** array, char* key_value, tbl_authen_t *p_authen);

int32
tbl_authen_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_authen_master_t*
tbl_authen_get_master();

tbl_authen_master_t*
tbl_authen_init_authen();

#endif /* !__TBL_AUTHEN_H__ */


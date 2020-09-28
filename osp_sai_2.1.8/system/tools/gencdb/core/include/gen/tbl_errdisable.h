
#ifndef __TBL_ERRDISABLE_H__
#define __TBL_ERRDISABLE_H__

int32
tbl_errdisable_add_errdisable_sync(tbl_errdisable_t *p_errdisable, uint32 sync);

int32
tbl_errdisable_del_errdisable_sync(tbl_errdisable_key_t *p_errdisable_key, uint32 sync);

int32
tbl_errdisable_set_errdisable_field_sync(tbl_errdisable_t *p_errdisable, tbl_errdisable_field_id_t field_id, uint32 sync);

int32
tbl_errdisable_add_errdisable(tbl_errdisable_t *p_errdisable);

int32
tbl_errdisable_del_errdisable(tbl_errdisable_key_t *p_errdisable_key);

int32
tbl_errdisable_set_errdisable_field(tbl_errdisable_t *p_errdisable, tbl_errdisable_field_id_t field_id);

tbl_errdisable_t*
tbl_errdisable_get_errdisable(tbl_errdisable_key_t *p_errdisable_key);

char*
tbl_errdisable_key_val2str(tbl_errdisable_t *p_errdisable, char *str, uint32 str_len);

int32
tbl_errdisable_key_str2val(char *str, tbl_errdisable_t *p_errdisable);

int32
tbl_errdisable_dump_one(tbl_errdisable_t *p_errdisable, tbl_iter_args_t *pargs);

char*
tbl_errdisable_key_name_dump(tbl_errdisable_t *p_errdisable, char* str);

char*
tbl_errdisable_key_value_dump(tbl_errdisable_t *p_errdisable, char* str);

char*
tbl_errdisable_field_name_dump(tbl_errdisable_t *p_errdisable, int32 field_id, char* str);

char*
tbl_errdisable_field_value_dump(tbl_errdisable_t *p_errdisable, int32 field_id, char* str);

char**
tbl_errdisable_table_dump(tbl_errdisable_t *p_errdisable, char** str);

int32
tbl_errdisable_table_parser(char** array, char* key_value, tbl_errdisable_t *p_errdisable);

int32
tbl_errdisable_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_errdisable_master_t*
tbl_errdisable_get_master();

tbl_errdisable_master_t*
tbl_errdisable_init_errdisable();

#endif /* !__TBL_ERRDISABLE_H__ */


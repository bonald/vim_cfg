
#ifndef __TBL_DOT1X_GLOBAL_H__
#define __TBL_DOT1X_GLOBAL_H__

int32
tbl_dot1x_global_set_dot1x_global_field_sync(tbl_dot1x_global_t *p_dot1x_global, tbl_dot1x_global_field_id_t field_id, uint32 sync);

int32
tbl_dot1x_global_set_dot1x_global_field(tbl_dot1x_global_t *p_dot1x_global, tbl_dot1x_global_field_id_t field_id);

tbl_dot1x_global_t*
tbl_dot1x_global_get_dot1x_global();

char*
tbl_dot1x_global_key_val2str(tbl_dot1x_global_t *p_dot1x_global, char *str, uint32 str_len);

int32
tbl_dot1x_global_key_str2val(char *str, tbl_dot1x_global_t *p_dot1x_global);

int32
tbl_dot1x_global_dump_one(tbl_dot1x_global_t *p_dot1x_global, tbl_iter_args_t *pargs);

char*
tbl_dot1x_global_key_name_dump(tbl_dot1x_global_t *p_dot1x_global, char* str);

char*
tbl_dot1x_global_key_value_dump(tbl_dot1x_global_t *p_dot1x_global, char* str);

char*
tbl_dot1x_global_field_name_dump(tbl_dot1x_global_t *p_dot1x_global, int32 field_id, char* str);

char*
tbl_dot1x_global_field_value_dump(tbl_dot1x_global_t *p_dot1x_global, int32 field_id, char* str);

char**
tbl_dot1x_global_table_dump(tbl_dot1x_global_t *p_dot1x_global, char** str);

int32
tbl_dot1x_global_table_parser(char** array, char* key_value, tbl_dot1x_global_t *p_dot1x_global);

int32
tbl_dot1x_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_dot1x_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dot1x_global_t*
tbl_dot1x_global_init_dot1x_global();

#endif /* !__TBL_DOT1X_GLOBAL_H__ */


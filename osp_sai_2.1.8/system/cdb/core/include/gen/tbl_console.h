
#ifndef __TBL_CONSOLE_H__
#define __TBL_CONSOLE_H__

int32
tbl_console_set_console_field_sync(tbl_console_t *p_console, tbl_console_field_id_t field_id, uint32 sync);

int32
tbl_console_set_console_field(tbl_console_t *p_console, tbl_console_field_id_t field_id);

tbl_console_t*
tbl_console_get_console();

char*
tbl_console_key_val2str(tbl_console_t *p_console, char *str, uint32 str_len);

int32
tbl_console_key_str2val(char *str, tbl_console_t *p_console);

int32
tbl_console_dump_one(tbl_console_t *p_console, tbl_iter_args_t *pargs);

char*
tbl_console_key_name_dump(tbl_console_t *p_console, char* str);

char*
tbl_console_key_value_dump(tbl_console_t *p_console, char* str);

char*
tbl_console_field_name_dump(tbl_console_t *p_console, int32 field_id, char* str);

char*
tbl_console_field_value_dump(tbl_console_t *p_console, int32 field_id, char* str);

char**
tbl_console_table_dump(tbl_console_t *p_console, char** str);

int32
tbl_console_table_parser(char** array, char* key_value, tbl_console_t *p_console);

int32
tbl_console_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_console_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_console_t*
tbl_console_init_console();

#endif /* !__TBL_CONSOLE_H__ */



#ifndef __TBL_LOG_H__
#define __TBL_LOG_H__

int32
tbl_log_add_log_sync(tbl_log_t *p_log, uint32 sync);

int32
tbl_log_del_log_sync(tbl_log_key_t *p_log_key, uint32 sync);

int32
tbl_log_set_log_field_sync(tbl_log_t *p_log, tbl_log_field_id_t field_id, uint32 sync);

int32
tbl_log_add_log(tbl_log_t *p_log);

int32
tbl_log_del_log(tbl_log_key_t *p_log_key);

int32
tbl_log_set_log_field(tbl_log_t *p_log, tbl_log_field_id_t field_id);

tbl_log_t*
tbl_log_get_log(tbl_log_key_t *p_log_key);

char*
tbl_log_key_val2str(tbl_log_t *p_log, char *str, uint32 str_len);

int32
tbl_log_key_str2val(char *str, tbl_log_t *p_log);

int32
tbl_log_dump_one(tbl_log_t *p_log, tbl_iter_args_t *pargs);

char*
tbl_log_key_name_dump(tbl_log_t *p_log, char* str);

char*
tbl_log_key_value_dump(tbl_log_t *p_log, char* str);

char*
tbl_log_field_name_dump(tbl_log_t *p_log, int32 field_id, char* str);

char*
tbl_log_field_value_dump(tbl_log_t *p_log, int32 field_id, char* str);

char**
tbl_log_table_dump(tbl_log_t *p_log, char** str);

int32
tbl_log_table_parser(char** array, char* key_value, tbl_log_t *p_log);

int32
tbl_log_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_log_master_t*
tbl_log_get_master();

tbl_log_master_t*
tbl_log_init_log();

#endif /* !__TBL_LOG_H__ */


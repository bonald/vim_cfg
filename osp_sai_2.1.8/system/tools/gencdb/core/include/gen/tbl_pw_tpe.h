
#ifndef __TBL_PW_TPE_H__
#define __TBL_PW_TPE_H__

int32
tbl_pw_tpe_add_pw_tpe_sync(tbl_pw_tpe_t *p_pw_tpe, uint32 sync);

int32
tbl_pw_tpe_del_pw_tpe_sync(tbl_pw_tpe_key_t *p_pw_tpe_key, uint32 sync);

int32
tbl_pw_tpe_set_pw_tpe_field_sync(tbl_pw_tpe_t *p_pw_tpe, tbl_pw_tpe_field_id_t field_id, uint32 sync);

int32
tbl_pw_tpe_add_pw_tpe(tbl_pw_tpe_t *p_pw_tpe);

int32
tbl_pw_tpe_del_pw_tpe(tbl_pw_tpe_key_t *p_pw_tpe_key);

int32
tbl_pw_tpe_set_pw_tpe_field(tbl_pw_tpe_t *p_pw_tpe, tbl_pw_tpe_field_id_t field_id);

tbl_pw_tpe_t*
tbl_pw_tpe_get_pw_tpe(tbl_pw_tpe_key_t *p_pw_tpe_key);

char*
tbl_pw_tpe_key_val2str(tbl_pw_tpe_t *p_pw_tpe, char *str, uint32 str_len);

int32
tbl_pw_tpe_key_str2val(char *str, tbl_pw_tpe_t *p_pw_tpe);

int32
tbl_pw_tpe_dump_one(tbl_pw_tpe_t *p_pw_tpe, tbl_iter_args_t *pargs);

char*
tbl_pw_tpe_key_name_dump(tbl_pw_tpe_t *p_pw_tpe, char* str);

char*
tbl_pw_tpe_key_value_dump(tbl_pw_tpe_t *p_pw_tpe, char* str);

char*
tbl_pw_tpe_field_name_dump(tbl_pw_tpe_t *p_pw_tpe, int32 field_id, char* str);

char*
tbl_pw_tpe_field_value_dump(tbl_pw_tpe_t *p_pw_tpe, int32 field_id, char* str);

char**
tbl_pw_tpe_table_dump(tbl_pw_tpe_t *p_pw_tpe, char** str);

int32
tbl_pw_tpe_table_parser(char** array, char* key_value, tbl_pw_tpe_t *p_pw_tpe);

int32
tbl_pw_tpe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pw_tpe_master_t*
tbl_pw_tpe_get_master();

tbl_pw_tpe_master_t*
tbl_pw_tpe_init_pw_tpe();

#endif /* !__TBL_PW_TPE_H__ */


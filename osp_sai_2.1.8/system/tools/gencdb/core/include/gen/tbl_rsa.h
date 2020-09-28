
#ifndef __TBL_RSA_H__
#define __TBL_RSA_H__

int32
tbl_rsa_add_rsa_sync(tbl_rsa_t *p_rsa, uint32 sync);

int32
tbl_rsa_del_rsa_sync(tbl_rsa_key_t *p_rsa_key, uint32 sync);

int32
tbl_rsa_set_rsa_field_sync(tbl_rsa_t *p_rsa, tbl_rsa_field_id_t field_id, uint32 sync);

int32
tbl_rsa_add_rsa(tbl_rsa_t *p_rsa);

int32
tbl_rsa_del_rsa(tbl_rsa_key_t *p_rsa_key);

int32
tbl_rsa_set_rsa_field(tbl_rsa_t *p_rsa, tbl_rsa_field_id_t field_id);

tbl_rsa_t*
tbl_rsa_get_rsa(tbl_rsa_key_t *p_rsa_key);

char*
tbl_rsa_key_val2str(tbl_rsa_t *p_rsa, char *str, uint32 str_len);

int32
tbl_rsa_key_str2val(char *str, tbl_rsa_t *p_rsa);

int32
tbl_rsa_dump_one(tbl_rsa_t *p_rsa, tbl_iter_args_t *pargs);

char*
tbl_rsa_key_name_dump(tbl_rsa_t *p_rsa, char* str);

char*
tbl_rsa_key_value_dump(tbl_rsa_t *p_rsa, char* str);

char*
tbl_rsa_field_name_dump(tbl_rsa_t *p_rsa, int32 field_id, char* str);

char*
tbl_rsa_field_value_dump(tbl_rsa_t *p_rsa, int32 field_id, char* str);

char**
tbl_rsa_table_dump(tbl_rsa_t *p_rsa, char** str);

int32
tbl_rsa_table_parser(char** array, char* key_value, tbl_rsa_t *p_rsa);

int32
tbl_rsa_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rsa_master_t*
tbl_rsa_get_master();

tbl_rsa_master_t*
tbl_rsa_init_rsa();

#endif /* !__TBL_RSA_H__ */


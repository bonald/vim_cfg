
#ifndef __TBL_LOGIN_SECURITY_GLB_H__
#define __TBL_LOGIN_SECURITY_GLB_H__

int32
tbl_login_security_glb_set_login_security_glb_field_sync(tbl_login_security_glb_t *p_login_security_glb, tbl_login_security_glb_field_id_t field_id, uint32 sync);

int32
tbl_login_security_glb_set_login_security_glb_field(tbl_login_security_glb_t *p_login_security_glb, tbl_login_security_glb_field_id_t field_id);

tbl_login_security_glb_t*
tbl_login_security_glb_get_login_security_glb();

char*
tbl_login_security_glb_key_val2str(tbl_login_security_glb_t *p_login_security_glb, char *str, uint32 str_len);

int32
tbl_login_security_glb_key_str2val(char *str, tbl_login_security_glb_t *p_login_security_glb);

int32
tbl_login_security_glb_dump_one(tbl_login_security_glb_t *p_login_security_glb, tbl_iter_args_t *pargs);

char*
tbl_login_security_glb_key_name_dump(tbl_login_security_glb_t *p_login_security_glb, char* str);

char*
tbl_login_security_glb_key_value_dump(tbl_login_security_glb_t *p_login_security_glb, char* str);

char*
tbl_login_security_glb_field_name_dump(tbl_login_security_glb_t *p_login_security_glb, int32 field_id, char* str);

char*
tbl_login_security_glb_field_value_dump(tbl_login_security_glb_t *p_login_security_glb, int32 field_id, char* str);

char**
tbl_login_security_glb_table_dump(tbl_login_security_glb_t *p_login_security_glb, char** str);

int32
tbl_login_security_glb_table_parser(char** array, char* key_value, tbl_login_security_glb_t *p_login_security_glb);

int32
tbl_login_security_glb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_login_security_glb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_login_security_glb_t*
tbl_login_security_glb_init_login_security_glb();

#endif /* !__TBL_LOGIN_SECURITY_GLB_H__ */



#ifndef __TBL_MANAGE_IF_H__
#define __TBL_MANAGE_IF_H__

int32
tbl_manage_if_set_manage_if_field_sync(tbl_manage_if_t *p_mng_if, tbl_manage_if_field_id_t field_id, uint32 sync);

int32
tbl_manage_if_set_manage_if_field(tbl_manage_if_t *p_mng_if, tbl_manage_if_field_id_t field_id);

tbl_manage_if_t*
tbl_manage_if_get_manage_if();

char*
tbl_manage_if_key_val2str(tbl_manage_if_t *p_mng_if, char *str, uint32 str_len);

int32
tbl_manage_if_key_str2val(char *str, tbl_manage_if_t *p_mng_if);

int32
tbl_manage_if_dump_one(tbl_manage_if_t *p_mng_if, tbl_iter_args_t *pargs);

char*
tbl_manage_if_key_name_dump(tbl_manage_if_t *p_mng_if, char* str);

char*
tbl_manage_if_key_value_dump(tbl_manage_if_t *p_mng_if, char* str);

char*
tbl_manage_if_field_name_dump(tbl_manage_if_t *p_mng_if, int32 field_id, char* str);

char*
tbl_manage_if_field_value_dump(tbl_manage_if_t *p_mng_if, int32 field_id, char* str);

char**
tbl_manage_if_table_dump(tbl_manage_if_t *p_mng_if, char** str);

int32
tbl_manage_if_table_parser(char** array, char* key_value, tbl_manage_if_t *p_mng_if);

int32
tbl_manage_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_manage_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_manage_if_t*
tbl_manage_if_init_manage_if();

#endif /* !__TBL_MANAGE_IF_H__ */


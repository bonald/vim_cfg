
#ifndef __TBL_MRT_H__
#define __TBL_MRT_H__

int32
tbl_mrt_add_mrt_sync(tbl_mrt_t *p_mrt, uint32 sync);

int32
tbl_mrt_del_mrt_sync(tbl_mrt_key_t *p_mrt_key, uint32 sync);

int32
tbl_mrt_set_mrt_field_sync(tbl_mrt_t *p_mrt, tbl_mrt_field_id_t field_id, uint32 sync);

int32
tbl_mrt_add_mrt(tbl_mrt_t *p_mrt);

int32
tbl_mrt_del_mrt(tbl_mrt_key_t *p_mrt_key);

int32
tbl_mrt_set_mrt_field(tbl_mrt_t *p_mrt, tbl_mrt_field_id_t field_id);

tbl_mrt_t*
tbl_mrt_get_mrt(tbl_mrt_key_t *p_mrt_key);

char*
tbl_mrt_key_val2str(tbl_mrt_t *p_mrt, char *str, uint32 str_len);

int32
tbl_mrt_key_str2val(char *str, tbl_mrt_t *p_mrt);

int32
tbl_mrt_dump_one(tbl_mrt_t *p_mrt, tbl_iter_args_t *pargs);

char*
tbl_mrt_key_name_dump(tbl_mrt_t *p_mrt, char* str);

char*
tbl_mrt_key_value_dump(tbl_mrt_t *p_mrt, char* str);

char*
tbl_mrt_field_name_dump(tbl_mrt_t *p_mrt, int32 field_id, char* str);

char*
tbl_mrt_field_value_dump(tbl_mrt_t *p_mrt, int32 field_id, char* str);

char**
tbl_mrt_table_dump(tbl_mrt_t *p_mrt, char** str);

int32
tbl_mrt_table_parser(char** array, char* key_value, tbl_mrt_t *p_mrt);

int32
tbl_mrt_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mrt_master_t*
tbl_mrt_get_master();

tbl_mrt_master_t*
tbl_mrt_init_mrt();

#endif /* !__TBL_MRT_H__ */


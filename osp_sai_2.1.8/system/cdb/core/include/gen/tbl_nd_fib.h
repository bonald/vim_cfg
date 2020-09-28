
#ifndef __TBL_ND_FIB_H__
#define __TBL_ND_FIB_H__

int32
tbl_nd_fib_add_nd_fib_sync(tbl_nd_fib_t *p_nd_fib, uint32 sync);

int32
tbl_nd_fib_del_nd_fib_sync(tbl_nd_fib_key_t *p_nd_fib_key, uint32 sync);

int32
tbl_nd_fib_set_nd_fib_field_sync(tbl_nd_fib_t *p_nd_fib, tbl_nd_fib_field_id_t field_id, uint32 sync);

int32
tbl_nd_fib_add_nd_fib(tbl_nd_fib_t *p_nd_fib);

int32
tbl_nd_fib_del_nd_fib(tbl_nd_fib_key_t *p_nd_fib_key);

int32
tbl_nd_fib_set_nd_fib_field(tbl_nd_fib_t *p_nd_fib, tbl_nd_fib_field_id_t field_id);

tbl_nd_fib_t*
tbl_nd_fib_get_nd_fib(tbl_nd_fib_key_t *p_nd_fib_key);

char*
tbl_nd_fib_key_val2str(tbl_nd_fib_t *p_nd_fib, char *str, uint32 str_len);

int32
tbl_nd_fib_key_str2val(char *str, tbl_nd_fib_t *p_nd_fib);

int32
tbl_nd_fib_dump_one(tbl_nd_fib_t *p_nd_fib, tbl_iter_args_t *pargs);

char*
tbl_nd_fib_key_name_dump(tbl_nd_fib_t *p_nd_fib, char* str);

char*
tbl_nd_fib_key_value_dump(tbl_nd_fib_t *p_nd_fib, char* str);

char*
tbl_nd_fib_field_name_dump(tbl_nd_fib_t *p_nd_fib, int32 field_id, char* str);

char*
tbl_nd_fib_field_value_dump(tbl_nd_fib_t *p_nd_fib, int32 field_id, char* str);

char**
tbl_nd_fib_table_dump(tbl_nd_fib_t *p_nd_fib, char** str);

int32
tbl_nd_fib_table_parser(char** array, char* key_value, tbl_nd_fib_t *p_nd_fib);

int32
tbl_nd_fib_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nd_fib_master_t*
tbl_nd_fib_get_master();

tbl_nd_fib_master_t*
tbl_nd_fib_init_nd_fib();

#endif /* !__TBL_ND_FIB_H__ */


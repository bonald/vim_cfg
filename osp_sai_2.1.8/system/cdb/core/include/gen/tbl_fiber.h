
#ifndef __TBL_FIBER_H__
#define __TBL_FIBER_H__

int32
tbl_fiber_add_fiber_sync(tbl_fiber_t *p_fiber, uint32 sync);

int32
tbl_fiber_del_fiber_sync(tbl_fiber_key_t *p_fiber_key, uint32 sync);

int32
tbl_fiber_set_fiber_field_sync(tbl_fiber_t *p_fiber, tbl_fiber_field_id_t field_id, uint32 sync);

int32
tbl_fiber_add_fiber(tbl_fiber_t *p_fiber);

int32
tbl_fiber_del_fiber(tbl_fiber_key_t *p_fiber_key);

int32
tbl_fiber_set_fiber_field(tbl_fiber_t *p_fiber, tbl_fiber_field_id_t field_id);

tbl_fiber_t*
tbl_fiber_get_fiber(tbl_fiber_key_t *p_fiber_key);

char*
tbl_fiber_key_val2str(tbl_fiber_t *p_fiber, char *str, uint32 str_len);

int32
tbl_fiber_key_str2val(char *str, tbl_fiber_t *p_fiber);

int32
tbl_fiber_dump_one(tbl_fiber_t *p_fiber, tbl_iter_args_t *pargs);

char*
tbl_fiber_key_name_dump(tbl_fiber_t *p_fiber, char* str);

char*
tbl_fiber_key_value_dump(tbl_fiber_t *p_fiber, char* str);

char*
tbl_fiber_field_name_dump(tbl_fiber_t *p_fiber, int32 field_id, char* str);

char*
tbl_fiber_field_value_dump(tbl_fiber_t *p_fiber, int32 field_id, char* str);

char**
tbl_fiber_table_dump(tbl_fiber_t *p_fiber, char** str);

int32
tbl_fiber_table_parser(char** array, char* key_value, tbl_fiber_t *p_fiber);

int32
tbl_fiber_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fiber_master_t*
tbl_fiber_get_master();

tbl_fiber_master_t*
tbl_fiber_init_fiber();

#endif /* !__TBL_FIBER_H__ */


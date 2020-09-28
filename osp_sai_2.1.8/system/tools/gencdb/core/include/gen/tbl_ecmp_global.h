
#ifndef __TBL_ECMP_GLOBAL_H__
#define __TBL_ECMP_GLOBAL_H__

int32
tbl_ecmp_global_set_ecmp_global_field_sync(tbl_ecmp_global_t *p_ecmp_glb, tbl_ecmp_global_field_id_t field_id, uint32 sync);

int32
tbl_ecmp_global_set_ecmp_global_field(tbl_ecmp_global_t *p_ecmp_glb, tbl_ecmp_global_field_id_t field_id);

tbl_ecmp_global_t*
tbl_ecmp_global_get_ecmp_global();

char*
tbl_ecmp_global_key_val2str(tbl_ecmp_global_t *p_ecmp_glb, char *str, uint32 str_len);

int32
tbl_ecmp_global_key_str2val(char *str, tbl_ecmp_global_t *p_ecmp_glb);

int32
tbl_ecmp_global_dump_one(tbl_ecmp_global_t *p_ecmp_glb, tbl_iter_args_t *pargs);

char*
tbl_ecmp_global_key_name_dump(tbl_ecmp_global_t *p_ecmp_glb, char* str);

char*
tbl_ecmp_global_key_value_dump(tbl_ecmp_global_t *p_ecmp_glb, char* str);

char*
tbl_ecmp_global_field_name_dump(tbl_ecmp_global_t *p_ecmp_glb, int32 field_id, char* str);

char*
tbl_ecmp_global_field_value_dump(tbl_ecmp_global_t *p_ecmp_glb, int32 field_id, char* str);

char**
tbl_ecmp_global_table_dump(tbl_ecmp_global_t *p_ecmp_glb, char** str);

int32
tbl_ecmp_global_table_parser(char** array, char* key_value, tbl_ecmp_global_t *p_ecmp_glb);

int32
tbl_ecmp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ecmp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ecmp_global_t*
tbl_ecmp_global_init_ecmp_global();

#endif /* !__TBL_ECMP_GLOBAL_H__ */


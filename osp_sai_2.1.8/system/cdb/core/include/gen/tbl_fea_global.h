
#ifndef __TBL_FEA_GLOBAL_H__
#define __TBL_FEA_GLOBAL_H__

int32
tbl_fea_global_set_fea_global_field_sync(tbl_fea_global_t *p_fea_global, tbl_fea_global_field_id_t field_id, uint32 sync);

int32
tbl_fea_global_set_fea_global_field(tbl_fea_global_t *p_fea_global, tbl_fea_global_field_id_t field_id);

tbl_fea_global_t*
tbl_fea_global_get_fea_global();

char*
tbl_fea_global_key_val2str(tbl_fea_global_t *p_fea_global, char *str, uint32 str_len);

int32
tbl_fea_global_key_str2val(char *str, tbl_fea_global_t *p_fea_global);

int32
tbl_fea_global_dump_one(tbl_fea_global_t *p_fea_global, tbl_iter_args_t *pargs);

char*
tbl_fea_global_key_name_dump(tbl_fea_global_t *p_fea_global, char* str);

char*
tbl_fea_global_key_value_dump(tbl_fea_global_t *p_fea_global, char* str);

char*
tbl_fea_global_field_name_dump(tbl_fea_global_t *p_fea_global, int32 field_id, char* str);

char*
tbl_fea_global_field_value_dump(tbl_fea_global_t *p_fea_global, int32 field_id, char* str);

char**
tbl_fea_global_table_dump(tbl_fea_global_t *p_fea_global, char** str);

int32
tbl_fea_global_table_parser(char** array, char* key_value, tbl_fea_global_t *p_fea_global);

int32
tbl_fea_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_fea_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_global_t*
tbl_fea_global_init_fea_global();

#endif /* !__TBL_FEA_GLOBAL_H__ */


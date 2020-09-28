
#ifndef __TBL_MLAG_H__
#define __TBL_MLAG_H__

int32
tbl_mlag_set_mlag_field_sync(tbl_mlag_t *p_mlag, tbl_mlag_field_id_t field_id, uint32 sync);

int32
tbl_mlag_set_mlag_field(tbl_mlag_t *p_mlag, tbl_mlag_field_id_t field_id);

tbl_mlag_t*
tbl_mlag_get_mlag();

char*
tbl_mlag_key_val2str(tbl_mlag_t *p_mlag, char *str, uint32 str_len);

int32
tbl_mlag_key_str2val(char *str, tbl_mlag_t *p_mlag);

int32
tbl_mlag_dump_one(tbl_mlag_t *p_mlag, tbl_iter_args_t *pargs);

char*
tbl_mlag_key_name_dump(tbl_mlag_t *p_mlag, char* str);

char*
tbl_mlag_key_value_dump(tbl_mlag_t *p_mlag, char* str);

char*
tbl_mlag_field_name_dump(tbl_mlag_t *p_mlag, int32 field_id, char* str);

char*
tbl_mlag_field_value_dump(tbl_mlag_t *p_mlag, int32 field_id, char* str);

char**
tbl_mlag_table_dump(tbl_mlag_t *p_mlag, char** str);

int32
tbl_mlag_table_parser(char** array, char* key_value, tbl_mlag_t *p_mlag);

int32
tbl_mlag_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_mlag_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mlag_t*
tbl_mlag_init_mlag();

#endif /* !__TBL_MLAG_H__ */


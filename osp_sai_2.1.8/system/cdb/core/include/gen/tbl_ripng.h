
#ifndef __TBL_RIPNG_H__
#define __TBL_RIPNG_H__

int32
tbl_ripng_add_ripng_sync(tbl_ripng_t *p_ripng, uint32 sync);

int32
tbl_ripng_del_ripng_sync(tbl_ripng_key_t *p_ripng_key, uint32 sync);

int32
tbl_ripng_set_ripng_field_sync(tbl_ripng_t *p_ripng, tbl_ripng_field_id_t field_id, uint32 sync);

int32
tbl_ripng_add_ripng(tbl_ripng_t *p_ripng);

int32
tbl_ripng_del_ripng(tbl_ripng_key_t *p_ripng_key);

int32
tbl_ripng_set_ripng_field(tbl_ripng_t *p_ripng, tbl_ripng_field_id_t field_id);

tbl_ripng_t*
tbl_ripng_get_ripng(tbl_ripng_key_t *p_ripng_key);

char*
tbl_ripng_key_val2str(tbl_ripng_t *p_ripng, char *str, uint32 str_len);

int32
tbl_ripng_key_str2val(char *str, tbl_ripng_t *p_ripng);

int32
tbl_ripng_dump_one(tbl_ripng_t *p_ripng, tbl_iter_args_t *pargs);

char*
tbl_ripng_key_name_dump(tbl_ripng_t *p_ripng, char* str);

char*
tbl_ripng_key_value_dump(tbl_ripng_t *p_ripng, char* str);

char*
tbl_ripng_field_name_dump(tbl_ripng_t *p_ripng, int32 field_id, char* str);

char*
tbl_ripng_field_value_dump(tbl_ripng_t *p_ripng, int32 field_id, char* str);

char**
tbl_ripng_table_dump(tbl_ripng_t *p_ripng, char** str);

int32
tbl_ripng_table_parser(char** array, char* key_value, tbl_ripng_t *p_ripng);

int32
tbl_ripng_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ripng_master_t*
tbl_ripng_get_master();

tbl_ripng_master_t*
tbl_ripng_init_ripng();

#endif /* !__TBL_RIPNG_H__ */


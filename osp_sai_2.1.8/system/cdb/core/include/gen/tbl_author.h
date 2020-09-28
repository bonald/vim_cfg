
#ifndef __TBL_AUTHOR_H__
#define __TBL_AUTHOR_H__

int32
tbl_author_add_author_sync(tbl_author_t *p_author, uint32 sync);

int32
tbl_author_del_author_sync(tbl_author_key_t *p_author_key, uint32 sync);

int32
tbl_author_set_author_field_sync(tbl_author_t *p_author, tbl_author_field_id_t field_id, uint32 sync);

int32
tbl_author_add_author(tbl_author_t *p_author);

int32
tbl_author_del_author(tbl_author_key_t *p_author_key);

int32
tbl_author_set_author_field(tbl_author_t *p_author, tbl_author_field_id_t field_id);

tbl_author_t*
tbl_author_get_author(tbl_author_key_t *p_author_key);

char*
tbl_author_key_val2str(tbl_author_t *p_author, char *str, uint32 str_len);

int32
tbl_author_key_str2val(char *str, tbl_author_t *p_author);

int32
tbl_author_dump_one(tbl_author_t *p_author, tbl_iter_args_t *pargs);

char*
tbl_author_key_name_dump(tbl_author_t *p_author, char* str);

char*
tbl_author_key_value_dump(tbl_author_t *p_author, char* str);

char*
tbl_author_field_name_dump(tbl_author_t *p_author, int32 field_id, char* str);

char*
tbl_author_field_value_dump(tbl_author_t *p_author, int32 field_id, char* str);

char**
tbl_author_table_dump(tbl_author_t *p_author, char** str);

int32
tbl_author_table_parser(char** array, char* key_value, tbl_author_t *p_author);

int32
tbl_author_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_author_master_t*
tbl_author_get_master();

tbl_author_master_t*
tbl_author_init_author();

#endif /* !__TBL_AUTHOR_H__ */


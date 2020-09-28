
#ifndef __TBL_L2EDIT_H__
#define __TBL_L2EDIT_H__

int32
tbl_l2edit_add_l2edit_sync(tbl_l2edit_t *p_l2edit, uint32 sync);

int32
tbl_l2edit_del_l2edit_sync(tbl_l2edit_key_t *p_l2edit_key, uint32 sync);

int32
tbl_l2edit_set_l2edit_field_sync(tbl_l2edit_t *p_l2edit, tbl_l2edit_field_id_t field_id, uint32 sync);

int32
tbl_l2edit_add_l2edit(tbl_l2edit_t *p_l2edit);

int32
tbl_l2edit_del_l2edit(tbl_l2edit_key_t *p_l2edit_key);

int32
tbl_l2edit_set_l2edit_field(tbl_l2edit_t *p_l2edit, tbl_l2edit_field_id_t field_id);

tbl_l2edit_t*
tbl_l2edit_get_l2edit(tbl_l2edit_key_t *p_l2edit_key);

char*
tbl_l2edit_key_val2str(tbl_l2edit_t *p_l2edit, char *str, uint32 str_len);

int32
tbl_l2edit_key_str2val(char *str, tbl_l2edit_t *p_l2edit);

int32
tbl_l2edit_dump_one(tbl_l2edit_t *p_l2edit, tbl_iter_args_t *pargs);

char*
tbl_l2edit_key_name_dump(tbl_l2edit_t *p_l2edit, char* str);

char*
tbl_l2edit_key_value_dump(tbl_l2edit_t *p_l2edit, char* str);

char*
tbl_l2edit_field_name_dump(tbl_l2edit_t *p_l2edit, int32 field_id, char* str);

char*
tbl_l2edit_field_value_dump(tbl_l2edit_t *p_l2edit, int32 field_id, char* str);

char**
tbl_l2edit_table_dump(tbl_l2edit_t *p_l2edit, char** str);

int32
tbl_l2edit_table_parser(char** array, char* key_value, tbl_l2edit_t *p_l2edit);

int32
tbl_l2edit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_l2edit_master_t*
tbl_l2edit_get_master();

tbl_l2edit_master_t*
tbl_l2edit_init_l2edit();

#endif /* !__TBL_L2EDIT_H__ */


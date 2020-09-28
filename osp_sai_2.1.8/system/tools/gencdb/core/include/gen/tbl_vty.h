
#ifndef __TBL_VTY_H__
#define __TBL_VTY_H__

int32
tbl_vty_add_vty_sync(tbl_vty_t *p_vty, uint32 sync);

int32
tbl_vty_del_vty_sync(tbl_vty_key_t *p_vty_key, uint32 sync);

int32
tbl_vty_set_vty_field_sync(tbl_vty_t *p_vty, tbl_vty_field_id_t field_id, uint32 sync);

int32
tbl_vty_add_vty(tbl_vty_t *p_vty);

int32
tbl_vty_del_vty(tbl_vty_key_t *p_vty_key);

int32
tbl_vty_set_vty_field(tbl_vty_t *p_vty, tbl_vty_field_id_t field_id);

tbl_vty_t*
tbl_vty_get_vty(tbl_vty_key_t *p_vty_key);

char*
tbl_vty_key_val2str(tbl_vty_t *p_vty, char *str, uint32 str_len);

int32
tbl_vty_key_str2val(char *str, tbl_vty_t *p_vty);

int32
tbl_vty_dump_one(tbl_vty_t *p_vty, tbl_iter_args_t *pargs);

char*
tbl_vty_key_name_dump(tbl_vty_t *p_vty, char* str);

char*
tbl_vty_key_value_dump(tbl_vty_t *p_vty, char* str);

char*
tbl_vty_field_name_dump(tbl_vty_t *p_vty, int32 field_id, char* str);

char*
tbl_vty_field_value_dump(tbl_vty_t *p_vty, int32 field_id, char* str);

char**
tbl_vty_table_dump(tbl_vty_t *p_vty, char** str);

int32
tbl_vty_table_parser(char** array, char* key_value, tbl_vty_t *p_vty);

int32
tbl_vty_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vty_master_t*
tbl_vty_get_master();

tbl_vty_master_t*
tbl_vty_init_vty();

#endif /* !__TBL_VTY_H__ */


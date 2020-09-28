
#ifndef __TBL_CONTROLLER_ATTR_H__
#define __TBL_CONTROLLER_ATTR_H__

int32
tbl_controller_attr_add_controller_attr_sync(tbl_controller_attr_t *p_controller_attr, uint32 sync);

int32
tbl_controller_attr_del_controller_attr_sync(tbl_controller_attr_key_t *p_controller_attr_key, uint32 sync);

int32
tbl_controller_attr_set_controller_attr_field_sync(tbl_controller_attr_t *p_controller_attr, tbl_controller_attr_field_id_t field_id, uint32 sync);

int32
tbl_controller_attr_add_controller_attr(tbl_controller_attr_t *p_controller_attr);

int32
tbl_controller_attr_del_controller_attr(tbl_controller_attr_key_t *p_controller_attr_key);

int32
tbl_controller_attr_set_controller_attr_field(tbl_controller_attr_t *p_controller_attr, tbl_controller_attr_field_id_t field_id);

tbl_controller_attr_t*
tbl_controller_attr_get_controller_attr(tbl_controller_attr_key_t *p_controller_attr_key);

char*
tbl_controller_attr_key_val2str(tbl_controller_attr_t *p_controller_attr, char *str, uint32 str_len);

int32
tbl_controller_attr_key_str2val(char *str, tbl_controller_attr_t *p_controller_attr);

int32
tbl_controller_attr_dump_one(tbl_controller_attr_t *p_controller_attr, tbl_iter_args_t *pargs);

char*
tbl_controller_attr_key_name_dump(tbl_controller_attr_t *p_controller_attr, char* str);

char*
tbl_controller_attr_key_value_dump(tbl_controller_attr_t *p_controller_attr, char* str);

char*
tbl_controller_attr_field_name_dump(tbl_controller_attr_t *p_controller_attr, int32 field_id, char* str);

char*
tbl_controller_attr_field_value_dump(tbl_controller_attr_t *p_controller_attr, int32 field_id, char* str);

char**
tbl_controller_attr_table_dump(tbl_controller_attr_t *p_controller_attr, char** str);

int32
tbl_controller_attr_table_parser(char** array, char* key_value, tbl_controller_attr_t *p_controller_attr);

int32
tbl_controller_attr_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_controller_attr_master_t*
tbl_controller_attr_get_master();

tbl_controller_attr_master_t*
tbl_controller_attr_init_controller_attr();

#endif /* !__TBL_CONTROLLER_ATTR_H__ */


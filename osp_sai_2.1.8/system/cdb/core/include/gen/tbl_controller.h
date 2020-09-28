
#ifndef __TBL_CONTROLLER_H__
#define __TBL_CONTROLLER_H__

int32
tbl_controller_add_controller_sync(tbl_controller_t *p_controller, uint32 sync);

int32
tbl_controller_del_controller_sync(tbl_controller_key_t *p_controller_key, uint32 sync);

int32
tbl_controller_set_controller_field_sync(tbl_controller_t *p_controller, tbl_controller_field_id_t field_id, uint32 sync);

int32
tbl_controller_add_controller(tbl_controller_t *p_controller);

int32
tbl_controller_del_controller(tbl_controller_key_t *p_controller_key);

int32
tbl_controller_set_controller_field(tbl_controller_t *p_controller, tbl_controller_field_id_t field_id);

tbl_controller_t*
tbl_controller_get_controller(tbl_controller_key_t *p_controller_key);

char*
tbl_controller_key_val2str(tbl_controller_t *p_controller, char *str, uint32 str_len);

int32
tbl_controller_key_str2val(char *str, tbl_controller_t *p_controller);

int32
tbl_controller_dump_one(tbl_controller_t *p_controller, tbl_iter_args_t *pargs);

char*
tbl_controller_key_name_dump(tbl_controller_t *p_controller, char* str);

char*
tbl_controller_key_value_dump(tbl_controller_t *p_controller, char* str);

char*
tbl_controller_field_name_dump(tbl_controller_t *p_controller, int32 field_id, char* str);

char*
tbl_controller_field_value_dump(tbl_controller_t *p_controller, int32 field_id, char* str);

char**
tbl_controller_table_dump(tbl_controller_t *p_controller, char** str);

int32
tbl_controller_table_parser(char** array, char* key_value, tbl_controller_t *p_controller);

int32
tbl_controller_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_controller_master_t*
tbl_controller_get_master();

tbl_controller_master_t*
tbl_controller_init_controller();

#endif /* !__TBL_CONTROLLER_H__ */

